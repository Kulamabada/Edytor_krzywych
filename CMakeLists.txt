cmake_minimum_required(VERSION 3.30)
project(edytor_krzywych)

list(APPEND CMAKE_PREFIX_PATH "C:/Qt/6.9.0/mingw_64")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    add_compile_options(/Zc:__cplusplus)
endif()
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

add_executable(edytor_krzywych main.cpp
        IMathProvider.h
        GeometricObject.h
        TransformMatrix.h
        AbstractCommand.h)

find_package(Qt6 COMPONENTS Core Gui Widgets REQUIRED)

target_link_libraries(edytor_krzywych PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets)

