#ifndef FOLDERPROJECTSETTINGS_H
#define FOLDERPROJECTSETTINGS_H

#include <variant>
#include <vector>
#include <unordered_map>
#include <string>
#include <memory>

namespace FolderProjectManager {
namespace Internal {

class Value;
using ValueRef = std::shared_ptr<Value>;
using Object = std::unordered_map<std::string, ValueRef>;
using Array = std::vector<ValueRef>;

class Setting {
public:
    Setting(const std::string &path, long long v);
    Setting(const std::string &path, double v);
    Setting(const std::string &path, const std::string &v);
    Setting(const std::string &path, const Array &v);

    std::vector<std::string> path() const;

private:
    std::vector<std::string> parse_path(const std::string &path) const;

    std::vector<std::string> _path;
};

class Value {
public:
    static ValueRef make_shared();
    static ValueRef object();
    static ValueRef array();
    static ValueRef integer(long long v = 0);
    static ValueRef real(double d = 0.f);
    static ValueRef string(const std::string &s = "");

    template<typename T>
    T& get() const {
        return std::get<T>(_data);
    }

    bool has(const std::string &key);
    bool has(size_t index);

    template<typename T>
    bool is() {
        return std::get_if<T>(&_data) != NULL;
    }

    ValueRef& operator[](size_t index);
    ValueRef& operator[](const std::string &key);

    Value& operator=(long long v);
    Value& operator=(double d);
    Value& operator=(const std::string &s);

    template<typename T>
    Value& operator=(const T &v)
    {
        _data = v;
    }

private:
    std::variant<Object, Array, long long, double, std::string> _data;
};

class FolderProjectSettings
{
public:
    FolderProjectSettings();

    ValueRef& operator[](const std::string &key);
    ValueRef& operator[](const Setting &key);

private:
    Object _base;
};

FolderProjectSettings& getDefault();

}
}

#endif // FOLDERPROJECTSETTINGS_H
