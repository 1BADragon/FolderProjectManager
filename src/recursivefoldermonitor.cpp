#include "recursivefoldermonitor.h"

#include <QApplication>
#include <QQueue>
#include <QRegExp>
#include <QRegularExpression>

namespace FolderProjectManager {
namespace Internal {

constexpr QDir::Filters DIRFILTERS = (QDir::AllDirs | QDir::Files | QDir::NoDotAndDotDot);

RecursiveFolderMonitor::RecursiveFolderMonitor(const Utils::FilePath &root, QObject *parent)
    : QObject{parent}, _root(root), _files(), _filters()
{
    buildFileList();
    connect(&_watcher, &Utils::FileSystemWatcher::fileChanged,
            this, &RecursiveFolderMonitor::fileChanged);

    connect(&_watcher, &Utils::FileSystemWatcher::directoryChanged,
            this, &RecursiveFolderMonitor::directoryChanged);
}

const QList<Utils::FilePath> RecursiveFolderMonitor::list() const
{
    return {_files.begin(), _files.end()};
}

void RecursiveFolderMonitor::fileChanged(const QString &path)
{
    auto f = Utils::FilePath::fromString(path);

    if (f.exists()) {
        if (!_watcher.watchesFile(path)) {
            _watcher.addFile(path, Utils::FileSystemWatcher::WatchModifiedDate);
            emit filesChanged();
        }
    } else {
        auto it = _files.find(Utils::FilePath::fromString(path));
        if (it != _files.end()) {
            _files.erase(it);
            _watcher.removeFile(path);
            emit filesChanged();
        }
    }
}

void RecursiveFolderMonitor::directoryChanged(const QString &path)
{
    auto f = Utils::FilePath::fromString(path);

    // scrub the list of childen of the dir
    auto it = _files.begin();
    while (it != _files.end()) {
        if (it->startsWith(path)) {
            it = _files.erase(it);
        } else {
            it++;
        }
    }

    if (f.exists()) {
        traverseDirDeapthFirst(f);
    }

    emit filesChanged();
}

void RecursiveFolderMonitor::setFilters(const QStringList &newFilters)
{
    _filters = newFilters;
    buildFileList();
}

void RecursiveFolderMonitor::buildFileList()
{
    _watcher.clear();
    _files.clear();

    _watcher.addDirectory(_root.toString(), Utils::FileSystemWatcher::WatchAllChanges);
    traverseDirBreathFirst(_root);
}

void RecursiveFolderMonitor::traverseDirDeapthFirst(const Utils::FilePath &dir, int max_depth)
{
    if (max_depth == 0) {
        return;
    }

    QApplication::processEvents();

    for (auto &c : dir.dirEntries(QStringList(), DIRFILTERS, QDir::Name)) {
        if (c.isDir()) {
            if (!_watcher.watchesDirectory(c.toString())) {
                _watcher.addDirectory(c.toString(), Utils::FileSystemWatcher::WatchAllChanges);
            }

            traverseDirDeapthFirst(c, max_depth - 1);
        } else if (c.isFile()) {
            _files.insert(c);
            if (!_watcher.watchesFile(c.toString())) {
                _watcher.addFile(c.toString(), Utils::FileSystemWatcher::WatchModifiedDate);
            }
        }
    }
}

void RecursiveFolderMonitor::traverseDirBreathFirst(const Utils::FilePath &dir)
{
    QQueue<Utils::FilePath> remaining;

    remaining.push_back(dir);

    while (!remaining.empty()) {
        QApplication::processEvents();
        Utils::FilePath at = remaining.front();
        remaining.pop_front();

        if (!at.isDir()) {
            continue;
        }

        for (auto &c : at.dirEntries(QStringList(), DIRFILTERS, QDir::Name)) {
            if (c.isDir()) {
                remaining.push_back(c);

                if (!_watcher.watchesDirectory(c.toString())) {
                    _watcher.addDirectory(c.toString(), Utils::FileSystemWatcher::WatchAllChanges);
                }
            } else if (c.isFile()) {
                if (!matchesFilter(c.toString())) {
                    _files.insert(c);
                }

                if (!_watcher.watchesFile(c.toString())) {
                    _watcher.addFile(c.toString(), Utils::FileSystemWatcher::WatchModifiedDate);
                }
            }
        }
    }
}

bool RecursiveFolderMonitor::matchesFilter(const QString &path)
{
    for (auto &c : _filters) {
        auto reges = QRegularExpression::wildcardToRegularExpression(c);
        QRegularExpression f(reges);

        auto match = f.match(c);
        if (match.hasMatch()) {
            return true;
        }
    }

    return false;
}

}
}
