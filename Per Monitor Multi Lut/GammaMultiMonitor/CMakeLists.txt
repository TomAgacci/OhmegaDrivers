cmake_minimum_required(VERSION 3.20)
project(GammaMultiMonitor LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(GammaMultiMonitor
    main.cpp
)

target_link_options(GammaMultiMonitor PRIVATE "/SUBSYSTEM:WINDOWS")

target_link_libraries(GammaMultiMonitor PRIVATE
    user32
    gdi32
)
