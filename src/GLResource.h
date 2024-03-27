#pragma once
#include "Common.h"
#include <optional>
#include <vector>
#include <variant>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
static const U32 INVALID_RESOURCE_HANDLE = U32_MAX;

typedef U32 ResourceHandle;

struct BufferHandle
{
    ResourceHandle handle = INVALID_RESOURCE_HANDLE;
    U32 count = 0;
};

struct TextureHandle
{
    ResourceHandle handle = INVALID_RESOURCE_HANDLE;
};

struct ShaderHandle
{
    ResourceHandle handle = INVALID_RESOURCE_HANDLE;
};

struct ProgramHandle
{
    ResourceHandle handle = INVALID_RESOURCE_HANDLE;
};

struct SamplerHandle
{
    ResourceHandle handle = INVALID_RESOURCE_HANDLE;
};

struct VAOHandle
{
    // TODO: currently only support triangle drawing.
    ResourceHandle handle = INVALID_RESOURCE_HANDLE;
    U32 count;
    bool hasIndices;
};

struct BufferDescriptor
{
    U32 size = 0;
    void *data = nullptr;
    U32 count = 0;
    GLbitfield flags = GL_DYNAMIC_STORAGE_BIT;
    const char *name = nullptr;
};

struct TextureDescriptor
{
    U32 width = {512};
    U32 height = {512};
    void* data = nullptr;
    U8 mipmaps = 1;
    GLenum target = {GL_TEXTURE_2D};
    GLenum internalFormat = {GL_RGBA8};
    GLenum minFilterMode = GL_LINEAR;
    GLenum magFilterMode = GL_LINEAR;
    GLenum wrapSMode = GL_REPEAT;
    GLenum wrapTMode = GL_REPEAT;
    GLenum wrapRMode = GL_INVALID_ENUM;
};



struct ShaderDescriptor
{
    const char *name = nullptr;
    const char *source = nullptr;
    GLenum type = GL_NONE;
};

struct ProgramDescriptor
{
    const char *name = nullptr;
    std::vector<ShaderHandle> shaders;
};

struct VertexAttribute
{
    U16 location;
    U16 offset;
    GLenum type;
    U8 componentPerElement;
    bool normalized = false;
    const char *name = nullptr;
};

struct VAODescriptor
{
    std::vector<VertexAttribute> attributes;
    BufferHandle vertexBuffer;
    BufferHandle indexBuffer;
    U32 stride = 0;
};

using GLUniformData = std::variant<int, float, glm::vec2, glm::vec3, glm::vec4, glm::mat3, glm::mat4>;




struct RenderbufferHandle {
    U32 width;
    U32 height;
    GLenum format;
    ResourceHandle  handle;
};

struct FramebufferHandle {
    ResourceHandle  handle;
    RenderbufferHandle depthbuffer;
};

struct FramebufferDescriptor {
    U32 width = 1;
    U32 height = 1;
    GLenum depthFormat = GL_DEPTH_COMPONENT24;
};



