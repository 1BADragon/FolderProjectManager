#ifndef __FOLDERBUILDCONFIGUREATION_H__
#define __FOLDERBUILDCONFIGUREATION_H__

#include <projectexplorer/buildconfiguration.h>

namespace FolderProjectManager {
namespace Internal {

class FolderBuildConfiguration : public ProjectExplorer::BuildConfiguration
{
    Q_OBJECT

    friend class ProjectExplorer::BuildConfigurationFactory;
    FolderBuildConfiguration(ProjectExplorer::Target *target, Utils::Id id);

    void addToEnvironment(Utils::Environment &env) const final;
};

class FolderBuildConfigurationFactory final : public ProjectExplorer::BuildConfigurationFactory
{
public:
    FolderBuildConfigurationFactory();
};

} // namespace Internal
} // namespace GenericProjectManager

#endif
