cmake_minimum_required(VERSION 3.10)

project(Stellar VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(DIRECTX_ROOT "C:/Program Files (x86)/Microsoft DirectX SDK (June 2010)")
include_directories(${DIRECTX_ROOT}/Include)
link_directories(${DIRECTX_ROOT}/Lib/x64)

file(GLOB_RECURSE CODE "code/src/*.cpp")
include_directories(${CMAKE_SOURCE_DIR}/code/headers)

add_library(Stellar SHARED ${CODE})

target_link_libraries(Stellar d3d11.lib dxgi.lib)
