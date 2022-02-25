/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of Qt Creator.
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
****************************************************************************/

#include "folderproject.h"

#include "folderbuildconfiguration.h"
#include "foldermakestep.h"
#include "folderprojectconstants.h"
#include "recursivefoldermonitor.h"

#include <coreplugin/documentmanager.h>
#include <coreplugin/icontext.h>
#include <coreplugin/icore.h>
#include <coreplugin/idocument.h>

#include <cppeditor/cppprojectupdaterinterface.h>

#include <extensionsystem/pluginmanager.h>

#include <projectexplorer/abi.h>
#include <projectexplorer/buildinfo.h>
#include <projectexplorer/buildsteplist.h>
#include <projectexplorer/buildsystem.h>
#include <projectexplorer/customexecutablerunconfiguration.h>
#include <projectexplorer/deploymentdata.h>
#include <projectexplorer/headerpath.h>
#include <projectexplorer/kitinformation.h>
#include <projectexplorer/projectexplorerconstants.h>
#include <projectexplorer/projectnodes.h>
#include <projectexplorer/selectablefilesmodel.h>
#include <projectexplorer/target.h>
#include <projectexplorer/taskhub.h>

#include <qtsupport/baseqtversion.h>
#include <qtsupport/qtcppkitinfo.h>
#include <qtsupport/qtkitinformation.h>

#include <utils/algorithm.h>
#include <utils/filesystemwatcher.h>
#include <utils/fileutils.h>
#include <utils/qtcassert.h>
#include <utils/qtcprocess.h>

#include <QDir>
#include <QFileInfo>
#include <QHash>
#include <QMetaObject>
#include <QPair>
#include <QSet>
#include <QStringList>

#include <set>

using namespace Core;
using namespace ProjectExplorer;
using namespace Utils;

namespace FolderProjectManager {
namespace Internal {

enum RefreshOptions {
    Files         = 0x01,
    Configuration = 0x02,
    Everything    = Files | Configuration
};

////////////////////////////////////////////////////////////////////////////////////
//
// GenericProjectFile
//
////////////////////////////////////////////////////////////////////////////////////

class FolderProjectFile : public Core::IDocument
{
public:
    FolderProjectFile(FolderProject *parent, const FilePath &fileName, RefreshOptions options)
        : m_project(parent), m_options(options)
    {
        setId("Generic.ProjectFile");
        setMimeType(Constants::FOLDERMIMETYPE);
        setFilePath(fileName);
    }

    ReloadBehavior reloadBehavior(ChangeTrigger, ChangeType) const final
    {
        return BehaviorSilent;
    }

    bool reload(QString *errorString, ReloadFlag flag, ChangeType type) override;

private:
    FolderProject *m_project = nullptr;
    RefreshOptions m_options;
};


////////////////////////////////////////////////////////////////////////////////////
//
// GenericProjectNode
//
////////////////////////////////////////////////////////////////////////////////////

class FolderBuildSystem : public BuildSystem
{
public:
    explicit FolderBuildSystem(Target *target);
    ~FolderBuildSystem();

    void triggerParsing() final;

    bool supportsAction(Node *, ProjectAction action, const Node *) const final
    {
        return  action == RemoveFile
                || action == Rename;
    }

    RemovedFilesFromProject removeFiles(Node *, const FilePaths &filePaths, FilePaths *) final;
    bool renameFile(Node *, const FilePath &oldFilePath, const FilePath &newFilePath) final;
    QString name() const final { return QLatin1String("generic"); }

    void refresh(RefreshOptions options);

    void parse(RefreshOptions options);

    using SourceFile = QPair<FilePath, QStringList>;
    using SourceFiles = QList<SourceFile>;
    SourceFiles processEntries(const QStringList &paths,
                               QHash<QString, QString> *map = nullptr) const;

    Utils::FilePath findCommonSourceRoot();
    void refreshCppCodeModel();
    void updateDeploymentData();

    void removeFiles(const FilePaths &filesToRemove);

private:
    RecursiveFolderMonitor _monitor;

    QStringList m_rawFileList;
    SourceFiles m_files;
    QHash<QString, QString> m_rawListEntries;
    QStringList m_rawProjectIncludePaths;
    ProjectExplorer::HeaderPaths m_projectIncludePaths;
    QStringList m_cxxflags;
    QStringList m_cflags;

