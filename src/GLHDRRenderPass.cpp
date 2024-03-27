#include "GLHDRRenderPass.h"
#include "Geometry.h"
#include "Renderer.h"

const char *cubeMapVS = R"(
#version 460 core
layout (location = 0) in vec3 position;

out vec3 WorldPos;

uniform mat4 view;
uniform mat4 proj;

void main() {
    WorldPos = position;
    gl_Position = proj * view * vec4(position, 1.0);
}
)";

const char *hdrToCubeFS = R"(
#version 460 core
out vec4 FragColor;
in vec3 WorldPos;
layout (binding = 0) uniform sampler2D equirectangularMap;

const vec2 invAtan = vec2(0.1591, 0.3183);

vec2 SampleSphericalMap(vec3 v)
{
    vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
    uv *= invAtan;
    uv += 0.5;
    return uv;
}

void main()
{		
    vec2 uv = SampleSphericalMap(normalize(WorldPos));
    vec3 color = texture(equirectangularMap, uv).rgb;
    FragColor = vec4(color, 1.0);
}
)";

void GLHDRRenderPass::init(SceneGraph &scene)
{
    mProgram = GLDevice::createProgram(cubeMapVS, hdrToCubeFS);

    FramebufferDescriptor descriptor{
        .width = 512,
        .height = 512,
        .depthFormat = GL_DEPTH_COMPONENT24};
    captureFBO = GLDevice::createFramebuffer(descriptor);

    //    glGenFramebuffers(1, &captureFBO);
    //    glGenRenderbuffers(1, &captureRBO);
    //
    //    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
    //    glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
    //    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512);
    //    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, captureRBO);

    // loadHDRTexture("resources/hdr/immenstadter_horn_2k.hdr", hdrTexture);
    // loadHDRTexture("resources/hdr/newport_loft.hdr", hdrTexture);
    mHDRTexture = GLDevice::createTexture(TextureDescriptor{.width = scene.environment.width,
                                                            .height = scene.environment.height,
                                                            .data = scene.environment.data.data(),
                                                            .target = GL_TEXTURE_2D,
                                                            .internalFormat = GL_RGB32F,
                                                            .minFilterMode = GL_LINEAR,
                                                            .magFilterMode = GL_LINEAR,
                                                            .wrapSMode = GL_CLAMP_TO_EDGE,
                                                            .wrapTMode = GL_CLAMP_TO_EDGE});

    envCubeMap = GLDevice::createTexture(TextureDescriptor{
        .width = 512,
        .height = 512,
        .mipmaps = 1,
        .target = GL_TEXTURE_CUBE_MAP,
        .internalFormat = GL_RGB16F,
        .minFilterMode = GL_LINEAR_MIPMAP_LINEAR,
        .magFilterMode = GL_LINEAR,
        .wrapSMode = GL_CLAMP_TO_EDGE,
        .wrapTMode = GL_CLAMP_TO_EDGE,
        .wrapRMode = GL_CLAMP_TO_EDGE,
    });
}

void GLHDRRenderPass::render(SceneGraph &scene)
{

    GLDevice::useProgram(mProgram);
    GLDevice::setUniform(mProgram, "proj", Renderer::CAPTURE_PROJECTION);
    GLDevice::bindTexture(mHDRTexture, 0);
    glViewport(0, 0, captureFBO.depthbuffer.width, captureFBO.depthbuffer.height);
    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO.handle);
    glBindRenderbuffer(GL_RENDERBUFFER, captureFBO.depthbuffer.handle);
    for (int i = 0; i < 6; i++)
    {
        GLDevice::setUniform(mProgram, "view", Renderer::CAPTURE_VIEW[i]);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, envCubeMap.handle, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        GLDevice::drawVAO(Renderer::CUBE_HANDLE);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    GLDevice::generateMipmap(envCubeMap);
}