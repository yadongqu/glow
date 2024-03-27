#include "FreeControl.h"
#include "Window.h"
#include "SceneGraph.h"
FreeControl::FreeControl(CameraData& camera, Window& window)
    :mCamera(camera), mWindow(window)
{

}

void FreeControl::update(F32 dt) {
    if(mWindow.getKey(GLFW_KEY_W) == GLFW_PRESS) {
        auto forward = glm::normalize(mCamera.target - mCamera.eye);
        mCamera.eye += forward * dt * speed;
        mCamera.view = glm::lookAt(mCamera.eye, mCamera.target, glm::vec3(0.0f, 1.0f, 0.0f));
    }

    if (mWindow.getKey(GLFW_KEY_S) == GLFW_PRESS) {
        auto forward = glm::normalize(mCamera.target - mCamera.eye);
        mCamera.eye -= forward * dt * speed;
        mCamera.view = glm::lookAt(mCamera.eye, mCamera.target, glm::vec3(0.0f, 1.0f, 0.0f));
    }

    if (mWindow.getKey(GLFW_KEY_A) == GLFW_PRESS) {
        auto right = glm::normalize(glm::cross(mCamera.target - mCamera.eye, glm::vec3(0.0f, 1.0f, 0.0f)));
        mCamera.eye -= right * dt * speed;
        mCamera.target -= right * dt * speed;
        mCamera.view = glm::lookAt(mCamera.eye, mCamera.target, glm::vec3(0.0f, 1.0f, 0.0f));
    }

    if (mWindow.getKey(GLFW_KEY_D) == GLFW_PRESS) {
        auto right = glm::normalize(glm::cross(mCamera.target - mCamera.eye, glm::vec3(0.0f, 1.0f, 0.0f)));
        mCamera.eye += right * dt * speed;
        mCamera.target += right * dt * speed;
        mCamera.view = glm::lookAt(mCamera.eye, mCamera.target, glm::vec3(0.0f, 1.0f, 0.0f));
    }
}