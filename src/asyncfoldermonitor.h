#ifndef __ASYNCFOLDERMONITOR_H__
#define __ASYNCFOLDERMONITOR_H__

#include <QObject>
#include <QThread>
#include <QQueue>
#include <QMutex>

#include <utils/filepath.h>
#include <utils/filesystemwatcher.h>

#include "asyncfoldermonitorworker.h"

namespace FolderProjectManager {
namespace Internal {

class AsyncFolderMonitor : public QObject
{
    Q_OBJECT
public:
    explicit AsyncFolderMonitor(const Utils::FilePath &root,
                                    QObject *parent = nullptr);
    virtual ~AsyncFolderMonitor();

    const QList<Utils::FilePath>& fileList();
    void setFilters(const QStringList &newFilters);

signals:
    void filesChanged();

private slots:
    void filesChangedSlot();

private:
    QThread _thread;
    AsyncFolderMonitorWorker *_worker;

    QList<Utils::FilePath> _last_list;

};

}
}

#endif // RECURSIVEFOLDERMONITOR_H
