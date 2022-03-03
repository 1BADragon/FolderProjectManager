#include "folderbuildconfiguration.h"

#include "foldermakestep.h"
#include "folderproject.h"
#include "folderprojectconstants.h"

#include <projectexplorer/buildinfo.h>
#include <projectexplorer/buildsteplist.h>
#include <projectexplorer/kitinformation.h>
#include <projectexplorer/projectexplorerconstants.h>
#include <projectexplorer/target.h>

#include <qtsupport/qtkitinformation.h>

#include <utils/aspects.h>
#include <utils/pathchooser.h>
#include <utils/qtcassert.h>


using namespace ProjectExplorer;
using namespace Utils;

namespace FolderProjectManager {
namespace Internal {

FolderBuildConfiguration::FolderBuildConfiguration(Target *parent, Utils::Id id)
    : BuildConfiguration(parent, id)
{
    setConfigWidgetDisplayName(tr("Generic Manager"));
    setBuildDirectoryHistoryCompleter("Generic.BuildDir.History");

    setInitializer([this](const BuildInfo &) {
        buildSteps()->appendStep(Constants::GENERIC_MS_ID);
        cleanSteps()->appendStep(Constants::GENERIC_MS_ID);
        updateCacheAndEmitEnvironmentChanged();
    });

    updateCacheAndEmitEnvironmentChanged();
}


// GenericBuildConfigurationFactory

FolderBuildConfigurationFactory::FolderBuildConfigurationFactory()
{
    registerBuildConfiguration<FolderBuildConfiguration>
        ("FolderProjectManager.FolderBuildConfiguration");

    setSupportedProjectType(Constants::FOLDERPROJECT_ID);
    setSupportedProjectMimeTypeName(Constants::FOLDERMIMETYPE);

    setBuildGenerator([](const Kit *, const FilePath &projectPath, bool forSetup) {
        BuildInfo info;
        info.typeName = BuildConfiguration::tr("Build");
        info.buildDirectory = forSetup ? Project::projectDirectory(projectPath) : projectPath;

        if (forSetup)  {
            //: The name of the build configuration created by default for a generic project.
            info.displayName = BuildConfiguration::tr("Default");
        }

        return QList<BuildInfo>{info};
    });
}

void FolderBuildConfiguration::addToEnvironment(Utils::Environment &env) const
{
    QtSupport::QtKitAspect::addHostBinariesToPath(kit(), env);
}

} // namespace Internal
} // namespace GenericProjectManager
