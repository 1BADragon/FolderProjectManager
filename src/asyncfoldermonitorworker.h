#ifndef ASYNCFOLDERMONITORWORKER_H
#define ASYNCFOLDERMONITORWORKER_H

#include <memory>
#include <optional>

#include <QObject>
#include <QString>
#include <QRegularExpression>
#include <QQueue>
#include <QMutex>
#include <QList>

#include <utils/filepath.h>
#include <utils/filesystemwatcher.h>

namespace FolderProjectManager {
namespace Internal {

class AsyncFolderMonitorWorker : public QObject
{
    Q_OBJECT
public:    
    struct Job {
        Utils::FilePath _path;

        Job(const Utils::FilePath &path): _path(path) {}
    };

    using JobQueue = QList<Job>;

    explicit AsyncFolderMonitorWorker(const Utils::FilePath &root, QObject *parent = nullptr);
    virtual ~AsyncFolderMonitorWorker();

    std::optional<QList<Utils::FilePath>> currentFileList();
    void setFilters(const QStringList &newFilters);

signals:
    void filesChanged();
    void updateFilters();

private slots:
    void fileChangedSlot(const QString &path);
    void directoryChangedSlot(const QString &path);

    void updateFiltersSlot();

private:
    JobQueue _job_queue;
    Utils::FileSystemWatcher *_watcher;
    QMutex _mut;

    Utils::FilePath _root;
    QSet<Utils::FilePath> _files;
    QVector<QRegularExpression> _filters;

    void processQueue();
    void purgeDirectory();
    void traverseDirDeapthFirst(const Utils::FilePath &dir, int max_depth = INT_MAX);
    void traverseDir(const Utils::FilePath &dir);
    bool matchesFilter(const QString &path);
    void purgeFolder(const QString &path);
};

}
}

#endif // ASYNCFOLDERMONITORWORKER_H
