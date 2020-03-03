# 说明

py_helper.hpp定义了将Python嵌入C++所需的内容:

1. Python环境初始化, 定义在PythonEnviron类中
    - 通过类构造与析构函数, 完成Python环境加载和卸载的基础工作
    - 通过Python脚本, 完成部分Python初始化工作
2. SafePyObject, 通过类析构函数, 完成对需要释放引用的PyObject*指针进行引用计数操作
3. pytype_from, pytype_to定义了python数据与C++内部数据转换的接口及示例

#### Python与C++数据转换示例

```cpp
#include "py_helper.hpp"

struct MyData
{
    int i;
    int j;
    std::string name;

    static inline char** kwlists() {
        static char* _kwlists[] = { "i", "j", "name",NULL };
        return _kwlists;
    }
};

std::ostream& operator<<(std::ostream& out, const MyData& data)
{
    out << data.i << " " << data.j << " " << data.name;
    return out;
}

template<>
PyObject* pytype_from(const MyData& value) {
    PyObject* dict = PyDict_New();
    auto kwlists = MyData::kwlists();
    int i = 0;
    py_dict_set_item(dict, kwlists[i++], value.i);
    py_dict_set_item(dict, kwlists[i++], value.j);
    py_dict_set_item(dict, kwlists[i++], value.name);
    return dict;
}

template<>
int pytype_to(PyObject* obj, MyData& value) {
    auto kwlists = MyData::kwlists();
    int i = 0;
    py_dict_get_item(obj, kwlists[i++], value.i);
    py_dict_get_item(obj, kwlists[i++], value.j);
    py_dict_get_item(obj, kwlists[i++], value.name);
    return 0;
}
```