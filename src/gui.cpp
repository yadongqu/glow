#include "gui.h"
#include <GLFW/glfw3.h>

namespace GUI
{
    void init(GLFWwindow *window)
    {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO &io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableSetMousePos;
        ImGui::StyleColorsDark();
        ImGui_ImplGlfw_InitForOpenGL(window, true);
        ImGui_ImplOpenGL3_Init("#version 460");
    }

    void newFrame()
    {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
    }

    void endFrame()
    {
        ImGui::End();
        ImGui::Render();
    }

    void draw()
    {
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    void cleanup()
    {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }

    void drawMenu(MenuState &state)
    {
        ImGui::Begin("Settings");
        ImGui::Text("Model: ");
        drawSelectable(state.modelPath, state.modelIndex);
        ImGui::Text("Environment: ");
        drawSelectable(state.environmentPath, state.environmentIndex);
        ImGui::Text("Render Mode: ");
        drawSelectable(state.renderModes, state.renderMode);
    }

    void drawSelectable(std::vector<std::string> &items, int &index)
    {

        if (ImGui::BeginCombo("##combo", items[index].c_str())) // The second parameter is the label previewed before opening the combo.
        {
            for (int n = 0; n < items.size(); n++)
            {
                bool is_selected = (index == n); // You can store your selection however you want, outside or inside your objects
                if (ImGui::Selectable(items[n].c_str(), is_selected))
                    index = n;
                if (is_selected)
                    ImGui::SetItemDefaultFocus(); // You may set the initial focus when opening the combo (scrolling + for keyboard navigation support)
            }
            ImGui::EndCombo();
        }
    }

} // namespace gui
