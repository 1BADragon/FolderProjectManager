#include "folderprojectplugin.h"

#include "folderprojectwizard.h"
#include "folderprojectconstants.h"
#include "folderprojectfileseditor.h"
#include "folderbuildconfiguration.h"
#include "foldermakestep.h"
#include "folderproject.h"

#include <coreplugin/icore.h>
#include <projectexplorer/projectmanager.h>

#include <QAction>
#include <QMessageBox>
#include <QFileDialog>
#include <QDebug>

namespace FolderProjectManager {
namespace Internal {

class FolderProjectPluginPrivate : public QObject
{
public:
    FolderProjectPluginPrivate();

    FolderFilesFactory projectFilesFactory;
    FolderMakeStepFactory makeStepFactory;
    FolderBuildConfigurationFactory buildConfigFactory;
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
    ProjectExplorer::ProjectManager::registerProjectType<FolderProject>(Constants::FOLDERMIMETYPE);

    Core::IWizardFactory::registerFactoryCreator([] {
        return QList<Core::IWizardFactory *>{new FolderProjectWizard};
    });
}

}
}
