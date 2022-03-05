#include "asyncfoldermonitor.h"

#include <QApplication>
#include <QQueue>
#include <QRegExp>

namespace FolderProjectManager {
namespace Internal {

AsyncFolderMonitor::AsyncFolderMonitor(const Utils::FilePath &root, QObject *parent)
    : QObject{parent}, _thread()
{
    _worker = new AsyncFolderMonitorWorker(root);

    _worker->moveToThread(&_thread);
    connect(&_thread, &QThread::finished, _worker, &QObject::deleteLater);
    connect(_worker, &AsyncFolderMonitorWorker::filesChanged,
            this, &AsyncFolderMonitor::filesChangedSlot);

    _thread.start();
}

AsyncFolderMonitor::~AsyncFolderMonitor()
{
    _thread.quit();
    _thread.wait();
}

QList<Utils::FilePath> AsyncFolderMonitor::fileList() const
{
    return _worker->currentFileList();
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
