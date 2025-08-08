cmake_minimum_required(VERSION 3.31)

project("voxel")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

file(GLOB_RECURSE SOURCES "src/*.cpp")
add_executable(${PROJECT_NAME} ${SOURCES} vendor/glad/src/glad.c)

target_compile_options(${PROJECT_NAME} PRIVATE -march=native -O3)

target_compile_definitions(${PROJECT_NAME} PRIVATE ASSETS_DIR="${PROJECT_SOURCE_DIR}/assets/")

include_directories(${PROJECT_NAME} PRIVATE
    include
    vendor/glad/include
    vendor/glm/
    vendor/stb/
    vendor/noise/
)

if(MINGW)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static -static-libgcc -static-libstdc++")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -static -static-libgcc -static-libstdc++")
endif()

find_package(glfw3 CONFIG REQUIRED)
find_package(OpenGL REQUIRED)

target_link_libraries(${PROJECT_NAME} PRIVATE
    OpenGL::GL
    glfw
)