#pragma once
#include "GLResource.h"
#include "SceneGraph.h"
namespace GLDevice
{

    BufferHandle createBuffer(const BufferDescriptor &descriptor);
    TextureHandle createTexture(const TextureDescriptor &descriptor);

    void bindTexture(TextureHandle &textureHandle, U32 slot = 0);
    void generateMipmap(TextureHandle &textureHandle);

    ShaderHandle createShader(const ShaderDescriptor &descriptor);
    ProgramHandle createProgram(const ProgramDescriptor &descriptor);
    ProgramHandle createProgram(const char *vsSource, const char *fsSource);
    VAOHandle createVAO(const VAODescriptor &descriptor);
    VAOHandle createVAO(std::vector<Vertex> &vertex, std::vector<U32> &indices);
    void drawVAO(VAOHandle vao);

    FramebufferHandle createFramebuffer(const FramebufferDescriptor &framebufferDescriptor);
    void resizeFramebuffer(FramebufferHandle &framebuffer, U32 newWidth, U32 newHeight);

    void useProgram(ProgramHandle handle);
    void setUniform(ProgramHandle program, const char *name, const GLUniformData &data);

    void deleteVAO(VAOHandle vao);
    void deleteBuffer(BufferHandle buffer);
    void deleteTexture(TextureHandle texture);
    void deleteShader(ShaderHandle shader);
    void deleteProgram(ProgramHandle program);
};