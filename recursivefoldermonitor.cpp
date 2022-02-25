#include "recursivefoldermonitor.h"

namespace FolderProjectManager {
namespace Internal {

RecursiveFolderMonitor::RecursiveFolderMonitor(const Utils::FilePath &root, QObject *parent)
    : QObject{parent}, _root(root), _list()
{
    buildFileList();
    connect(&_watcher, &Utils::FileSystemWatcher::fileChanged,
            this, &RecursiveFolderMonitor::fileChanged);

    connect(&_watcher, &Utils::FileSystemWatcher::directoryChanged,
            this, &RecursiveFolderMonitor::directoryChanged);
}

const std::list<Utils::FilePath>& RecursiveFolderMonitor::list() const
{
    return _list;
}

void RecursiveFolderMonitor::fileChanged(const QString &path)
{
    Q_UNUSED(path);
    buildFileList();
    emit filesUpdated();
}

void RecursiveFolderMonitor::directoryChanged(const QString &path)
{
    Q_UNUSED(path);
    buildFileList();
    emit filesUpdated();
}

void RecursiveFolderMonitor::buildFileList()
{
    _watcher.clear();
    _list.clear();
    traverseDir(_root);
}

void RecursiveFolderMonitor::traverseDir(const Utils::FilePath &dir)
{
    QStringList filters;
    for (auto &c : dir.dirEntries(filters)) {
        if (c == dir || dir.isChildOf(c)) {
            continue;
        }

        if (c.isDir()) {
            traverseDir(c);
        } else if (c.isFile()) {
            _list.push_back(c);
        }
    }
}

}
}
