cmake_minimum_required(VERSION 3.27)
project(MyCppPlayground)

set(CMAKE_CXX_STANDARD 17)

find_package(OpenGL REQUIRED)
find_package(OpenCV REQUIRED)

set(InspireFacePath ${CMAKE_SOURCE_DIR}/libs/inspireface-macos/InspireFace)
include_directories(${InspireFacePath}/include)
set(LibInspireFace ${InspireFacePath}/lib/libInspireFace.dylib)

include_directories(
        src
        libs/tinyformat
        libs/glfw-3.4/include
        libs/imgui-1.90.9
        libs/imgui-1.90.9/backends
)

set(IMGUI_SRC
        libs/imgui-1.90.9/imgui.cpp
        libs/imgui-1.90.9/imgui_demo.cpp
        libs/imgui-1.90.9/imgui_draw.cpp
        libs/imgui-1.90.9/imgui_tables.cpp
        libs/imgui-1.90.9/imgui_widgets.cpp
        libs/imgui-1.90.9/backends/imgui_impl_glfw.cpp
        libs/imgui-1.90.9/backends/imgui_impl_opengl3.cpp
)

set(OPENGL_SRC
        ${IMGUI_SRC}
        src/opengl/wrap/filter/BaseFilter.h
        src/opengl/wrap/filter/NV21Filter.h
        src/opengl/wrap/filter/TextureFilter.h
        src/opengl/wrap/filter/FaceMorphFilter.h
        src/opengl/wrap/Framebuffer.h
        src/opengl/wrap/FramebufferPool.h
        src/opengl/wrap/GLCoord.h
        src/opengl/wrap/GLUtil.h
        src/opengl/wrap/Program.h
        src/opengl/wrap/Texture.h
        src/opengl/wrap/Viewport.h
        src/opengl/GLMain.cpp
        src/opengl/GLRenderer.cpp
        src/opengl/GLFaceMorph.cpp
)

set(GLFW_INSTALL OFF)
set(GLFW_BUILD_DOCS OFF)
add_subdirectory(libs/glfw-3.4)

set(FACE_SRC
        src/face/detect/InspireFaceDetector.cpp
        src/face/morph/FaceMorphTest.cpp
)

add_executable(MyCppPlayground
        src/main.cpp
        src/Log.cpp
        src/utils/Delaunator.cpp
        ${OPENGL_SRC}
        ${FACE_SRC}
)

target_link_libraries(MyCppPlayground
        glfw
        OpenGL::GL
        ${OpenCV_LIBS}
        ${LibInspireFace}
)
