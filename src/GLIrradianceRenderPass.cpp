#include "GLIrradianceRenderPass.h"
#include "Geometry.h"
#include "Renderer.h"
const char *irradianceVS = R"(
#version 460 core
layout (location = 0) in vec3 aPos;

out vec3 WorldPos;

uniform mat4 proj;
uniform mat4 view;

void main()
{
    WorldPos = aPos;  
    gl_Position =  proj * view * vec4(WorldPos, 1.0);
}
)";

const char *irradianceFS = R"(

#version 460 core
out vec4 FragColor;
in vec3 WorldPos;

layout (binding = 0) uniform samplerCube environmentMap;

const float PI = 3.14159265359;

void main()
{		
    vec3 N = normalize(WorldPos);

    vec3 irradiance = vec3(0.0);   
    
    // tangent space calculation from origin point
    vec3 up    = vec3(0.0, 1.0, 0.0);
    vec3 right = normalize(cross(up, N));
    up         = normalize(cross(N, right));
       
    float sampleDelta = 0.025;
    float nrSamples = 0.0f;
    for(float phi = 0.0; phi < 2.0 * PI; phi += sampleDelta)
    {
        for(float theta = 0.0; theta < 0.5 * PI; theta += sampleDelta)
        {
            // spherical to cartesian (in tangent space)
            vec3 tangentSample = vec3(sin(theta) * cos(phi),  sin(theta) * sin(phi), cos(theta));
            // tangent space to world
            vec3 sampleVec = tangentSample.x * right + tangentSample.y * up + tangentSample.z * N; 

            irradiance += texture(environmentMap, sampleVec).rgb * cos(theta) * sin(theta);
            nrSamples++;
        }
    }
    irradiance = PI * irradiance * (1.0 / float(nrSamples));
    
    FragColor = vec4(irradiance, 1.0);
}
)";

void GLIrradianceRenderPass::init(SceneGraph &scene)
{
    mProgram = GLDevice::createProgram(irradianceVS, irradianceFS);

    FramebufferDescriptor descriptor{
        .width = 32,
        .height = 32,
        .depthFormat = GL_DEPTH_COMPONENT24};
    captureFBO = GLDevice::createFramebuffer(descriptor);

    irradianceMap = GLDevice::createTexture(
        TextureDescriptor{
            .width = 32,
            .height = 32,
            .target = GL_TEXTURE_CUBE_MAP,
            .internalFormat = GL_RGB16F,
            .minFilterMode = GL_LINEAR,
            .magFilterMode = GL_LINEAR,
            .wrapSMode = GL_CLAMP_TO_EDGE,
            .wrapTMode = GL_CLAMP_TO_EDGE,
            .wrapRMode = GL_CLAMP_TO_EDGE});
}

void GLIrradianceRenderPass::render(SceneGraph &scene)
{
    GLDevice::useProgram(mProgram);
    GLDevice::setUniform(mProgram, "proj", Renderer::CAPTURE_PROJECTION);
    GLDevice::bindTexture(mCubeMap, 0);
    glViewport(0, 0, 32, 32);
    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO.handle);
    for (int i = 0; i < 6; i++)
    {
        GLDevice::setUniform(mProgram, "view", Renderer::CAPTURE_VIEW[i]);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, irradianceMap.handle, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        GLDevice::drawVAO(Renderer::CUBE_HANDLE);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}