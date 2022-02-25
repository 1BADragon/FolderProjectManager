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

private:
    Utils::FilePath _root;
    std::list<Utils::FilePath> _list;

    void buildFileList();
    void traverseDir(const Utils::FilePath &dir);
};

}
}

#endif // RECURSIVEFOLDERMONITOR_H
