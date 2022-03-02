#ifndef __FOLDERPROJECTPLUGIN_H__
#define __FOLDERPROJECTPLUGIN_H__

#include <extensionsystem/iplugin.h>

namespace FolderProjectManager {
namespace Internal {

class FolderProjectPlugin : public ExtensionSystem::IPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QtCreatorPlugin" FILE "FolderProjectManager.json")

public:
    ~FolderProjectPlugin() override;

private:
    bool initialize(const QStringList &arguments, QString *errorString) override;

    class FolderProjectPluginPrivate *d = nullptr;
};

} // namespace Internal
} // namespace GenericProject

#endif
