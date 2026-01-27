cmake_minimum_required(VERSION 3.21)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if ("${CMAKE_SYSTEM_NAME}" STREQUAL "iOS" OR IOS)
    set(CMAKE_OSX_ARCHITECTURES "arm64")
else()
    set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64")
endif()
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(GEODE_DISABLE_PRECOMPILED_HEADERS ON)

project(SendDB VERSION 1.0.0)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS src/*.cpp)
add_library(${PROJECT_NAME} SHARED ${SOURCES})

target_include_directories(SendDB PRIVATE ${CMAKE_SOURCE_DIR}/src ${CMAKE_SOURCE_DIR}/include)

if (PROJECT_IS_TOP_LEVEL)
    target_compile_definitions(${PROJECT_NAME} PRIVATE AWCW_HORRIBLE_API_EXPORTING)
endif()

if (NOT DEFINED ENV{GEODE_SDK})
    message(FATAL_ERROR "Unable to find Geode SDK! Please define GEODE_SDK environment variable to point to Geode")
else()
    message(STATUS "Found Geode: $ENV{GEODE_SDK}")
endif()

add_subdirectory($ENV{GEODE_SDK} ${CMAKE_CURRENT_BINARY_DIR}/geode)

if (GEODE_TARGET_PLATFORM STREQUAL "Win64")
    target_link_libraries(${PROJECT_NAME} opengl32)
elseif (GEODE_TARGET_PLATFORM STREQUAL "Android32")
    target_link_libraries(${PROJECT_NAME} GLESv3)
elseif (GEODE_TARGET_PLATFORM STREQUAL "Android64")
    target_link_libraries(${PROJECT_NAME} GLESv3)
elseif (GEODE_TARGET_PLATFORM STREQUAL "MacOS")
    target_link_libraries(${PROJECT_NAME} "-framework OpenGL -framework Cocoa")
elseif (GEODE_TARGET_PLATFORM STREQUAL "iOS")
    target_link_libraries(${PROJECT_NAME} "-framework OpenGLES")
endif()

CPMAddPackage("gh:prevter/rock#6a2d29e")
CPMAddPackage("gh:camila314/UIBuilder#2ab80ad")
CPMAddPackage("gh:dankmeme01/cue#211f644")
CPMAddPackage("gh:hanickadot/compile-time-regular-expressions#eb9577a")
target_link_libraries(${PROJECT_NAME} rock UIBuilder cue ctre)

setup_geode_mod(${PROJECT_NAME})