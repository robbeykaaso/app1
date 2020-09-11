#include "py.h"
#include <Python.h>
#include <iostream>

//https://www.cnblogs.com/findumars/p/6142330.html
void testPython(){
    system("python plugin/test34.py \"D:/mywork/build-app-Desktop_Qt_5_12_2_MSVC2015_64bit-Release/deepsight/project/8D71C8A1-0E43-4f3f-9B87-792B62C7ABB1/image/0F9D79DE-24F9-44e3-A09F-FB99A19B6B5C/V1-13.bmp\"");
    return;
    Py_Initialize();

    std::string chdir_cmd = "sys.path.append('./plugin/')";
    const char* cstr_cmd = chdir_cmd.c_str();
    PyRun_SimpleString("import sys");
    PyRun_SimpleString(cstr_cmd);

    PyObject* moduleName = PyUnicode_FromString("test34"); //模块名，不是文件名
    PyObject* pModule = PyImport_Import(moduleName);
    if (!pModule) // 加载模块失败
    {
        std::cout << "[ERROR] Python get module failed." << std::endl;
        return;
    }
    std::cout << "[INFO] Python get module succeed." << std::endl;

    // 加载函数
    PyObject* pv = PyObject_GetAttrString(pModule, "binarization");
    if (!pv || !PyCallable_Check(pv))  // 验证是否加载成功
    {
        std::cout << "[ERROR] Can't find function (test)" << std::endl;
        return;
    }
    std::cout << "[INFO] Get function (test) succeed." << std::endl;

    // 设置参数
    PyObject* args = PyTuple_New(2);   // 2个参数
    //Py_BuildValue("s", "D:/mywork/build-app-Desktop_Qt_5_12_2_MSVC2015_64bit-Release/deepsight/project/8D71C8A1-0E43-4f3f-9B87-792B62C7ABB1/image/0F9D79DE-24F9-44e3-A09F-FB99A19B6B5C/V1-13.bmp");
    // 调用函数

    PyObject* pRet = PyObject_CallObject(pv, args);

    Py_Finalize();
}
