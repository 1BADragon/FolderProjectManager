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

    const QList<Utils::FilePath>& list() const;

    void setFilters(const QStringList &newFilters);

signals:
    void filesUpdated();

private slots:
    void fileChanged(const QString &path);
    void directoryChanged(const QString &path);

private:
    Utils::FilePath _root;
    QList<Utils::FilePath> _list;
    Utils::FileSystemWatcher _watcher;
    QStringList _filters;


    void buildFileList();
    void traverseDir(const Utils::FilePath &dir);
};

}
}

#endif // RECURSIVEFOLDERMONITOR_H
