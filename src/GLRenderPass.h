#pragma once
#include "SceneGraph.h"
#include "GLDevice.h"
class GLRenderPass
{
public:
    virtual void init(SceneGraph &scene) = 0;
    virtual void render(SceneGraph &scene) = 0;

    virtual ~GLRenderPass() = default;
};