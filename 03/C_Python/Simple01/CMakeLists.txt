# 设置CMake版本要求
cmake_minimum_required(VERSION 3.0)

# 设置项目
project(C_PYTHON_SIMPLE VERSION 0.1)

# 设置C++版本
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED on)

# 指定Python
if( APPLE )
    set(py_version 3.8)
    set(PYTHON_PATH "/Library/Frameworks/Python.framework/Versions/${py_version}")
    include_directories("${PYTHON_PATH}/include/python${py_version}")
    set(PythonLibName "python${py_version}")

    # 指定链接目录
    link_directories("${PYTHON_PATH}/lib")
else()
    set(py_version 38)
    set(PYTHON_PATH $ENV{PY${py_version}BIN})
    if(NOT DEFINED PYTHON_PATH)
        message(FATAL "PY${py_version}执行目录未设置")
    endif()
    include_directories("${PYTHON_PATH}/include")
    set(PythonLibName "python${py_version}")

    # 指定链接目录
    link_directories("${PYTHON_PATH}/libs")
endif()


# 生成执行程序
set(EXE_NAME main)
aux_source_directory(${PROJECT_SOURCE_DIR} SRC_LIST)
add_executable(${EXE_NAME} ${SRC_LIST})
target_link_libraries(${EXE_NAME} ${PythonLibName})