cmake_minimum_required(VERSION 3.15)
project(SerGame)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

include(FetchContent)

FetchContent_Declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG        v1.12.0
)
FetchContent_MakeAvailable(spdlog)

FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG        1.0.1
)
FetchContent_MakeAvailable(glm)

find_package(SDL2 REQUIRED)
find_package(OpenGL REQUIRED)

find_library(EPOXY_LIB epoxy REQUIRED)

include_directories(
    src
    ${SDL2_INCLUDE_DIRS}
    ${OPENGL_INCLUDE_DIR}
    ${spdlog_SOURCE_DIR}/include
    ${glm_SOURCE_DIR}
)

file(GLOB_RECURSE ENGINE_SOURCES 
    "src/SerGame/*.cpp" 
    "src/SerGame/*.c"
)

add_library(SerEngine SHARED ${ENGINE_SOURCES})
target_link_libraries(SerEngine spdlog::spdlog SDL2::SDL2 OpenGL::GL ${EPOXY_LIB})

add_executable(Sandbox "src/Sandbox/SandboxApp.cpp")
target_link_libraries(Sandbox SerEngine)
