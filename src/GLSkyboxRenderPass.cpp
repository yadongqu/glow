#include "GLSkyboxRenderPass.h"
#include "Geometry.h"
#include "Renderer.h"

const char *skyboxVS = R"(
#version 460 core
layout (location = 0) in vec3 aPos;

uniform mat4 proj;
uniform mat4 view;

out vec3 WorldPos;

void main()
{
    WorldPos = aPos;

	mat4 rotView = mat4(mat3(view));
	vec4 clipPos = proj * rotView * vec4(WorldPos, 1.0);

	gl_Position = clipPos.xyww;
}
)";

const char *skyboxFS = R"(
#version 460 core
out vec4 FragColor;
in vec3 WorldPos;

layout (binding = 0) uniform samplerCube environmentMap;

void main()
{		
    vec3 envColor = textureLod(environmentMap, WorldPos, 0.0).rgb;
    
    // HDR tonemap and gamma correct
    envColor = envColor / (envColor + vec3(1.0));
    envColor = pow(envColor, vec3(1.0/2.2)); 
    
    FragColor = vec4(envColor, 1.0);
}
)";

void GLSkyboxRenderPass::init(SceneGraph &scene)
{
    mProgram = GLDevice::createProgram(skyboxVS, skyboxFS);
    GLDevice::useProgram(mProgram);
    GLDevice::setUniform(mProgram, "proj", scene.camera.proj);
    GLDevice::setUniform(mProgram, "view", scene.camera.view);
}

void GLSkyboxRenderPass::render(SceneGraph &scene)
{

    GLDevice::useProgram(mProgram);
    GLDevice::bindTexture(mCubeMap, 0);
    GLDevice::drawVAO(Renderer::CUBE_HANDLE);
}
