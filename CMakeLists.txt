cmake_minimum_required(VERSION 3.27)
project(MyCppPlayground)

set(CMAKE_CXX_STANDARD 17)

set(LIBS_DIR libs)

find_package(OpenGL REQUIRED)

include_directories(
        src
        ${LIBS_DIR}/tinyformat
        ${LIBS_DIR}/glfw-3.4/include
        ${LIBS_DIR}/imgui-1.90.9
        ${LIBS_DIR}/imgui-1.90.9/backends
)

set(IMGUI_SRC
        ${LIBS_DIR}/imgui-1.90.9/imgui.cpp
        ${LIBS_DIR}/imgui-1.90.9/imgui_demo.cpp
        ${LIBS_DIR}/imgui-1.90.9/imgui_draw.cpp
        ${LIBS_DIR}/imgui-1.90.9/imgui_tables.cpp
        ${LIBS_DIR}/imgui-1.90.9/imgui_widgets.cpp
        ${LIBS_DIR}/imgui-1.90.9/backends/imgui_impl_glfw.cpp
        ${LIBS_DIR}/imgui-1.90.9/backends/imgui_impl_opengl3.cpp
)

set(OPENGL_SRC
        ${IMGUI_SRC}
        src/opengl/wrap/filter/BaseFilter.h
        src/opengl/wrap/filter/NV21Filter.h
        src/opengl/wrap/filter/TextureFilter.h
        src/opengl/wrap/Framebuffer.h
        src/opengl/wrap/FramebufferPool.h
        src/opengl/wrap/GLCoord.h
        src/opengl/wrap/GLUtil.h
        src/opengl/wrap/Program.h
        src/opengl/wrap/Texture.h
        src/opengl/wrap/Viewport.h
        src/opengl/GLMain.cpp
        src/opengl/GLRenderer.cpp
)

set(GLFW_INSTALL OFF)
set(GLFW_BUILD_DOCS OFF)
add_subdirectory(${LIBS_DIR}/glfw-3.4)

add_executable(MyCppPlayground
        src/main.cpp
        src/Log.cpp
        ${OPENGL_SRC}
)

target_link_libraries(MyCppPlayground
        glfw
        OpenGL::GL
)
