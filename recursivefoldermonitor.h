#ifndef RECURSIVEFOLDERMONITOR_H
#define RECURSIVEFOLDERMONITOR_H

#include <QObject>
#include <list>

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

    const std::list<Utils::FilePath>& list() const;

signals:
    void filesUpdated();

private slots:
    void fileChanged(const QString &path);
    void directoryChanged(const QString &path);

private:
    Utils::FilePath _root;
    std::list<Utils::FilePath> _list;
    Utils::FileSystemWatcher _watcher;

    void buildFileList();
    void traverseDir(const Utils::FilePath &dir);
};

}
}

#endif // RECURSIVEFOLDERMONITOR_H
