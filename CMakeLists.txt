cmake_minimum_required (VERSION 3.0)

project(ImageSoft)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# SOIL
set(SOIL deps/soil/src)
file(GLOB soil ${SOIL}/image_DXT.c ${SOIL}/image_helper.c ${SOIL}/SOIL.c ${SOIL}/stb_image_aug.c ${SOIL}/test_SOIL.c)
# imGui
file(GLOB imGui deps/imgui/*.cpp deps/imgui/examples/imgui_impl_opengl3.cpp deps/imgui/examples/imgui_impl_glfw.cpp)
#glad
add_executable(ImageSoft src/main.cpp src/ImageWindowState.cpp ${imGui} ${soil} deps/glad/src/glad.c)
target_include_directories(ImageSoft PUBLIC deps/glad/include)
target_include_directories(ImageSoft PUBLIC deps/imgui)
add_definitions(-DIMGUI_IMPL_OPENGL_LOADER_GLAD)
target_include_directories(ImageSoft PUBLIC deps/soil/src)
# GLFW
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(deps/glfw)
target_link_libraries(ImageSoft glfw)

find_package(OpenGL REQUIRED)
if (OPENGL_FOUND)
    target_include_directories(ImageSoft PUBLIC ${OPENGL_INCLUDE_DIR})
    target_link_libraries(ImageSoft ${OPENGL_gl_LIBRARY})
endif(OPENGL_FOUND)






