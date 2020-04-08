cmake_minimum_required(VERSION 3.15)
project(PBSPro VERSION 20.0.0)

include(GNUInstallDirs)
find_package(Python3 COMPONENTS Interpreter Development REQUIRED)

add_subdirectory(src)
