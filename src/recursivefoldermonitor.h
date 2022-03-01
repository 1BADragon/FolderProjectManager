#ifndef RECURSIVEFOLDERMONITOR_H
#define RECURSIVEFOLDERMONITOR_H

#include <QObject>
#include <QList>

#include <utils/filepath.h>
#include <utils/filesystemwatcher.h>

namespace FolderProjectManager {
namespace Internal {

class RecursiveFolderMonitor : public QObject
{
    Q_OBJECT
public:
    explicit RecursiveFolderMonitor(const Utils::FilePath &root,
                                    QObject *parent = nullptr);

    const QList<Utils::FilePath> list() const;

    void setFilters(const QStringList &newFilters);

signals:
    void filesChanged();

private slots:
    void fileChanged(const QString &path);
    void directoryChanged(const QString &path);

private:
    Utils::FilePath _root;
    QSet<Utils::FilePath> _files;
    Utils::FileSystemWatcher _watcher;
    QStringList _filters;

    void buildFileList();
    void traverseDirDeapthFirst(const Utils::FilePath &dir, int max_depth = INT_MAX);
    void traverseDirBreathFirst(const Utils::FilePath &dir);
    bool matchesFilter(const QString &path);
};

}
}

#endif // RECURSIVEFOLDERMONITOR_H
