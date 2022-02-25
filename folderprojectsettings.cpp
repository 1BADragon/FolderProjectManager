#include "folderprojectsettings.h"

namespace FolderProjectManager {
namespace Internal {

static FolderProjectSettings default_settings;

ValueRef Value::object()
{
    auto r = Value::make_shared();

    r->_data = Object();
    return r;
}

ValueRef Value::array()
{
    auto r = Value::make_shared();

    r->_data = Array();
    return r;
}

ValueRef Value::integer(long long v)
{
    auto r = Value::make_shared();

    (*r) = v;
    return r;
}

ValueRef Value::real(double d)
{
    auto r = Value::make_shared();

    (*r) = d;
    return r;
}

ValueRef Value::string(const std::string &s)
{
    auto r = Value::make_shared();

    (*r) = s;
    return r;
}

bool Value::has(const std::string &key)
{
    if (_data.index() != 0) {
        return false;
    }

    return std::get<Object>(_data).count(key) > 0;
}

bool Value::has(size_t index)
{
    if (_data.index() != 1) {
        return false;
    }

    return std::get<Array>(_data).size() > index;
}

ValueRef& Value::operator[](size_t index)
{
    return std::get<Array>(_data)[index];
}

ValueRef& Value::operator[](const std::string &key)
{
    return std::get<Object>(_data)[key];
}

Value& Value::operator=(long long v)
{
    this->_data = v;
    return *this;
}

Value& Value::operator=(double d)
{
    this->_data = d;
    return *this;
}

Value& Value::operator=(const std::string &s)
{
    this->_data = s;
    return *this;
}

FolderProjectSettings::FolderProjectSettings()
    : _base()
{

}

ValueRef& FolderProjectSettings::operator[](const std::string &key)
{
    return _base[key];
}

ValueRef& FolderProjectSettings::operator[](const Setting &key)
{
    auto path = key.path();

    Object &o = _base;

    for (size_t i = 0; i < path.size() - 1; ++i) {
        auto &p = path[i];
        auto loc = o.find(p);

        if (loc == o.end()) {
            o[p] = Value::object();
        }

        o = o[p]->get<Object>();
    }

    return o[path.back()];
}

FolderProjectSettings& getDefault()
{
    return default_settings;
}

}
}
