#pragma once
#include "GLRenderPass.h"
class Renderer;
class GLHDRRenderPass : public GLRenderPass
{
public:
    virtual void init(SceneGraph &scene) override;

    virtual void render(SceneGraph &scene) override;

    TextureHandle getEnvCubeMap()
    {
        return envCubeMap;
    }

private:
    ProgramHandle mProgram;
    TextureHandle envCubeMap;
    TextureHandle mHDRTexture;
    FramebufferHandle captureFBO;
};