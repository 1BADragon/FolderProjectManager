#include "folderprojectsettings.h"

#include <memory>

#include <QJsonDocument>
#include <QJsonValue>
#include <QJsonValueRef>
#include <QTextCodec>

namespace FolderProjectManager {
namespace Internal {

static std::unique_ptr<QJsonObject> default_settings;
static QJsonObject& defaultObject();
static QString formatJsonObject(const QJsonObject &obj);

Setting::Setting(const QString &path, QJsonValue val)
    : _path(path)
{
    defaultObject().insert(path, val);
}

QString Setting::path() const
{
    return _path;
}

bool FolderProjectSettings::setFile(const Utils::FilePath &filepath)
{
    _path = filepath;
    return refresh();
}

QJsonValueRef FolderProjectSettings::operator[](const QString &key)
{
    return resolve(key);
}

QJsonValueRef FolderProjectSettings::operator[](const Setting &key)
{
    return resolve(key.path());
}

QJsonValueRef FolderProjectSettings::resolve(const QString &path)
{
    return _base[path];
}

QString FolderProjectSettings::format() const
{
    return formatJsonObject(_base);
}

void FolderProjectSettings::update()
{
    for (auto &k : default_settings->keys()) {
        if (!_base.contains(k)) {
            _base.insert(k, default_settings->value(k));
        }
    }

    _path.writeFileContents(format().toLocal8Bit());
}

bool FolderProjectSettings::refresh()
{
    auto doc = QJsonDocument::fromJson(_path.fileContents());
    if (!doc.isObject()) {
        return false;
    }

    _base = doc.object();
    return true;
}

void FolderProjectSettings::insert(const QString &key, QJsonValue &val)
{
    _base.insert(key, val);
}


QString defaultSettingDocument()
{
    return formatJsonObject(defaultObject());
}

static QJsonObject& defaultObject()
{
    if (!default_settings) {
        default_settings = std::make_unique<QJsonObject>();
    }

    return *default_settings;
}

static QString formatJsonObject(const QJsonObject &obj)
{
    QJsonDocument d;

    d.setObject(obj);

    return QString::fromUtf8(d.toJson(QJsonDocument::Indented));
}

}
}
