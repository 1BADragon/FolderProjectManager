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
class Value {
    static ValueRef object();
    static ValueRef array();
    static ValueRef integer(long long v = 0);
    static ValueRef real(double d = 0.f);
    static ValueRef string(const std::string &s = "");


    template<typename T>
    T& get() const {
        return std::get<T>(_data);
    }

    ValueRef operator[](size_t index);
    ValueRef operator[](const std::string &key);

    Value& operator=(long long v);
    Value& operator=(double d);
    Value& operator=(const std::string &s);

private:
    std::variant<Object, Array, long long, double, std::string> _data;
};

class FolderProjectSettings
{
public:
    FolderProjectSettings();

private:
    Object _base;
};

FolderProjectSettings& getDefault();

}
}


#endif // FOLDERPROJECTSETTINGS_H
