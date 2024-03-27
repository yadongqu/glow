#pragma once
#include "GLDevice.h"
#include <GLFW/glfw3.h>
#include "SceneGraph.h"
#include "GLHDRRenderPass.h"
#include "GLSkyboxRenderPass.h"
#include "GLIrradianceRenderPass.h"
#include "GLPrefilterRenderPass.h"
#include "GLBrdfRenderPass.h"
#include "GLPBRRenderPass.h"
#include "GLBoundingBoxPass.h"
#include <string>
#include "gui.h"
class Window;
class Renderer
{
public:
    Renderer(Window *window);

    void initScene(SceneGraph &sceneGraph);

    void render(SceneGraph &sceneGraph);

    void resize(U32 width, U32 height);

    U32 width()
    {
        return mWidth;
    }

    U32 height()
    {
        return mHeight;
    }

    static glm::mat4 CAPTURE_PROJECTION;
    static glm::mat4 CAPTURE_VIEW[6];
    static VAOHandle CUBE_HANDLE;

private:
    Window *mWindow;
    ProgramHandle mDefaultProgram;
    U32 mWidth, mHeight;

    GLHDRRenderPass mHDRRenderPass;
    GLSkyboxRenderPass mSkyboxRenderPass;
    GLIrradianceRenderPass mIrradianceRenderPass;
    GLPrefilterRenderPass mPrefilterRenderPass;
    GLBrdfRenderPass mBrdfRenderPass;
    GLPBRRenderPass mPBRRenderPass;
    GLBoundingBoxPass mBoundingboxPass;

    MenuState mMenuState;

    int mModelIndex = 0;
};
