cmake_minimum_required(VERSION 3.20 FATAL_ERROR)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")
include(FetchContent)

set(NAME PathTracer)
project(${NAME} LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ---- Solution Explorer organization ------------------------------
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER "CMake")

# ------------------------------------------------------------------
# Vulkan SDK
# ------------------------------------------------------------------
if (NOT IS_DIRECTORY $ENV{VULKAN_SDK})
    message(WARNING "Vulkan SDK not detected, if installed please set VULKAN_SDK env var or manually provide libraries")
else()
    message(STATUS "Found Vulkan SDK in $ENV{VULKAN_SDK}")
    list(APPEND CMAKE_PREFIX_PATH "$ENV{VULKAN_SDK}")
endif()

find_package(Vulkan REQUIRED)

# ------------------------------------------------------------------
# Dependencies
# ------------------------------------------------------------------
find_package(SDL3 CONFIG REQUIRED)

list(APPEND CMAKE_PREFIX_PATH "C:/Program Files/KTX-Software")
find_package(Ktx CONFIG REQUIRED)

add_library(slang SHARED IMPORTED)
set_target_properties(slang PROPERTIES
    IMPORTED_LOCATION   "$ENV{VULKAN_SDK}/Bin/slang.dll"
    IMPORTED_IMPLIB     "$ENV{VULKAN_SDK}/Lib/slang.lib"
    INTERFACE_INCLUDE_DIRECTORIES "$ENV{VULKAN_SDK}/include")

# ------------------------------------------------------------------
# Engine (static library)
# ------------------------------------------------------------------
file(GLOB_RECURSE ENGINE_SOURCES CONFIGURE_DEPENDS
    ${CMAKE_SOURCE_DIR}/engine/*.cpp
    ${CMAKE_SOURCE_DIR}/engine/*.h
    ${CMAKE_SOURCE_DIR}/engine/*.hpp)

set(VOLK_SOURCE $ENV{VULKAN_SDK}/include/Volk/volk.c)

add_library(engine STATIC ${ENGINE_SOURCES} ${VOLK_SOURCE})

target_include_directories(engine
    PUBLIC
        ${CMAKE_SOURCE_DIR}/engine
    SYSTEM PUBLIC
        $ENV{VULKAN_SDK}/include
        ${CMAKE_SOURCE_DIR}/external/tinygltf)

target_compile_definitions(engine PUBLIC
    VK_NO_PROTOTYPES
    VMA_STATIC_VULKAN_FUNCTIONS=0
    VMA_DYNAMIC_VULKAN_FUNCTIONS=1
    GLM_FORCE_DEPTH_ZERO_TO_ONE
    GLM_FORCE_RADIANS
    SHADER_DIR="${CMAKE_SOURCE_DIR}/shaders"
    SCENE_DIR="${CMAKE_SOURCE_DIR}/scenes")

target_link_libraries(engine PUBLIC
    Vulkan::Headers
    SDL3::SDL3
    KTX::ktx
    slang)

# ---- Mirror the engine/ folder tree in Solution Explorer ----
source_group(TREE ${CMAKE_SOURCE_DIR}/engine PREFIX "engine" FILES ${ENGINE_SOURCES})
source_group("external" FILES ${VOLK_SOURCE})
set_target_properties(engine PROPERTIES FOLDER "Engine")

# ------------------------------------------------------------------
# Shaders (browsable only; Slang compiles them at runtime)
# ------------------------------------------------------------------
file(GLOB_RECURSE SHADER_FILES CONFIGURE_DEPENDS
    ${CMAKE_SOURCE_DIR}/shaders/*.slang)

add_custom_target(shaders SOURCES ${SHADER_FILES})
source_group(TREE ${CMAKE_SOURCE_DIR}/shaders PREFIX "shaders" FILES ${SHADER_FILES})
set_target_properties(shaders PROPERTIES FOLDER "Engine")

# ------------------------------------------------------------------
# Application
# ------------------------------------------------------------------
file(GLOB_RECURSE APP_SOURCES CONFIGURE_DEPENDS
    ${CMAKE_SOURCE_DIR}/app/*.cpp
    ${CMAKE_SOURCE_DIR}/app/*.h)

add_executable(${NAME} ${APP_SOURCES})
target_link_libraries(${NAME} PRIVATE engine)

source_group(TREE ${CMAKE_SOURCE_DIR}/app PREFIX "app" FILES ${APP_SOURCES})
set_target_properties(${NAME} PROPERTIES
    FOLDER "App"
    VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")
set_property(DIRECTORY ${CMAKE_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${NAME})

if (WIN32)
    add_custom_command(TARGET ${NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:KTX::ktx>
            $<TARGET_FILE:SDL3::SDL3>
            $<TARGET_FILE:slang>
            $<TARGET_FILE_DIR:${NAME}>
        COMMAND_EXPAND_LISTS)
endif()

# ------------------------------------------------------------------
# Tools (optional)
# ------------------------------------------------------------------
if (EXISTS ${CMAKE_SOURCE_DIR}/tools/CMakeLists.txt)
    add_subdirectory(tools)
endif()