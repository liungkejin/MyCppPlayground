//
// Created by LiangKeJin on 2024/7/27.
//
#include "GLRenderer.h"
#include "wrap/filter/TextureFilter.h"
#include "wrap/filter/NV21Filter.h"

using namespace wuta;

ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
uint8_t pixels[100*100*4] = {
        0, 255, 0, 255,
        255, 255, 0, 255,
        0, 0, 0, 255,
        255, 0, 0, 255};

NV21Filter nv21Filter;
TextureFilter filter;
Texture2D texture2D(100,100);
GLRect rect;

int i = 0;
float rotate = 0;
void GLRenderer::onRender(int width, int height) {
    GLUtil::clearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
//    memset(pixels+4*100*10, 128, 100*4*50);
    pixels[i] = 255;
    i = (i+1)%(40000);
    texture2D.update(pixels);
    filter.setViewport(width, height);
    filter.inputTexture(texture2D);
    rect.setRect(0, 0, 500, 500);
//    rect.scale(0.5, 0.5);
    rect.translate(width/2 - 250, height/2 - 250);
    rect.setRotation(rotate);
    filter.setVertexCoord(rect, width, height);
    filter.render();
}

bool show_demo_window = false;
void GLRenderer::onRenderImgui(int width, int height, ImGuiIO& io) {
    // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
    if (show_demo_window)
        ImGui::ShowDemoWindow(&show_demo_window);

    // 2. Show a simple window that we create ourselves. We use a Begin/End pair to create a named window.
    {
        static int counter = 0;

        ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

        ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
        ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state

        ImGui::SliderFloat("rotate", &rotate, 0.0f, 360.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
        ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

        if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
            counter++;
        ImGui::SameLine();
        ImGui::Text("counter = %d", counter);

        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
        ImGui::End();
    }
}

void GLRenderer::onPostRender(int width, int height) {
    //
}

void GLRenderer::onExit() {
    filter.release();
    texture2D.release();
}