cmake_minimum_required(VERSION 3.20)
project(GammaD3D12 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(GammaD3D12
    main.cpp
)

target_link_options(GammaD3D12 PRIVATE "/SUBSYSTEM:WINDOWS")

target_link_libraries(GammaD3D12 PRIVATE
    d3d12
    dxgi
    dxguid
    user32
    gdi32
)
