#include "foldermakestep.h"
#include "folderprojectconstants.h"

#include <projectexplorer/buildsteplist.h>
#include <projectexplorer/projectexplorerconstants.h>

using namespace ProjectExplorer;

namespace FolderProjectManager {
namespace Internal {

class GenericMakeStep : public ProjectExplorer::MakeStep
{
public:
    explicit GenericMakeStep(BuildStepList *parent, Utils::Id id);
};

GenericMakeStep::GenericMakeStep(BuildStepList *parent, Utils::Id id)
    : MakeStep(parent, id)
{
    setAvailableBuildTargets({"all", "clean"});
    if (parent->id() == ProjectExplorer::Constants::BUILDSTEPS_BUILD) {
        setSelectedBuildTarget("all");
    } else if (parent->id() == ProjectExplorer::Constants::BUILDSTEPS_CLEAN) {
        setSelectedBuildTarget("clean");
        setIgnoreReturnValue(true);
    }
}

FolderMakeStepFactory::FolderMakeStepFactory()
{
    registerStep<GenericMakeStep>(Constants::GENERIC_MS_ID);
    setDisplayName(MakeStep::defaultDisplayName());
    setSupportedProjectType(Constants::FOLDERPROJECT_ID);
}

} // namespace Internal
} // namespace GenericProjectManager
