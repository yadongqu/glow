#pragma once

#include "GLRenderPass.h"

class GLIrradianceRenderPass : public GLRenderPass
{
public:
    virtual void init(SceneGraph &scene) override;
    virtual void render(SceneGraph &scene) override;

    void setEnvCubemap(TextureHandle &textureHandle)
    {
        mCubeMap = textureHandle;
    }

    TextureHandle getIrradianceMap()
    {
        return irradianceMap;
    }

private:
    TextureHandle mCubeMap;
    TextureHandle irradianceMap;
    FramebufferHandle captureFBO;
    ProgramHandle mProgram;
};