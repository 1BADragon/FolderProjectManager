#include "folderprojectsettings.h"

namespace FolderProjectManager {
namespace Internal {

static FolderProjectSettings default_settings;

ValueRef Value::object()
{
    auto r = std::make_shared<Value>();

    r->_data = Object();
    return r;
}

ValueRef Value::array()
{
    auto r = std::make_shared<Value>();

    r->_data = Array();
    return r;
}

ValueRef Value::integer(long long v)
{
    auto r = std::make_shared<Value>();

    (*r) = v;
    return r;
}

ValueRef Value::real(double d)
{
    auto r = std::make_shared<Value>();

    (*r) = d;
    return r;
}

ValueRef Value::string(const std::string &s)
{
    auto r = std::make_shared<Value>();

    (*r) = s;
    return r;
}

ValueRef Value::operator[](size_t index)
{
    return std::get<Array>(_data)[index];
}

ValueRef Value::operator[](const std::string &key)
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

FolderProjectSettings& getDefault()
{
    return default_settings;
}

}
}
