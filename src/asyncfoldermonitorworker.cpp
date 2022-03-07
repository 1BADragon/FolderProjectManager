#include "asyncfoldermonitorworker.h"

namespace FolderProjectManager {
namespace Internal {

constexpr QDir::Filters DIRFILTERS = (QDir::AllDirs | QDir::Files | QDir::NoDotAndDotDot);

AsyncFolderMonitorWorker::AsyncFolderMonitorWorker(const Utils::FilePath &root, QObject *parent)
    : QObject{parent}
{
    _watcher = new Utils::FileSystemWatcher(this);

    _root = root;
    connect(_watcher, &Utils::FileSystemWatcher::directoryChanged,
            this, &AsyncFolderMonitorWorker::directoryChangedSlot);
}

AsyncFolderMonitorWorker::~AsyncFolderMonitorWorker()
{

}

QList<Utils::FilePath> AsyncFolderMonitorWorker::currentFileList()
{
    QMutexLocker _(&_mut);

    return {_files.begin(), _files.end()};
}

void AsyncFolderMonitorWorker::directoryChangedSlot(const QString &path)
{
    auto f = Utils::FilePath::fromString(path);

    // scrub the list of childen of the dir
    purgeFolder(path);

    if (f.exists()) {
        _job_queue.emplace_back(f);
    }

    processQueue();
    emit filesChanged();
}

void AsyncFolderMonitorWorker::setFilters(const QStringList &newFilters)
{
    _filters.clear();

    for (auto filter : newFilters) {
        auto reges = QRegularExpression::wildcardToRegularExpression(filter,
                                                                     QRegularExpression::UnanchoredWildcardConversion);

        _filters.emplace_back(reges);
    }
    _job_queue.clear();
    _job_queue.emplace_back(_root);
    processQueue();
}

void AsyncFolderMonitorWorker::processQueue()
{
    while (!_job_queue.empty()) {
        auto job = _job_queue.takeFirst();

        if (!_watcher->watchesDirectory(job._path.toString())) {
            _watcher->addDirectory(_root.toString(), Utils::FileSystemWatcher::WatchAllChanges);
        }

        traverseDir(job._path);
    }
}

void AsyncFolderMonitorWorker::traverseDir(const Utils::FilePath &dir)
{
    if (!dir.isDir()) {
        return;
    }

    QMutexLocker _(&_mut);

    for (auto &c : dir.dirEntries(QStringList(), DIRFILTERS, QDir::Name)) {
        if (c.isDir()) {
            _job_queue.emplace_back(c);

            if (!_watcher->watchesDirectory(c.toString())) {
                _watcher->addDirectory(c.toString(), Utils::FileSystemWatcher::WatchAllChanges);
            }
        } else if (c.isFile()) {
            if (!matchesFilter(c.toString())) {
                _files.insert(c);
            }
        }
    }
}

bool AsyncFolderMonitorWorker::matchesFilter(const QString &path)
{
    for (auto &f : _filters) {
        auto match = f.match(path);
        if (match.hasMatch()) {
            return true;
        }
    }

    return false;
}

void FolderProjectManager::Internal::AsyncFolderMonitorWorker::purgeFolder(const QString &path)
{
    auto it = _files.begin();
    while (it != _files.end()) {
        if (it->startsWith(path)) {
            it = _files.erase(it);
        } else {
            it++;
        }
    }
}

}
}
