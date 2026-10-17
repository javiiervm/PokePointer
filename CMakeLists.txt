cmake_minimum_required(VERSION 3.24)
project(PokePointer VERSION 0.3.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
option(POKEPOINTER_BUILD_APP "Build the Qt Bluetooth CLI" ON)
include(CTest)

function(pokepointer_warnings target)
    if(MSVC)
        target_compile_options(${target} PRIVATE /W4 /permissive- /utf-8)
    else()
        target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic)
    endif()
endfunction()

add_library(pokepointer_core
    src/protocol/InputDecoder.cpp
    src/core/ConnectionState.cpp
    src/core/DeviceSelection.cpp
)
target_include_directories(pokepointer_core PUBLIC src)
pokepointer_warnings(pokepointer_core)

if(POKEPOINTER_BUILD_APP)
    if(WIN32 AND (NOT MSVC OR NOT CMAKE_CXX_COMPILER_ID STREQUAL "MSVC"))
        message(FATAL_ERROR "The Windows Bluetooth application requires MSVC and a matching Qt MSVC build. MinGW is not supported.")
    endif()
    if(WIN32 AND NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
        message(FATAL_ERROR "The initial Windows target requires a 64-bit build. Configure with -A x64.")
    endif()
    find_package(Qt6 6.5 REQUIRED COMPONENTS Core Bluetooth)
    add_library(pokepointer_runtime
        src/ble/BleSession.cpp src/ble/BleSession.h
        src/app/Application.cpp src/app/Application.h
        src/app/DeviceStore.cpp src/app/DeviceStore.h
        src/platform/ConsoleStop.cpp src/platform/ConsoleStop.h
    )
    if(WIN32)
        target_sources(pokepointer_runtime PRIVATE
            src/platform/WindowsMouseOutput.cpp src/platform/WindowsMouseOutput.h)
        target_link_libraries(pokepointer_runtime PRIVATE user32)
    endif()
    set_target_properties(pokepointer_runtime PROPERTIES AUTOMOC ON)
    target_link_libraries(pokepointer_runtime PUBLIC pokepointer_core Qt6::Core Qt6::Bluetooth)
    pokepointer_warnings(pokepointer_runtime)
    add_executable(pokepointer src/main.cpp)
    target_link_libraries(pokepointer PRIVATE pokepointer_runtime)
    pokepointer_warnings(pokepointer)
    if(BUILD_TESTING)
        add_executable(pokepointer_qt_tests tests/qt_tests.cpp)
        target_link_libraries(pokepointer_qt_tests PRIVATE pokepointer_runtime)
        pokepointer_warnings(pokepointer_qt_tests)
        add_test(NAME qt_offline COMMAND pokepointer_qt_tests)
        add_test(NAME cli_help COMMAND pokepointer --help)
        add_test(NAME cli_invalid_combination COMMAND pokepointer --scan --inspect-gatt)
        add_test(NAME cli_invalid_mouse_combination COMMAND pokepointer --scan --mouse)
        set_tests_properties(cli_invalid_combination cli_invalid_mouse_combination PROPERTIES WILL_FAIL TRUE)
    endif()
endif()

if(BUILD_TESTING)
    add_executable(pokepointer_core_tests tests/core_tests.cpp)
    target_link_libraries(pokepointer_core_tests PRIVATE pokepointer_core)
    pokepointer_warnings(pokepointer_core_tests)
    add_test(NAME core COMMAND pokepointer_core_tests)

    add_executable(pokepointer_pointer_tests tests/pointer_tests.cpp)
    target_link_libraries(pokepointer_pointer_tests PRIVATE pokepointer_core)
    pokepointer_warnings(pokepointer_pointer_tests)
    add_test(NAME pointer COMMAND pokepointer_pointer_tests)

    add_executable(pokepointer_gesture_tests tests/gesture_tests.cpp)
    target_link_libraries(pokepointer_gesture_tests PRIVATE pokepointer_core)
    pokepointer_warnings(pokepointer_gesture_tests)
    add_test(NAME gestures COMMAND pokepointer_gesture_tests)
endif()
