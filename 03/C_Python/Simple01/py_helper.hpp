/*
 * Python功能助手, 基础扩展集中在一个文件中
 *
 * 历史
 *  2020-03-03 姚彧 集成
 */
#ifndef __YY_PY_HELPER_H__
#define __YY_PY_HELPER_H__

 // 使用Py_ssize_t(Python推荐)
#define PY_SSIZE_T_CLEAN

//关闭Python调试, 从而在windows下不会链接到调试库
#ifdef _DEBUG
#   undef _DEBUG
#   include <Python.h>
#   define _DEBUG
#else
#   include <Python.h>
#endif
#include <vector>
#include <string>

/*
 * 安全的PyObject, 保证需要的PyObject*在生命同期结束后, 引用减少
 */
struct SafePyObject
{
    PyObject* obj;
    SafePyObject(PyObject* obj = NULL) :obj(obj) {}
    ~SafePyObject() { release(); }
    SafePyObject& operator=(PyObject* obj)
    {
        release();
        this->obj = obj;
        return *this;
    }
    //inline PyObject* operator&() { return obj; }
    inline operator bool() { return obj; }
    inline operator PyObject* () { return obj; }

private:
    void release()
    {
        if (obj)
            Py_XDECREF(obj);
        obj = NULL;
    }
};

/*
 * Python环境
 */
class PythonEnviron
{
public:
    PythonEnviron();
    ~PythonEnviron() { Py_FinalizeEx(); }
};

PythonEnviron::PythonEnviron()
{
    Py_InitializeEx(0);
    static char script[] = R"(
def update_sys_path(py):
    import os
    import sys

    def update_path(base):
        i=3
        while (i>0):
            py_dir = os.path.join(base, py)
            if os.path.isdir(py_dir):
                if py_dir not in sys.path:
                    print("sys.path.append:", py_dir)
                    sys.path.append(py_dir)
                return 1
            i-=1
            base = os.path.dirname(base)

    for base in reversed(sys.path):
        base = os.path.abspath(base)
        if 'python' not in base.lower():
            break
    for base in (base, os.getcwd()):
        if(base and update_path(base)):
            return
    print("%s python plugin not exists." % py)
    sys.exit(0)

update_sys_path("py")
del update_sys_path
import app
    )";
    PyRun_SimpleString(script);
}

/*
 * C_Python数据互转
 */

#define py_tuple_get_item(p, pos, data) pytype_to(PyTuple_GetItem((p), (pos)), data)
#define py_tuple_set_item(p, pos, data) PyTuple_SetItem((p), (pos), pytype_from(data))
#define py_tuple_SetItemString(p, pos, data) PyTuple_SetItem((p), (pos), pytype_from<std::string>(data))

#define py_list_get_item(p, pos, data) pytype_to(PyList_GetItem((p), (pos)), data)
#define py_list_set_item(p, pos, data) PyList_SetItem((p), (pos), pytype_from(data))
#define py_list_SetItemString(p, pos, data) PyList_SetItem((p), (pos), pytype_from<std::string>(data))

#define py_dict_get_item(dict, key, result) pytype_to(PyDict_GetItemString((dict), (key)), result)
#define py_dict_set_item(dict, key, v) PyDict_SetItemString((dict), (key), pytype_from(v))
#define py_dict_SetItemString(dict, key, s) PyDict_SetItemString((dict), (key), pytype_from<std::string>(s))

 // 互转接口
template<typename T>
PyObject* pytype_from(const T& value) {
    static_assert(false, "pytype_from<T> Not implemented!");
}

template<typename T>
int pytype_to(PyObject* obj, T& value) {
    static_assert(false, "pytype_to<T> Not implemented!");
}

/*
 * PyObject*特化
 */
template<>
PyObject* pytype_from(PyObject* const& value) {
    auto res = (PyObject*)value;
    Py_XINCREF(res);
    return res;
}

template<>
int pytype_to(PyObject* obj, PyObject*& value) {
    Py_XINCREF(obj);
    value = obj;
    return 0;
}

/*
 * char* const 特化
 * const char* p;   //指向常字符的指针,const修饰char*
 * char* const p;   //指针是常量, 内容可以改变(++p错误), const修饰p
 */
template<>
PyObject* pytype_from(char* const& value)
{
    return PyUnicode_FromString(value);
}

/*
 * std::string特化
 */
template<>
PyObject* pytype_from(const std::string& value)
{
    return PyUnicode_FromStringAndSize(value.c_str(), value.size());
}

