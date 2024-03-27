#pragma once

#include "GLRenderPass.h"

class GLBoundingBoxPass : public GLRenderPass
{
public:
    virtual void init(SceneGraph &scene) override;
    virtual void render(SceneGraph &scene) override;

    void renderNode(SceneGraph &scene, int32_t index);

private:
    ProgramHandle mProgram;
    GLuint mVAO;
    GLuint mVBO;
    GLuint mEBO;
};