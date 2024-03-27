#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "Common.h"
#include <functional>
#include <glm/glm.hpp>
struct WindowDescriptor
{
    U32 width = 1024;
    U32 height = 768;
    const char *title = "Glow";
    U8 samples = 4;
};

class Window
{
public:
    Window(const WindowDescriptor &descriptor);

    void pollEvent();

    bool shouldClose();

    void swapBuffers();

    void cleanup();

    void getSize(int *width, int *height);

    GLFWwindow *getWindow()
    {
        return mWindow;
    }

    void setSizeCallback(std::function<void(U32, U32)> callback)
    {
        mSizeCallback = callback;
    }

    glm::vec2 getMousePos()
    {
        double x, y;
        glfwGetCursorPos(mWindow, &x, &y);
        return {x, y};
    }

    int getMouseButton(int button)
    {
        return glfwGetMouseButton(mWindow, button);
    }

    int getKey(int key) {
        return glfwGetKey(mWindow, key);
    }

private:
    GLFWwindow *mWindow;

    std::function<void(U32, U32)> mSizeCallback = [](U32, U32) {};
};