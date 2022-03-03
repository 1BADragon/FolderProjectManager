#ifndef __FOLDERPROJECT_H__
#define __FOLDERPROJECT_H__

#include <projectexplorer/project.h>

namespace FolderProjectManager {
namespace Internal {

class FolderProject : public ProjectExplorer::Project
{
    Q_OBJECT

public:
    explicit FolderProject(const Utils::FilePath &filename);

private:
    RestoreResult fromMap(const QVariantMap &map, QString *errorMessage) final;
    ProjectExplorer::DeploymentKnowledge deploymentKnowledge() const final;
    void configureAsExampleProject(ProjectExplorer::Kit *kit) override;
};

} // namespace Internal
} // namespace GenericProjectManager

#endif
