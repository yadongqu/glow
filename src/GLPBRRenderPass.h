#pragma once

#include "GLRenderPass.h"
#include "Frustum.h"
class GLPBRRenderPass : public GLRenderPass
{
public:
    virtual void init(SceneGraph &scene) override;

    virtual void render(SceneGraph &scene) override;

    void setIrradianceMap(TextureHandle irrdianceMap)
    {
        this->irrdianceMap = irrdianceMap;
    }

    void setPrefilterMap(TextureHandle prefilterMap)
    {
        this->prefilterMap = prefilterMap;
    }

    void setBrdfLut(TextureHandle brdfLut)
    {
        this->brdfLut = brdfLut;
    }

    void setRenderMode(int mode)
    {
        renderMode = mode;
    }

private:
    void renderNode(SceneGraph &scene, int32_t index);
    ProgramHandle mProgram;
    VAOHandle mVAO;
    TextureHandle irrdianceMap;
    TextureHandle prefilterMap;
    TextureHandle brdfLut;

    Frustum frustum;
    int renderMode = 0;
    float rotation = 0.0f;
    glm::mat4 model = glm::mat4(1.0f);
};
