#pragma once

#include "GLRenderPass.h"

class GLBrdfRenderPass : public GLRenderPass
{
public:
    virtual void init(SceneGraph &scene) override;
    virtual void render(SceneGraph &scene) override;

    TextureHandle getBrdfLut()
    {
        return mBrdfLut;
    }

private:
    ProgramHandle mProgram;
    TextureHandle mBrdfLut;
    VAOHandle mVAO;
    FramebufferHandle captureFBO;
};