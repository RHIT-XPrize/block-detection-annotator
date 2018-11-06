# CMakeList.txt : CMake project for Boost_Python_Build, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

message("EnvironmentVariableName = $ENV{BOOST_ROOT}")
find_package(PythonLibs ${PY_VERSION} REQUIRED)
find_package(Boost 1.68.0 REQUIRED COMPONENTS python)