cmake_minimum_required(VERSION 3.31)

project("voxel")

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB IMGUI_CORE_SOURCES "vendor/imgui/*.cpp")
file(GLOB_RECURSE SOURCES "src/*.cpp")
add_executable(${PROJECT_NAME} ${SOURCES} vendor/glad/src/glad.c ${IMGUI_CORE_SOURCES} vendor/imgui/backends/imgui_impl_glfw.cpp vendor/imgui/backends/imgui_impl_opengl3.cpp )

target_compile_options(${PROJECT_NAME} PRIVATE -march=native -O3)

target_compile_definitions(${PROJECT_NAME} PRIVATE ASSETS_DIR="${PROJECT_SOURCE_DIR}/assets/")

include_directories(${PROJECT_NAME} PRIVATE
    include
    vendor/glad/include
    vendor/glm/
    vendor/stb/
    vendor/noise/
    vendor/imgui
)

find_package(glfw3 CONFIG REQUIRED)
find_package(OpenGL REQUIRED)

target_link_libraries(${PROJECT_NAME} PRIVATE
    OpenGL::GL
    glfw
    -lstdc++exp
)