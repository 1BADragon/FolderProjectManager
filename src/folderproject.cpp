#include "folderproject.h"

#include "folderbuildconfiguration.h"
#include "foldermakestep.h"
#include "folderprojectconstants.h"
#include "asyncfoldermonitor.h"
#include "folderprojectsettings.h"

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
#include <QList>

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

namespace Settings {
Setting includes("includedirs", Array());
Setting cflags("cflags", Array());
Setting cxxflags("c++flags", Array());
Setting ignores("ignore", Array());
}

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
        return  action == AddNewFile
                || action == RemoveFile
                || action == Rename;
    }

    RemovedFilesFromProject removeFiles(Node *, const FilePaths &filePaths, FilePaths *) final;
    bool renameFile(Node *, const FilePath &oldFilePath, const FilePath &newFilePath) final;
    QString name() const final { return QLatin1String("generic"); }

    void refresh(RefreshOptions options);

    void parse(RefreshOptions options);

    using SourceFile = QPair<FilePath, QStringList>;
    using SourceFiles = QList<SourceFile>;

    Utils::FilePath findCommonSourceRoot();
    void refreshCppCodeModel();

private:
    AsyncFolderMonitor _monitor;
    FolderProjectSettings _settings;

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
    setProjectLanguages(Context(ProjectExplorer::Constants::CXX_LANGUAGE_ID));
    setDisplayName(fileName.completeBaseName());
    setBuildSystemCreator([](Target *t) {
        return new FolderBuildSystem(t);
    });
}

FolderBuildSystem::FolderBuildSystem(Target *target)
    : BuildSystem(target), _monitor(target->project()->projectDirectory())
{
    _settings.setFile(target->project()->projectFilePath());
    _settings.update();

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

    connect(&_monitor, &AsyncFolderMonitor::filesChanged, this, [this]() {
        qDebug() << "file Changed triggered";
        this->refresh(Everything);
    });

    connect(target, &Target::activeBuildConfigurationChanged, this, [this, target] {
        if (target == project()->activeTarget()) {
            refresh(Everything);
        }
    });
    connect(project(), &Project::activeTargetChanged, this, [this, target] {
        if (target == project()->activeTarget()) {
            refresh(Everything);
        }
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

RemovedFilesFromProject FolderBuildSystem::removeFiles(Node *, const FilePaths &filePaths, FilePaths *)
{
    Q_UNUSED(filePaths);

    return RemovedFilesFromProject::Ok;
}

bool FolderBuildSystem::renameFile(Node *, const FilePath &oldFilePath, const FilePath &newFilePath)
{
    Q_UNUSED(oldFilePath);
    Q_UNUSED(newFilePath);

    return true;
}

void FolderBuildSystem::refresh(RefreshOptions options)
{
    Q_UNUSED(options);

    ParseGuard guard = guardParsingRun();
    auto baseDir = projectDirectory();

    QStringList filters;
    for (auto v : _settings[Settings::ignores].toArray()) {
        filters.push_back(v.toString());
    }
    _monitor.setFilters(filters);

    auto root = std::make_unique<ProjectNode>(baseDir);
    std::vector<std::unique_ptr<FileNode>> fileNodes;
    for (auto &f : _monitor.fileList()) {
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
    refreshCppCodeModel();
    emitBuildSystemUpdated();
}

void FolderBuildSystem::refreshCppCodeModel()
{
    if (!m_cppCodeModelUpdater) {
        return;
    }
    if (target() != project()->activeTarget()) {
        return;
    }

    _settings.refresh();

    QtSupport::CppKitInfo kitInfo(kit());
    QTC_ASSERT(kitInfo.isValid(), return);

    RawProjectPart rpp;
    rpp.setDisplayName(project()->displayName());
    rpp.setProjectFileLocation(projectFilePath().toString());
    rpp.setQtVersion(kitInfo.projectPartQtVersion);

    ProjectExplorer::HeaderPaths headers;
    auto base_dir = projectDirectory();
    for (auto v : _settings[Settings::includes].toArray()) {        
        auto path = Utils::FilePath::fromString(v.toString());
        Utils::FilePath complete_path;
        HeaderPathType t;

        if (path.isRelativePath()) {
            complete_path = base_dir.pathAppended(path.toString());
            t = ProjectExplorer::HeaderPathType::User;
        } else {
            complete_path = path;
            t = ProjectExplorer::HeaderPathType::System;
        }

        headers.push_back({complete_path.toString(), t});
    }
    rpp.setHeaderPaths(headers);

    QStringList cflags;
    for (auto v : _settings[Settings::cflags].toArray()) {
        cflags.append(v.toString());
    }
    rpp.setFlagsForCxx({nullptr, cflags, projectDirectory().toString()});

    QStringList cxxflags;
    for (auto v : _settings[Settings::cxxflags].toArray()) {
        cxxflags.append(v.toString());
    }
    rpp.setFlagsForC({nullptr, cxxflags, projectDirectory().toString()});

    QStringList file_list;
    for (auto &f : _monitor.fileList()) {
        file_list.push_back(f.toString());
    }
    rpp.setFiles(file_list);

    rpp.setPreCompiledHeaders(Utils::filtered(file_list, [](const QString &f) {
        return f.contains("pch");
    }));

    m_cppCodeModelUpdater->update({project(), kitInfo, activeParseEnvironment(), {rpp}});
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
    if (Target *t = m_project->activeTarget()) {
        static_cast<FolderBuildSystem *>(t->buildSystem())->refresh(m_options);
    }

    return true;
}

} // namespace Internal
} // namespace GenericProjectManager
