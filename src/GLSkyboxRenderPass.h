#pragma once
#include "GLRenderPass.h"
class GLSkyboxRenderPass : public GLRenderPass
{
public:
    virtual void init(SceneGraph &scene) override;
    virtual void render(SceneGraph &scene) override;

    void setEnvCubeMap(TextureHandle &textureHandle)
    {
        mCubeMap = textureHandle;
    }

private:
    ProgramHandle mProgram;
    TextureHandle mCubeMap;
};