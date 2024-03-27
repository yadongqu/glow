#pragma once
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <string>
#include <vector>
class GLFWwindow;

struct MenuState
{
    std::vector<std::string> renderModes;
    int renderMode = 0;
    std::vector<std::string> modelPath;
    int modelIndex = 0;
    std::vector<std::string> environmentPath;
    int environmentIndex = 0;
};

namespace GUI
{
    void init(GLFWwindow *window);

    void newFrame();

    void endFrame();

    void drawMenu(MenuState &state);

    void drawSelectable(std::vector<std::string> &items, int &index);

    void draw();

    void cleanup();
};