#pragma once
#include "Common.h"
class Window;
class CameraData;
class FreeControl {
public:
    FreeControl(CameraData& camera, Window& window);
    void update(F32 dt);

private:
        Window &mWindow;
        CameraData &mCamera;
        F32 speed{100.0f};
};