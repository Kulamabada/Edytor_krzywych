cmake_minimum_required(VERSION 3.30)
project(edytor_krzywych LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

if(NOT QT_ROOT AND DEFINED ENV{QT_ROOT})
    set(QT_ROOT "$ENV{QT_ROOT}")
endif()

if(QT_ROOT)
    list(PREPEND CMAKE_PREFIX_PATH "${QT_ROOT}")
elseif(NOT Qt6_DIR AND NOT DEFINED ENV{Qt6_DIR})
    if(MSVC)
        set(_qt_kits "msvc2022_64" "msvc2019_64")
    elseif(WIN32)
        set(_qt_kits "mingw_64")
    elseif(APPLE)
        set(_qt_kits "macos")
    else()
        set(_qt_kits "gcc_64")
    endif()

    set(_qt_candidates "")
    foreach(_base "C:/Qt" "D:/Qt" "$ENV{HOME}/Qt" "$ENV{USERPROFILE}/Qt")
        foreach(_kit IN LISTS _qt_kits)
            file(GLOB _found LIST_DIRECTORIES true "${_base}/6.*/${_kit}")
            list(APPEND _qt_candidates ${_found})
        endforeach()
    endforeach()

    if(_qt_candidates)
        list(SORT _qt_candidates COMPARE NATURAL ORDER DESCENDING)
        list(GET _qt_candidates 0 QT_ROOT)
        list(PREPEND CMAKE_PREFIX_PATH "${QT_ROOT}")
        message(STATUS "Wybrano Qt: ${QT_ROOT}")
    else()
        message(WARNING "Nie znaleziono Qt 6 w typowych katalogach. "
                "Podaj -DQT_ROOT=<ścieżka do kitu Qt>.")
    endif()
endif()

find_package(Qt6 6.5 COMPONENTS Core Gui Widgets REQUIRED)
message(STATUS "Qt ${Qt6_VERSION} (${Qt6_DIR})")




include(FetchContent)
FetchContent_Declare(
        Eigen3
        GIT_REPOSITORY https://gitlab.com/libeigen/eigen.git
        GIT_TAG        3.4.0
        GIT_SHALLOW    TRUE
        SOURCE_SUBDIR  nie_istnieje   # pomija CMakeLists Eigena
        FIND_PACKAGE_ARGS 3.4 NO_MODULE
)
FetchContent_MakeAvailable(Eigen3)

if(NOT TARGET Eigen3::Eigen)
    add_library(Eigen3::Eigen INTERFACE IMPORTED)
    set_target_properties(Eigen3::Eigen PROPERTIES
            INTERFACE_SYSTEM_INCLUDE_DIRECTORIES "${eigen3_SOURCE_DIR}"
            INTERFACE_INCLUDE_DIRECTORIES "${eigen3_SOURCE_DIR}")
endif()




add_executable(edytor_krzywych
        main.cpp

        Math/IMathProvider.h
        Math/EigenMathAdapter.cpp
        Math/EigenMathAdapter.h

        Geometry/GeometricObject.h
        Geometry/Curve.h
        Geometry/Curve.cpp
        Geometry/CurveStyle.h
        Geometry/Point.cpp
        Geometry/Point.h
        Geometry/TransformMatrix.cpp
        Geometry/TransformMatrix.h
        Geometry/InterpolatedCurve.cpp
        Geometry/InterpolatedCurve.h
        Geometry/BezierCurve.cpp
        Geometry/BezierCurve.h
        Geometry/Image.cpp
        Geometry/Image.h

        Core/AbstractCommand.h
        Core/ApplicationCore.cpp
        Core/ApplicationCore.h
        Core/Command.cpp
        Core/Command.h
        Core/CommandManager.cpp
        Core/CommandManager.h
        Core/JsonSerializable.h
        Core/ProjectSerializer.cpp
        Core/ProjectSerializer.h

        Graphics/Canvas.cpp
        Graphics/Canvas.h
        Graphics/ViewportManager.cpp
        Graphics/ViewportManager.h
        Graphics/QtRenderer.cpp
        Graphics/QtRenderer.h

        UI/MainWindow.cpp
        UI/MainWindow.h
        UI/ActionHandler.cpp
        UI/ActionHandler.h
        UI/WeightsPanel.cpp
        UI/WeightsPanel.h
)

target_include_directories(edytor_krzywych PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

if(MSVC)
    target_compile_options(edytor_krzywych PRIVATE /Zc:__cplusplus /permissive- /W4)
else()
    target_compile_options(edytor_krzywych PRIVATE -Wall -Wextra)
endif()

target_link_libraries(edytor_krzywych PRIVATE
        Qt6::Core Qt6::Gui Qt6::Widgets
        Eigen3::Eigen
)

if(WIN32)
    find_program(WINDEPLOYQT_EXECUTABLE windeployqt
            HINTS "${QT_ROOT}/bin" "${Qt6_DIR}/../../../bin")
    if(WINDEPLOYQT_EXECUTABLE)
        add_custom_command(TARGET edytor_krzywych POST_BUILD
                COMMAND "${WINDEPLOYQT_EXECUTABLE}"
                --no-translations --no-system-d3d-compiler
                "$<TARGET_FILE:edytor_krzywych>"
                COMMENT "windeployqt")
    endif()
endif()

# Aplikacja okienkowa na Windows (bez konsoli)
# set_target_properties(edytor_krzywych PROPERTIES WIN32_EXECUTABLE ON)