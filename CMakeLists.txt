cmake_minimum_required(VERSION 3.31)

project("voxel")

file(GLOB_RECURSE SOURCES "src/*.cpp")
add_executable(${PROJECT_NAME} ${SOURCES} vendor/glad/src/glad.c)


target_compile_definitions(${PROJECT_NAME} PRIVATE ASSETS_DIR="${PROJECT_SOURCE_DIR}/assets/")

include_directories(${PROJECT_NAME} PRIVATE
    include
    vendor/glad/include
    vendor/glm/
    vendor/stb/
)

find_package(glfw3 CONFIG REQUIRED)
find_package(OpenGL REQUIRED)

target_link_libraries(${PROJECT_NAME} PRIVATE
    OpenGL::GL
    glfw
)