template<>
int pytype_to(PyObject* obj, std::string& value)
{
    if (obj && PyUnicode_Check(obj))
    {
        Py_ssize_t  nLen = 0;
        auto pDest = PyUnicode_AsUTF8AndSize(obj, &nLen);
        value.assign(pDest, nLen);
        return 0;
    }
    return -1;
}

/*
 * long特化
 */
template<>
PyObject* pytype_from(const long& value)
{
    return PyLong_FromLong(value);
}

template<>
int pytype_to(PyObject* obj, long& value)
{
    if (obj && PyLong_Check(obj))
    {
        value = PyLong_AsLong(obj);
        return 0;
    }
    return -1;
}

/*
 * X型整数特化
 */
#define IMPL_INT_CODE(X) \
template<> \
PyObject* pytype_from(const X &value) \
{ \
    return PyLong_FromLong((long)value); \
} \
 \
template<> \
int pytype_to(PyObject* obj, X &value) \
{ \
    if (obj && PyLong_Check(obj)) \
    { \
        value = (X)PyLong_AsLong(obj); \
        return 0; \
    } \
    return -1; \
} \

IMPL_INT_CODE(int)
IMPL_INT_CODE(unsigned int)
IMPL_INT_CODE(short)
IMPL_INT_CODE(unsigned short)
IMPL_INT_CODE(char)
IMPL_INT_CODE(unsigned char)

#ifdef _WIN32
IMPL_INT_CODE(unsigned long)
IMPL_INT_CODE(int64_t)
#else
#ifndef __x86_64__
IMPL_INT_CODE(int64_t)
#endif
IMPL_INT_CODE(uint64_t)
#endif

/*
 * double特化
 */
    template<>
PyObject* pytype_from(const double& value)
{
    return PyFloat_FromDouble(value);
}

template<>
int pytype_to(PyObject* obj, double& value)
{
    if (obj && PyFloat_Check(obj))
    {
        value = PyFloat_AsDouble(obj);
        return 0;
    }
    // long也强制转换成double
    long v;
    if (pytype_to(obj, v) == 0)
    {
        value = (double)v;
        return 0;
    }
    return -1;
}

/*
 * float特化
 */
template<>
PyObject* pytype_from(const float& value)
{
    return PyFloat_FromDouble((double)value);
}

template<>
int pytype_to(PyObject* obj, float& value)
{
    // 使用double特化
    double v;
    if (pytype_to(obj, v) == 0)
    {
        value = (float)v;
        return 0;
    }
    return -1;
}

/*
 * vector<T>特化
 */
template<typename T>
PyObject* pytype_from(const std::vector<T>& value)
{
    PyObject* list = PyList_New(value.size());
    int i = 0;
    for (auto& v : value)
        py_list_set_item(list, i++, v);
    return value;
}

template<typename T>
int pytype_to(PyObject* obj, std::vector<T>& value)
{
    T v;
    value.clear();
    if (obj == NULL)
        return -1;
    int ret = 0;
    if (PyTuple_Check(obj))
    {
        auto n = PyTuple_Size(obj);
        for (decltype(n) i = 0; i < n; ++i)
        {
            if (py_tuple_get_item(obj, i, v)) {
                ret = -1;
                break;
            }
            value.push_back(v);
        }
    }
    else if (PyList_Check(obj))
    {
        auto n = PyList_Size(obj);
        for (decltype(n) i = 0; i < n; ++i)
        {
            if (py_list_get_item(obj, i, v)) {
                ret = -1;
                break;
            }
            value.push_back(v);
        }
    }
    return ret;
}

/*
 * 调用Python的 模块.函数(tag, context字典)
 */
template<int N = 0>
int py_call(const std::string& module_name,
    const std::string& func_name,
    const std::string& tag,
    PyObject* context)
{
    SafePyObject pModuleName, pModule, pFunc;
    SafePyObject pArgs, pValue;

    pModuleName = PyUnicode_DecodeFSDefault(module_name.c_str());
    pModule = PyImport_Import(pModuleName);
    if (pModule)
    {
        pFunc = PyObject_GetAttrString(pModule, func_name.c_str());
        if (pFunc && (PyCallable_Check(pFunc))) {
            pArgs = PyTuple_New(2);
            py_tuple_set_item(pArgs, 0, tag);
            py_tuple_set_item(pArgs, 1, context);
            pValue = PyObject_CallObject(pFunc, pArgs);
        }
        else {
            if (PyErr_Occurred())
                PyErr_Print();
            fprintf(stderr, "Cannot find function \"%s.%s\"\n", module_name.c_str(), func_name.c_str());
        }
    }
    else {
        PyErr_Print();
        fprintf(stderr, "Failed to load '%s'\n", module_name.c_str());
    }

    return 0;
}


#endif //__YY_PY_HELPER_H__