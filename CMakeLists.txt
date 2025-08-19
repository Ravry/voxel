cmake_minimum_required(VERSION 3.31)

project("voxel")

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(glfw3 CONFIG REQUIRED)
find_package(OpenGL REQUIRED)

include(FetchContent)

# ---- JoltPhysics ----
FetchContent_Declare(
        JoltPhysics
        GIT_REPOSITORY "https://github.com/jrouwe/JoltPhysics"
        GIT_TAG "v5.3.0"
        SOURCE_SUBDIR "Build"
)
FetchContent_MakeAvailable(JoltPhysics)

# ---- GLM ----
FetchContent_Declare(
        GLM
        GIT_REPOSITORY "https://github.com/g-truc/glm"
        GIT_TAG "1.0.0"
)
FetchContent_MakeAvailable(GLM)

# ---- Assimp ----
FetchContent_Declare(
        ASSIMP
        GIT_REPOSITORY https://github.com/assimp/assimp.git
        GIT_TAG "v5.4.3"
)
set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_ZLIB ON CACHE BOOL "" FORCE)
set(ASSIMP_INSTALL OFF CACHE BOOL "" FORCE)
set(ASSIMP_WARNINGS_AS_ERRORS OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(ASSIMP)

# ---- ImGui ----
FetchContent_Declare(
        imgui
        GIT_REPOSITORY https://github.com/ocornut/imgui.git
        GIT_TAG "v1.92.1"
)

FetchContent_MakeAvailable(imgui)

file(GLOB_RECURSE SOURCES "src/*.cpp")
add_executable(${PROJECT_NAME} ${SOURCES} vendor/glad/src/glad.c)

target_sources(${PROJECT_NAME} PRIVATE
        ${imgui_SOURCE_DIR}/imgui.cpp
        ${imgui_SOURCE_DIR}/imgui_draw.cpp
        ${imgui_SOURCE_DIR}/imgui_tables.cpp
        ${imgui_SOURCE_DIR}/imgui_widgets.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)

include_directories(${PROJECT_NAME} PRIVATE
        include/
        vendor/glad/include/
        vendor/stb/
        vendor/noise/
        ${imgui_SOURCE_DIR}
        ${imgui_SOURCE_DIR}/backends
        ${JoltPhysics_SOURCE_DIR}/
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    -lstdc++exp
    OpenGL::GL
    glfw
    glm::glm
    assimp::assimp
    Jolt
)

target_compile_options(${PROJECT_NAME} PRIVATE -march=native -O3)
target_compile_definitions(${PROJECT_NAME} PRIVATE ASSETS_DIR="${PROJECT_SOURCE_DIR}/assets/")

if(MINGW)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libgcc -static-libstdc++")
#    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static")
endif()