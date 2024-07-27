cmake_minimum_required(VERSION 3.27)
project(MyCppPlayground)

set(CMAKE_CXX_STANDARD 17)

find_package(OpenGL REQUIRED)

include_directories(
        opengl/glfw-3.4/include
        opengl/imgui-1.90.9
        opengl/imgui-1.90.9/backends
)

set(IMGUI_SRC
        opengl/imgui-1.90.9/imgui.cpp
        opengl/imgui-1.90.9/imgui_demo.cpp
        opengl/imgui-1.90.9/imgui_draw.cpp
        opengl/imgui-1.90.9/imgui_tables.cpp
        opengl/imgui-1.90.9/imgui_widgets.cpp
        opengl/imgui-1.90.9/backends/imgui_impl_glfw.cpp
        opengl/imgui-1.90.9/backends/imgui_impl_opengl3.cpp
        opengl/GLMain.cpp
)

set(OPENGL_SRC
        ${IMGUI_SRC}
        opengl/GLMain.cpp
        opengl/GLRenderer.cpp
)

set(GLFW_INSTALL OFF)
set(GLFW_BUILD_DOCS OFF)
add_subdirectory(opengl/glfw-3.4)

add_executable(MyCppPlayground
        main.cpp
        ${OPENGL_SRC}
)

target_link_libraries(MyCppPlayground
        glfw
        OpenGL::GL
)
