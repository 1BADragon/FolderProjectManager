#ifndef __FOLDERPROJECTSETTINGS_H__
#define __FOLDERPROJECTSETTINGS_H__

#include <QJsonObject>
#include <QJsonArray>
#include <QString>

#include <utils/filepath.h>

namespace FolderProjectManager {
namespace Internal {

using Array = QJsonArray;

class Setting {
public:
    Setting(const QString &path, QJsonValue val);

    QString path() const;

private:
    QString _path;
};

class FolderProjectSettings
{
public:
    FolderProjectSettings() = default;

    bool setFile(const Utils::FilePath &filepath);

    QJsonValueRef operator[](const QString &key);
    QJsonValueRef operator[](const Setting &key);

    QString format() const;

    void update();
    bool refresh();
    void insert(const QString &key, QJsonValue &val);

private:
    QJsonObject _base;
    Utils::FilePath _path;

    QJsonValueRef resolve(const QString &path);
};

QString defaultSettingDocument();

}
}

#endif // FOLDERPROJECTSETTINGS_H
