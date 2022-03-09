#include "asyncfoldermonitor.h"

#include <QApplication>
#include <QQueue>
#include <QRegExp>
#include <QDebug>

namespace FolderProjectManager {
namespace Internal {

AsyncFolderMonitor::AsyncFolderMonitor(const Utils::FilePath &root, QObject *parent)
    : QObject{parent}, _thread()
{
    _thread.setObjectName("AsyncWorkerThread");
    _worker = new AsyncFolderMonitorWorker(root);

    _worker->moveToThread(&_thread);
    connect(_worker, &AsyncFolderMonitorWorker::filesChanged,
            this, &AsyncFolderMonitor::filesChangedSlot);

    _thread.start();
}

AsyncFolderMonitor::~AsyncFolderMonitor()
{
    _thread.quit();
    _thread.wait();

    delete _worker;
}

const QList<Utils::FilePath>& AsyncFolderMonitor::fileList()
{
    auto list = _worker->currentFileList();

    if (list) {
        _last_list = std::move(*list);
    }

    return _last_list;
}

void AsyncFolderMonitor::setFilters(const QStringList &newFilters)
{
    _worker->setFilters(newFilters);
}

void AsyncFolderMonitor::filesChangedSlot() {
    emit filesChanged();
}

}
}
