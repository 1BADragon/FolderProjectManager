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

const QList<Utils::FilePath> &RecursiveFolderMonitor::list() const
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

void RecursiveFolderMonitor::setFilters(const QStringList &newFilters)
{
    _filters = newFilters;
    buildFileList();
}

void RecursiveFolderMonitor::buildFileList()
{
    _watcher.clear();
    _list.clear();

    _watcher.addDirectory(_root.toString(), Utils::FileSystemWatcher::WatchAllChanges);
    traverseDir(_root);
}

void RecursiveFolderMonitor::traverseDir(const Utils::FilePath &dir)
{
    for (auto &c : dir.dirEntries(_filters)) {
        if (c == dir || dir.isChildOf(c)) {
            continue;
        }

        if (c.isDir()) {
            _watcher.addDirectory(c.toString(), Utils::FileSystemWatcher::WatchAllChanges);
            traverseDir(c);
        } else if (c.isFile()) {
            _list.push_back(c);
            _watcher.addFile(c.toString(), Utils::FileSystemWatcher::WatchModifiedDate);
        }
    }
}

}
}
