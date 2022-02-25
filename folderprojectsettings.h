#ifndef FOLDERPROJECTSETTINGS_H
#define FOLDERPROJECTSETTINGS_H

#include <QSettings>
#include <QVariant>
#include <QString>

namespace FolderProjectManager {
namespace Internal {

using SettingValue = QVariant;

class Setting {
public:
    Setting(const QString &key);
    Setting(const QString &key, const SettingValue &v);

private:
    friend class SettingRegistry;

    QString key() const;
    QVariant def() const;

    QString _key;
    QVariant _default;
};

class SettingRegistry : public QSettings {
public:
    SettingRegistry(QString filename);

    static SettingRegistry& getDefault();

    SettingValue value(const Setting &key) const;
    void setValue(const Setting &key, SettingValue v);

private:

};

}
}

#endif // FOLDERPROJECTSETTINGS_H
