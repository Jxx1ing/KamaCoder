cmake_minimum_required(VERSION 3.15)
# 设置CMake最低版本要求为3.15

project(PerCacheTest)
# 定义项目名称为PerCacheTest

set(CMAKE_CXX_STANDARD 17)
# 设置C++语言标准为C++17

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../bin)
# 设置可执行文件输出路径为当前源目录上一级的bin目录

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../include)
# 添加包含目录，包含当前源目录上一级的include目录

add_executable(testKLruCache testKLruCache.cc)
add_executable(testKLruKCache testKLruKCache.cc)
add_executable(testKHashLruCaches testKHashLruCaches.cc)
# 添加名为testKLruCache的可执行文件，源文件为testKLruCache.cc
# 添加名为testKLruKCache的可执行文件，源文件为testKLruKCache.cc
# 添加名为testKHashLruCaches的可执行文件，源文件为testKHashLruCaches.cc

