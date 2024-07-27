cmake_minimum_required(VERSION 3.27)
project(MyCppPlayground)

set(CMAKE_CXX_STANDARD 17)

set(LIBS_DIR libs)

find_package(OpenGL REQUIRED)

include_directories(
        .
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
        opengl/GLMain.cpp
)

set(OPENGL_SRC
        ${IMGUI_SRC}
        opengl/wrap/filter/BaseFilter.h
        opengl/wrap/filter/NV21Filter.h
        opengl/wrap/filter/TextureFilter.h
        opengl/wrap/Framebuffer.h
        opengl/wrap/FramebufferPool.h
        opengl/wrap/GLCoord.h
        opengl/wrap/GLUtil.h
        opengl/wrap/Program.h
        opengl/wrap/Texture.h
        opengl/wrap/Viewport.h
        opengl/GLMain.cpp
        opengl/GLRenderer.cpp
)

set(GLFW_INSTALL OFF)
set(GLFW_BUILD_DOCS OFF)
add_subdirectory(${LIBS_DIR}/glfw-3.4)

add_executable(MyCppPlayground
        main.cpp
        Log.cpp
        ${OPENGL_SRC}
)

target_link_libraries(MyCppPlayground
        glfw
        OpenGL::GL
)
