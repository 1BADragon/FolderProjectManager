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

    connect(this, &AsyncFolderMonitorWorker::updateFilters,
            this, &AsyncFolderMonitorWorker::updateFiltersSlot,
            Qt::QueuedConnection);
}

AsyncFolderMonitorWorker::~AsyncFolderMonitorWorker()
{
    delete _watcher;
}

std::optional<QList<Utils::FilePath> > AsyncFolderMonitorWorker::currentFileList()
{
    if (_mut.try_lock()) {
        auto list = QList(_files.begin(), _files.end());
        _mut.unlock();
        return list;
    }

    return std::nullopt;
}

void AsyncFolderMonitorWorker::directoryChangedSlot(const QString &path)
{
    auto f = Utils::FilePath::fromString(path);

    QMutexLocker _(&_mut);

    auto _old = _files;

    // scrub the list of childen of the dir
    purgeFolder(path);

    if (f.exists()) {
        _job_queue.emplace_back(f);
    }

    processQueue();

    if (_old != _files) {
        emit filesChanged();
    }
}

void AsyncFolderMonitorWorker::updateFiltersSlot()
{
    QMutexLocker _(&_mut);

    _job_queue.clear();
    _job_queue.push_back(_root);

    processQueue();
}

void AsyncFolderMonitorWorker::setFilters(const QStringList &newFilters)
{
    if (!_mut.try_lock()) {
        return;
    }

    _filters.clear();

    for (const auto &filter : newFilters) {
        auto reges = QRegularExpression::wildcardToRegularExpression(filter,
                                                                     QRegularExpression::UnanchoredWildcardConversion);

        _filters.emplace_back(reges);
    }

    _mut.unlock();

    emit updateFilters();
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

    for (auto &c : dir.dirEntries(QStringList(), DIRFILTERS, QDir::Name)) {
        if (matchesFilter(c.toString())) {
            continue;
        }
        if (c.isDir()) {
            _job_queue.emplace_back(c);

            if (!_watcher->watchesDirectory(c.toString())) {
                _watcher->addDirectory(c.toString(), Utils::FileSystemWatcher::WatchAllChanges);
            }
        } else if (c.isFile()) {
            _files.insert(c);
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
