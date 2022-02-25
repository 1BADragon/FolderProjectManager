#include "folderprojectsettings.h"

#include <QSettings>

namespace FolderProjectManager {
namespace Internal {

Setting::Setting(const QString &key) :
    _key(key)
{
}

Setting::Setting(const QString &key, const SettingValue &v) :
    _key(key), _default(v)
{
}

QString Setting::key() const
{
    return _key;
}

SettingRegistry::SettingRegistry(QString filename) :
    QSettings(filename)
{

}

SettingValue SettingRegistry::value(const Setting &key) const
{
    if (contains(key.key())) {
        return QSettings::value(key.key());
    }

    return key.def();
}

void SettingRegistry::setValue(const Setting &key, SettingValue v)
{
    QSettings::setValue(key.key(), v);
}


}
}