    CppEditor::CppProjectUpdaterInterface *m_cppCodeModelUpdater = nullptr;
};

////////////////////////////////////////////////////////////////////////////////////
//
// GenericProject
//
////////////////////////////////////////////////////////////////////////////////////

FolderProject::FolderProject(const Utils::FilePath &fileName)
    : Project(Constants::FOLDERMIMETYPE, fileName)
{
    setId(Constants::FOLDERPROJECT_ID);
    //setProjectLanguages(Context(ProjectExplorer::Constants::CXX_LANGUAGE_ID));
    setDisplayName(fileName.completeBaseName());
    setBuildSystemCreator([](Target *t) { return new FolderBuildSystem(t); });
}

FolderBuildSystem::FolderBuildSystem(Target *target)
    : BuildSystem(target), _monitor(target->project()->projectDirectory())
{
    QObject *projectUpdaterFactory = ExtensionSystem::PluginManager::getObjectByName(
                "CppProjectUpdaterFactory");
    if (projectUpdaterFactory) {
        const bool successFullyCreatedProjectUpdater
                = QMetaObject::invokeMethod(projectUpdaterFactory,
                                            "create",
                                            Q_RETURN_ARG(CppEditor::CppProjectUpdaterInterface*,
                                                         m_cppCodeModelUpdater));
        QTC_CHECK(successFullyCreatedProjectUpdater);
    }

    connect(target->project(), &Project::projectFileIsDirty, this, [this](const FilePath &p) {
        Q_UNUSED(p);
        this->refresh(Everything);
    });

    connect(&_monitor, &RecursiveFolderMonitor::filesUpdated, this, [this]() {
        this->refresh(Everything);
    });

    connect(target, &Target::activeBuildConfigurationChanged, this, [this, target] {
        if (target == project()->activeTarget())
            refresh(Everything);
    });
    connect(project(), &Project::activeTargetChanged, this, [this, target] {
        if (target == project()->activeTarget())
            refresh(Everything);
    });
}

FolderBuildSystem::~FolderBuildSystem()
{
    delete m_cppCodeModelUpdater;
}

void FolderBuildSystem::triggerParsing()
{
    refresh(Everything);
}

static void insertSorted(QStringList *list, const QString &value)
{
    const auto it = std::lower_bound(list->begin(), list->end(), value);
    if (it == list->end())
        list->append(value);
    else if (*it > value)
        list->insert(it, value);
}

RemovedFilesFromProject FolderBuildSystem::removeFiles(Node *, const FilePaths &filePaths, FilePaths *)
{
    QStringList newList = m_rawFileList;

    for (const FilePath &filePath : filePaths) {
        QHash<QString, QString>::iterator i = m_rawListEntries.find(filePath.toString());
        if (i != m_rawListEntries.end())
            newList.removeOne(i.value());
    }

    return RemovedFilesFromProject::Ok;
}

bool FolderBuildSystem::renameFile(Node *, const FilePath &oldFilePath, const FilePath &newFilePath)
{
    //    QStringList newList = m_rawFileList;

    //    QHash<QString, QString>::iterator i = m_rawListEntries.find(oldFilePath.toString());
    //    if (i != m_rawListEntries.end()) {
    //        int index = newList.indexOf(i.value());
    //        if (index != -1) {
    //            QDir baseDir(projectDirectory().toString());
    //            newList.removeAt(index);
    //            insertSorted(&newList, baseDir.relativeFilePath(newFilePath.toString()));
    //        }
    //    }

    //    return saveRawFileList(newList);
    return true;
}

void FolderBuildSystem::refresh(RefreshOptions options)
{
    ParseGuard guard = guardParsingRun();
    auto baseDir = projectDirectory();
    RecursiveFolderMonitor monitor(baseDir);

    auto root = std::make_unique<ProjectNode>(baseDir);
    std::vector<std::unique_ptr<FileNode>> fileNodes;
    for (auto &f : monitor.list()) {
        FileType fileType = FileType::Source;
        if (f == projectFilePath()) {
            fileType = FileType::Project;
        }
        fileNodes.emplace_back(std::make_unique<FileNode>(f, fileType));
    }

    root->addNestedNodes(std::move(fileNodes), baseDir);
    root->compress();
    setRootProjectNode(std::move(root));

    guard.markAsSuccess();


    //    ParseGuard guard = guardParsingRun();
    //    parse(options);

    //    if (options & Files) {
    //        auto newRoot = std::make_unique<ProjectNode>(projectDirectory());
    //        newRoot->setDisplayName(projectFilePath().completeBaseName());

    //        // find the common base directory of all source files
    //        FilePath baseDir = findCommonSourceRoot();

    //        std::vector<std::unique_ptr<FileNode>> fileNodes;
    //        for (const SourceFile &f : qAsConst(m_files)) {
    //            FileType fileType = FileType::Source; // ### FIXME
    //            if (f.first.endsWith(".qrc"))
    //                fileType = FileType::Resource;
    //            fileNodes.emplace_back(std::make_unique<FileNode>(f.first, fileType));
    //        }
    //        newRoot->addNestedNodes(std::move(fileNodes), baseDir);

    //        newRoot->compress();
    //        setRootProjectNode(std::move(newRoot));
    //    }

    //    refreshCppCodeModel();
    //    updateDeploymentData();
    //    guard.markAsSuccess();

    refreshCppCodeModel();
    emitBuildSystemUpdated();
}

/**
 * Expands environment variables and converts the path from relative to the
 * project to an absolute path.
 *
 * The \a map variable is an optional argument that will map the returned
 * absolute paths back to their original \a entries.
 */
FolderBuildSystem::SourceFiles FolderBuildSystem::processEntries(
        const QStringList &paths, QHash<QString, QString> *map) const
{
    const BuildConfiguration *const buildConfig = target()->activeBuildConfiguration();

    const Utils::Environment buildEnv = buildConfig ? buildConfig->environment()
                                                    : Utils::Environment::systemEnvironment();

    const Utils::MacroExpander *expander = project()->macroExpander();
    if (buildConfig)
        expander = buildConfig->macroExpander();
    else
        expander = target()->macroExpander();

    const QDir projectDir(projectDirectory().toString());

    QFileInfo fileInfo;
    SourceFiles sourceFiles;
    std::set<QString> seenFiles;
    for (const QString &path : paths) {
        QString trimmedPath = path.trimmed();
        if (trimmedPath.isEmpty())
            continue;

        trimmedPath = buildEnv.expandVariables(trimmedPath);
        trimmedPath = expander->expand(trimmedPath);

        trimmedPath = Utils::FilePath::fromUserInput(trimmedPath).toString();

        QStringList tagsForFile;
        const int tagListPos = trimmedPath.indexOf('|');
        if (tagListPos != -1) {
            tagsForFile = trimmedPath.mid(tagListPos + 1).simplified()
                    .split(' ', Qt::SkipEmptyParts);
            trimmedPath = trimmedPath.left(tagListPos).trimmed();
        }

        if (!seenFiles.insert(trimmedPath).second)
            continue;

        fileInfo.setFile(projectDir, trimmedPath);
        if (fileInfo.exists()) {
            const QString absPath = fileInfo.absoluteFilePath();
            sourceFiles.append({FilePath::fromString(absPath), tagsForFile});
            if (map)
                map->insert(absPath, trimmedPath);
        }
    }
    return sourceFiles;
}

void FolderBuildSystem::refreshCppCodeModel()
{
    if (!m_cppCodeModelUpdater)
        return;
    if (target() != project()->activeTarget())
        return;
    QtSupport::CppKitInfo kitInfo(kit());
    QTC_ASSERT(kitInfo.isValid(), return);

    RawProjectPart rpp;
    rpp.setDisplayName(project()->displayName());
    rpp.setProjectFileLocation(projectFilePath().toString());
    rpp.setQtVersion(kitInfo.projectPartQtVersion);
    rpp.setHeaderPaths(m_projectIncludePaths);
    rpp.setFlagsForCxx({nullptr, m_cxxflags, projectDirectory().toString()});
    rpp.setFlagsForC({nullptr, m_cflags, projectDirectory().toString()});

    static const auto sourceFilesToStringList = [](const SourceFiles &sourceFiles) {
        return Utils::transform(sourceFiles, [](const SourceFile &f) {
            return f.first.toString();
        });
    };
    QStringList file_list;
    for (auto &f : _monitor.list()) {
        file_list.push_back(f.toString());
    }
    rpp.setFiles(file_list);
    rpp.setPreCompiledHeaders(sourceFilesToStringList(
                                  Utils::filtered(m_files, [](const SourceFile &f) { return f.second.contains("pch"); })));

    m_cppCodeModelUpdater->update({project(), kitInfo, activeParseEnvironment(), {rpp}});
}

void FolderBuildSystem::updateDeploymentData()
{
    static const QString fileName("QtCreatorDeployment.txt");
    Utils::FilePath deploymentFilePath;
    BuildConfiguration *bc = target()->activeBuildConfiguration();
    if (bc)
        deploymentFilePath = bc->buildDirectory().pathAppended(fileName);

    bool hasDeploymentData = QFileInfo::exists(deploymentFilePath.toString());
    if (!hasDeploymentData) {
        deploymentFilePath = projectDirectory().pathAppended(fileName);
        hasDeploymentData = QFileInfo::exists(deploymentFilePath.toString());
    }
    if (hasDeploymentData) {
        DeploymentData deploymentData;
        deploymentData.addFilesFromDeploymentFile(deploymentFilePath.toString(),
                                                  projectDirectory().toString());
        setDeploymentData(deploymentData);
    }
}

void FolderBuildSystem::removeFiles(const FilePaths &filesToRemove)
{

}

Project::RestoreResult FolderProject::fromMap(const QVariantMap &map, QString *errorMessage)
{
    const RestoreResult result = Project::fromMap(map, errorMessage);
    if (result != RestoreResult::Ok)
        return result;

    if (!activeTarget())
        addTargetForDefaultKit();

    // Sanity check: We need both a buildconfiguration and a runconfiguration!
    const QList<Target *> targetList = targets();
    if (targetList.isEmpty())
        return RestoreResult::Error;

    for (Target *t : targetList) {
        if (!t->activeBuildConfiguration()) {
            removeTarget(t);
            continue;
        }
        if (!t->activeRunConfiguration())
            t->addRunConfiguration(new CustomExecutableRunConfiguration(t));
    }

    if (Target *t = activeTarget())
        static_cast<FolderBuildSystem *>(t->buildSystem())->refresh(Everything);

    return RestoreResult::Ok;
}

ProjectExplorer::DeploymentKnowledge FolderProject::deploymentKnowledge() const
{
    return DeploymentKnowledge::Approximative;
}

void FolderProject::configureAsExampleProject(ProjectExplorer::Kit *kit)
{
    QList<BuildInfo> infoList;
    const QList<Kit *> kits(kit != nullptr ? QList<Kit *>({kit}) : KitManager::kits());
    for (Kit *k : kits) {
        if (auto factory = BuildConfigurationFactory::find(k, projectFilePath())) {
            for (int i = 0; i < 5; ++i) {
                BuildInfo buildInfo;
                buildInfo.displayName = tr("Build %1").arg(i + 1);
                buildInfo.factory = factory;
                buildInfo.kitId = kit->id();
                buildInfo.buildDirectory = projectFilePath();
                infoList << buildInfo;
            }
        }
    }
    setup(infoList);
}

bool FolderProjectFile::reload(QString *errorString, IDocument::ReloadFlag flag, IDocument::ChangeType type)
{
    Q_UNUSED(errorString)
    Q_UNUSED(flag)
    Q_UNUSED(type)
    if (Target *t = m_project->activeTarget())
        static_cast<FolderBuildSystem *>(t->buildSystem())->refresh(m_options);

    return true;
}

void FolderProject::editFilesTriggered()
{
    //    SelectableFilesDialogEditFiles sfd(projectDirectory(),
    //                                       files(Project::AllFiles),
    //                                       ICore::dialogParent());
    //    if (sfd.exec() == QDialog::Accepted) {
    //        if (Target *t = activeTarget()) {
    //            auto bs = static_cast<FolderBuildSystem *>(t->buildSystem());
    //            bs->setFiles(transform(sfd.selectedFiles(), &FilePath::toString));
    //        }
    //    }
}

void FolderProject::removeFilesTriggered(const FilePaths &filesToRemove)
{
    if (Target *t = activeTarget())
        static_cast<FolderBuildSystem *>(t->buildSystem())->removeFiles(filesToRemove);
}

} // namespace Internal
} // namespace GenericProjectManager
