#pragma once
#include "Common.h"
#include <glm/glm.hpp>
class Window;
class CameraData;
class OrbitControl
{
public:
    OrbitControl(CameraData &camera, Window &window, F32 radius = 5.0f);

    void rotate(F32 dx, F32 dy);

    void update(F32 dt);

private:
    CameraData &mCamera;
    Window &mWindow;
    F32 mRadius;
    F32 mTheta;
    F32 mPhi;
    bool enabled = false;
    glm::vec2 cursorPos;
};