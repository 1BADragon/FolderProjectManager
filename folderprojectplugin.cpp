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

#include "folderprojectplugin.h"

#include "folderbuildconfiguration.h"
#include "folderprojectwizard.h"
#include "folderprojectconstants.h"
#include "folderprojectfileseditor.h"
#include "foldermakestep.h"
#include "folderproject.h"

#include <coreplugin/icore.h>
#include <coreplugin/actionmanager/actionmanager.h>
#include <coreplugin/actionmanager/actioncontainer.h>
#include <coreplugin/actionmanager/command.h>

#include <projectexplorer/projectexplorerconstants.h>
#include <projectexplorer/projectmanager.h>
#include <projectexplorer/projectnodes.h>
#include <projectexplorer/projecttree.h>
#include <projectexplorer/selectablefilesmodel.h>
#include <projectexplorer/taskhub.h>

#include <utils/algorithm.h>
#include <utils/fileutils.h>
#include <utils/qtcassert.h>

#include <QAction>

using namespace Core;
using namespace ProjectExplorer;
using namespace Utils;
namespace PEC = ProjectExplorer::Constants;

namespace FolderProjectManager {
namespace Internal {

class FolderProjectPluginPrivate : public QObject
{
public:
    FolderProjectPluginPrivate();

    FolderFilesFactory projectFilesFactory;
    FolderMakeStepFactory makeStepFactory;
    FolderBuildConfigurationFactory buildConfigFactory;

    QAction editFilesAction{FolderProjectPlugin::tr("Edit Files..."), nullptr};
};

FolderProjectPlugin::~FolderProjectPlugin()
{
    delete d;
}

bool FolderProjectPlugin::initialize(const QStringList &, QString *)
{
    d = new FolderProjectPluginPrivate;
    return true;
}

FolderProjectPluginPrivate::FolderProjectPluginPrivate()
{
    ProjectManager::registerProjectType<FolderProject>(Constants::FOLDERMIMETYPE);

    IWizardFactory::registerFactoryCreator([] { return QList<IWizardFactory *>{new FolderProjectWizard}; });

    ActionContainer *mproject = ActionManager::actionContainer(PEC::M_PROJECTCONTEXT);

    Command *command = ActionManager::registerAction(&editFilesAction,
        "GenericProjectManager.EditFiles", Context(Constants::FOLDERPROJECT_ID));
    command->setAttribute(Command::CA_Hide);
    mproject->addAction(command, PEC::G_PROJECT_FILES);

    connect(&editFilesAction, &QAction::triggered, this, [] {
        if (auto genericProject = qobject_cast<FolderProject *>(ProjectTree::currentProject()))
            genericProject->editFilesTriggered();
    });

    const auto removeDirAction = new QAction(FolderProjectPlugin::tr("Remove Directory"), this);
    Command * const cmd = ActionManager::registerAction(removeDirAction, "GenericProject.RemoveDir",
                                                        Context(PEC::C_PROJECT_TREE));
    ActionManager::actionContainer(PEC::M_FOLDERCONTEXT)->addAction(cmd, PEC::G_FOLDER_OTHER);
    connect(removeDirAction, &QAction::triggered, this, [] {
        const auto folderNode = ProjectTree::currentNode()->asFolderNode();
        QTC_ASSERT(folderNode, return);
        const auto project = qobject_cast<FolderProject *>(folderNode->getProject());
        QTC_ASSERT(project, return);
        const FilePaths filesToRemove = transform(
                    folderNode->findNodes([](const Node *node) { return node->asFileNode(); }),
                    [](const Node *node) { return node->filePath();});
        project->removeFilesTriggered(filesToRemove);
    });
}

} // namespace Internal
} // namespace GenericProjectManager
