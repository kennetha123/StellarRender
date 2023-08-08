cmake_minimum_required(VERSION 3.10)

project(Stellar VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)


file(GLOB_RECURSE CODE "code/src/*.cpp")
include_directories(${CMAKE_SOURCE_DIR}/code/headers)

add_library(Stellar SHARED ${CODE})

target_link_libraries(Stellar d3d11.lib dxgi.lib D3DCompiler.lib)

target_compile_definitions(Stellar PRIVATE BUILDING_STELLAR_DLL)
set_target_properties(Stellar PROPERTIES
    WINDOWS_EXPORT_ALL_SYMBOLS YES
)