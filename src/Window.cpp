#include "Window.h"
#include <spdlog/spdlog.h>
Window::Window(const WindowDescriptor &descriptor)
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // set sample
    glfwWindowHint(GLFW_SAMPLES, 4);

    auto &width = descriptor.width;
    auto &height = descriptor.height;
    auto &title = descriptor.title;

    mWindow = glfwCreateWindow(width, height, title, nullptr, nullptr);
    glfwMakeContextCurrent(mWindow);
    glfwSetWindowUserPointer(mWindow, this);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        spdlog::error("Failed to initialize GLAD");
        exit(-1);
    }

    glfwSetFramebufferSizeCallback(mWindow, [](GLFWwindow *window, int width, int height)
                                   { auto app = static_cast<Window *>(glfwGetWindowUserPointer(window));
                                     app->mSizeCallback(width, height); });
}
void Window::cleanup()
{
    glfwDestroyWindow(mWindow);
    glfwTerminate();
}

void Window::pollEvent()
{
    glfwPollEvents();
}

bool Window::shouldClose()
{
    return glfwWindowShouldClose(mWindow);
}

void Window::swapBuffers()
{
    glfwSwapBuffers(mWindow);
}

void Window::getSize(int *width, int *height)
{
    glfwGetWindowSize(mWindow, width, height);
}