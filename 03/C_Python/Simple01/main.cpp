#include <iostream>
#include "py_helper.hpp"
#include "mydata.hpp"

void test_mydata();

int main()
{
    PythonEnviron py_environ;
    test_mydata();
    std::cout << "测试" << std::endl;
    return 0;
}

void test_mydata()
{
    MyData data{ 1,2,"张三" };
    SafePyObject py_data = pytype_from(data);
    std::cout << "Before: " << data << std::endl;
    py_call("app", "test", "test", py_data.obj);
    pytype_to(py_data, data);
    std::cout << "After: " << data << std::endl;
}