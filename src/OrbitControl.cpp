#include "OrbitControl.h"
#include "SceneGraph.h"
#include "Window.h"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/common.hpp>

OrbitControl::OrbitControl(CameraData &camera, Window &window, F32 radius)
    : mCamera(camera), mWindow(window), mRadius(radius)
{
    mTheta = glm::atan(mCamera.eye.z, mCamera.eye.x);
    mPhi = glm::asin(mCamera.eye.y / mRadius);
}

void OrbitControl::rotate(F32 dx, F32 dy)
{
    mTheta += dx * 0.01f;
    mPhi += dy * 0.01f;
    mPhi = glm::clamp(mPhi, -glm::half_pi<float>() + 0.02f,
                      glm::half_pi<float>() - 0.02f);

    float x = mRadius * glm::cos(mPhi) * glm::cos(mTheta);
    float y = mRadius * glm::sin(mPhi);
    float z = mRadius * glm::cos(mPhi) * glm::sin(mTheta);

    mCamera.eye = glm::vec3(x, y, z);
    mCamera.view = glm::lookAt(mCamera.eye, mCamera.target, glm::vec3(0.0f, 1.0f, 0.0f));
}

void OrbitControl::update(F32 dt)
{
    if (mWindow.getMouseButton(GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
    {
        if (!enabled)
        {
            enabled = true;
            glm::vec2 newCurPos = mWindow.getMousePos();
            cursorPos = newCurPos;
            return;
        }
    }
    else
    {
        if (enabled)
        {
            enabled = false;
            return;
        }
    }

    if (enabled)
    {
        glm::vec2 newCurPos = mWindow.getMousePos();
        auto delta = newCurPos - cursorPos;
        cursorPos = newCurPos;
        rotate(delta.x, delta.y);
    }
}