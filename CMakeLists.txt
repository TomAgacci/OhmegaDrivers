cmake_minimum_required(VERSION 3.20)
project(GammaWinUI LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Windows App SDK
find_package(Microsoft.WindowsAppSDK 1.5 REQUIRED)
find_package(Microsoft.UI.Xaml REQUIRED)

add_executable(GammaWinUI
    App.xaml.cpp
    App.xaml
    MainWindow.xaml.cpp
    MainWindow.xaml
    Gamma.cpp
    Gamma.h
)

target_link_libraries(GammaWinUI PRIVATE
    Microsoft.WindowsAppSDK::Microsoft.WindowsAppSDK
    Microsoft.UI.Xaml::Microsoft.UI.Xaml
    user32
    gdi32
    comctl32
)

target_link_options(GammaWinUI PRIVATE "/SUBSYSTEM:WINDOWS")
