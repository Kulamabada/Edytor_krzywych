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

add_executable(edytor_krzywych
        main.cpp

        Math/IMathProvider.h
        Math/IMathProvider.h
        Math/EigenMathAdapter.cpp
        Math/EigenMathAdapter.h

        Geometry/GeometricObject.h
        Geometry/Curve.h
        Geometry/Point.cpp
        Geometry/Point.h
        Geometry/TransformMatrix.cpp
        Geometry/TransformMatrix.h
        Geometry/InterpolatedCurve.cpp
        Geometry/InterpolatedCurve.h
        Geometry/NaturalSpline.cpp
        Geometry/NaturalSpline.h

        Core/AbstractCommand.h
        Core/ApplicationCore.cpp
        Core/ApplicationCore.h
        Core/Command.cpp
        Core/Command.h
        Core/CommandManager.cpp
        Core/CommandManager.h
        Core/CurveCommand.h

        Graphics/Canvas.cpp
        Graphics/Canvas.h
        Graphics/ViewportManager.cpp
        Graphics/ViewportManager.h
        Graphics/QtRenderer.cpp
        Graphics/QtRenderer.h
)

find_package(Qt6 COMPONENTS Core Gui Widgets REQUIRED)
target_link_libraries(edytor_krzywych PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets)

# Elastyczne podejście do znajdowania Eigen3
find_package(Eigen3 CONFIG QUIET)
if(NOT Eigen3_FOUND)
    set(Eigen3_DIR "${CMAKE_CURRENT_SOURCE_DIR}/eigen-3.4.0/cmake")
    find_package(Eigen3 CONFIG QUIET)
    if(NOT Eigen3_FOUND)
        # Używamy include_directories jako ostateczność
        include_directories("${CMAKE_CURRENT_SOURCE_DIR}/eigen-3.4.0")
        message(STATUS "Using Eigen3 headers directly")
    endif()
endif()