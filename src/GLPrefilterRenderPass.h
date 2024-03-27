#pragma once

#include "GLRenderPass.h"

class GLPrefilterRenderPass : public GLRenderPass
{
public:
    virtual void init(SceneGraph &scene) override;
    virtual void render(SceneGraph &scene) override;

    void setEnvCubemap(TextureHandle cubemap)
    {
        mCubeMap = cubemap;
    }

    TextureHandle getPrefilterMap()
    {
        return prefilterMap;
    }

private:
    TextureHandle mCubeMap;
    TextureHandle prefilterMap;
    ProgramHandle mProgram;
    FramebufferHandle captureFBO;
    U8 maxMipLevels = 5;
};