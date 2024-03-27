#include "GLDevice.h"
#include <spdlog/spdlog.h>

namespace GLDevice
{
    BufferHandle createBuffer(const BufferDescriptor &descriptor)
    {
        U32 handle = 0;
        glCreateBuffers(1, &handle);
        // TODO: currently we only create immutable buffers
        glNamedBufferStorage(handle, descriptor.size, descriptor.data, descriptor.flags);
        return {.handle = handle, .count = descriptor.count};
    }

    TextureHandle createTexture(const TextureDescriptor &descriptor)
    {
        U32 handle = 0;
        glCreateTextures(descriptor.target, 1, &handle);
        glTextureStorage2D(handle, descriptor.mipmaps, descriptor.internalFormat, descriptor.width, descriptor.height);

        // TODO: currently only support a few texture format we will actually be using.

        auto dataFormat = GL_RGBA;
        auto dataType = GL_UNSIGNED_BYTE;
        switch (descriptor.internalFormat)
        {
        case GL_RGBA8:
            dataFormat = GL_RGBA;
            break;
        case GL_RGB8:
            dataFormat = GL_RGB;
            break;
        case GL_R8:
            dataFormat = GL_RED;
        case GL_RGB16F:
            dataFormat = GL_RGB;
            dataType = GL_HALF_FLOAT;
            break;
        case GL_R16F:
            dataFormat = GL_RED;
            dataType = GL_HALF_FLOAT;
            break;
        case GL_RG16F:
            dataFormat = GL_RG;
            dataType = GL_HALF_FLOAT;
            break;
        case GL_RGB32F:
            dataFormat = GL_RGB;
            dataType = GL_FLOAT;
            break;
        case GL_R32F:
            dataFormat = GL_RED;
            dataType = GL_FLOAT;
            break;
        case GL_RG32F:
            dataFormat = GL_RG;
            dataType = GL_FLOAT;
        default:
            spdlog::error("UnSupported internal format: {}", descriptor.internalFormat);
            break;
        }

        if (descriptor.data)
        {
            glTextureSubImage2D(handle, 0, 0, 0, descriptor.width, descriptor.height, dataFormat, dataType, descriptor.data);
        }

        glTextureParameteri(handle, GL_TEXTURE_MIN_FILTER, descriptor.minFilterMode);
        glTextureParameteri(handle, GL_TEXTURE_MAG_FILTER, descriptor.magFilterMode);
        glTextureParameteri(handle, GL_TEXTURE_WRAP_S, descriptor.wrapSMode);
        glTextureParameteri(handle, GL_TEXTURE_WRAP_T, descriptor.wrapTMode);
        if (descriptor.wrapRMode != GL_INVALID_ENUM)
        {
            glTextureParameteri(handle, GL_TEXTURE_WRAP_R, descriptor.wrapRMode);
        }

        return {.handle = handle};
    }

    void bindTexture(TextureHandle &textureHandle, U32 slot)
    {
        glBindTextureUnit(slot, textureHandle.handle);
    }

    void generateMipmap(TextureHandle &textureHandle)
    {
        glGenerateTextureMipmap(textureHandle.handle);
    }

    ShaderHandle createShader(const ShaderDescriptor &descriptor)
    {
        U32 handle = glCreateShader(descriptor.type);
        glShaderSource(handle, 1, &descriptor.source, nullptr);
        glCompileShader(handle);

        GLint isCompiled = 0;
        glGetShaderiv(handle, GL_COMPILE_STATUS, &isCompiled);
        if (isCompiled == GL_FALSE)
        {
            GLint maxLength = 0;
            glGetShaderiv(handle, GL_INFO_LOG_LENGTH, &maxLength);
            std::vector<GLchar> infoLog(maxLength);
            glGetShaderInfoLog(handle, maxLength, &maxLength, &infoLog[0]);
            spdlog::error("Shader compile error: {}", infoLog.data());
            handle = INVALID_RESOURCE_HANDLE;
        }

        return {.handle = handle};
    }

    ProgramHandle createProgram(const ProgramDescriptor &descriptor)
    {
        U32 handle = glCreateProgram();
        for (auto &&shader : descriptor.shaders)
        {
            glAttachShader(handle, shader.handle);
        }
        glLinkProgram(handle);

        GLint isLinked = 0;
        glGetProgramiv(handle, GL_LINK_STATUS, &isLinked);
        if (isLinked == GL_FALSE)
        {
            GLint maxLength = 0;
            glGetProgramiv(handle, GL_INFO_LOG_LENGTH, &maxLength);
            std::vector<GLchar> infoLog(maxLength);
            glGetProgramInfoLog(handle, maxLength, &maxLength, &infoLog[0]);
            spdlog::error("Program link error: {}", infoLog.data());
            handle = INVALID_RESOURCE_HANDLE;
        }
        return {.handle = handle};
    }

    ProgramHandle createProgram(const char *vsSource, const char *fsSource)
    {
        ShaderDescriptor vertexShader{
            .name = "default vertex shader",
            .source = vsSource,
            .type = GL_VERTEX_SHADER,
        };
        ShaderHandle vertexShaderHandle = createShader(vertexShader);

        ShaderDescriptor fragmentShader{
            .name = "default fragment shader",
            .source = fsSource,
            .type = GL_FRAGMENT_SHADER,
        };

        ShaderHandle fragmentShaderHandle = createShader(fragmentShader);

        ProgramDescriptor programDescriptor{
            .name = "default program",
            .shaders = {vertexShaderHandle, fragmentShaderHandle},
        };

        return createProgram(programDescriptor);
    }

    VAOHandle createVAO(const VAODescriptor &descriptor)
    {
        U32 handle = 0;
        glCreateVertexArrays(1, &handle);

        glVertexArrayVertexBuffer(handle, 0, descriptor.vertexBuffer.handle, 0, descriptor.stride);
        U32 count = descriptor.vertexBuffer.count;
        bool hasIndices = descriptor.indexBuffer.handle != INVALID_RESOURCE_HANDLE;
        if (hasIndices)
        {
            glVertexArrayElementBuffer(handle, descriptor.indexBuffer.handle);
            count = descriptor.indexBuffer.count;
        }

        for (auto &&attribute : descriptor.attributes)
        {
            glEnableVertexArrayAttrib(handle, attribute.location);
            // TODO: currently only support single buffer binding, so we hard code to 0
            glVertexArrayAttribBinding(handle, attribute.location, 0);
            glVertexArrayAttribFormat(handle, attribute.location, attribute.componentPerElement, attribute.type, attribute.normalized, attribute.offset);
        }

        return {.handle = handle, .count = count, .hasIndices = hasIndices};
    }

    VAOHandle createVAO(std::vector<Vertex> &vertices, std::vector<U32> &indices)
    {
        BufferDescriptor vertexBufferDescriptor{
            .size = static_cast<U32>(vertices.size() * sizeof(Vertex)),
            .data = vertices.data(),
            .count = static_cast<U32>(vertices.size()),
            .flags = GL_DYNAMIC_STORAGE_BIT,
            .name = "vertex",
        };
        BufferHandle vertexHandle = createBuffer(vertexBufferDescriptor);
        BufferDescriptor indicesBufferDescriptor{
            .size = static_cast<U32>(indices.size() * sizeof(uint32_t)),
            .data = indices.data(),
            .count = static_cast<U32>(indices.size()),
            .flags = GL_DYNAMIC_STORAGE_BIT,
            .name = "index",
        };
        BufferHandle indexHandle = createBuffer(indicesBufferDescriptor);
        VAODescriptor vaoDescriptor{
            .attributes = Vertex::getAttributes(),
            .vertexBuffer = vertexHandle,
            .indexBuffer = indexHandle,
            .stride = sizeof(Vertex),
        };
        return createVAO(vaoDescriptor);
    }

    void drawVAO(VAOHandle vao)
    {
        glBindVertexArray(vao.handle);

        if (vao.hasIndices)
        {
            glDrawElements(GL_TRIANGLES, vao.count, GL_UNSIGNED_INT, 0);
        }
        else
        {
            glDrawArrays(GL_TRIANGLES, 0, vao.count);
        }
    }

    FramebufferHandle createFramebuffer(const FramebufferDescriptor &framebufferDescriptor)
    {
        U32 framebufferHandle, renderbufferHandle;
        glCreateFramebuffers(1, &framebufferHandle);
        glCreateRenderbuffers(1, &renderbufferHandle);
        glNamedRenderbufferStorage(renderbufferHandle, framebufferDescriptor.depthFormat, framebufferDescriptor.width, framebufferDescriptor.height);
        glNamedFramebufferRenderbuffer(framebufferHandle, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, renderbufferHandle);

        return {.handle = framebufferHandle,
                .depthbuffer =
                    RenderbufferHandle{
                        .width = framebufferDescriptor.width,
                        .height = framebufferDescriptor.height,
                        .format = framebufferDescriptor.depthFormat,
                        .handle = renderbufferHandle}};
    }

    void resizeFramebuffer(FramebufferHandle &framebuffer, U32 newWidth, U32 newHeight)
    {
        // TODO: not sure if this is correct, need to double check.
        glNamedRenderbufferStorage(framebuffer.depthbuffer.handle, framebuffer.depthbuffer.format, newWidth, newHeight);
        glNamedFramebufferRenderbuffer(framebuffer.handle, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, framebuffer.depthbuffer.handle);
    }

    void useProgram(ProgramHandle handle)
    {
        glUseProgram(handle.handle);
    }

    void setUniform(ProgramHandle program, const char *name, const GLUniformData &data)
    {
        auto location = glGetUniformLocation(program.handle, name);
        if (location == -1)
        {
            printf("Unknown uniform: %s\n", name);
            return;
        }

        switch (data.index())
        {
        case 0:
            glUniform1i(location, std::get<int>(data));
            break;
        case 1:
            glUniform1f(location, std::get<float>(data));
            break;
        case 2:
            glUniform2fv(location, 1, glm::value_ptr(std::get<glm::vec2>(data)));
            break;
        case 3:
            glUniform3fv(location, 1, glm::value_ptr(std::get<glm::vec3>(data)));
            break;
        case 4:
            glUniform4fv(location, 1, glm::value_ptr(std::get<glm::vec4>(data)));
            break;
        case 5:
            glUniformMatrix3fv(location, 1, GL_FALSE, glm::value_ptr(std::get<glm::mat3>(data)));
            break;
        case 6:
            glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(std::get<glm::mat4>(data)));
            break;
        default:
            printf("Unknown uniform type: %d\n", data.index());
            break;
        }
    }

    void deleteVAO(VAOHandle vao)
    {
        glDeleteVertexArrays(1, &vao.handle);
    }

    void deleteBuffer(BufferHandle buffer)
    {
        glDeleteBuffers(1, &buffer.handle);
    }

    void deleteTexture(TextureHandle texture)
    {
        glDeleteTextures(1, &texture.handle);
    }

    void deleteShader(ShaderHandle shader)
    {
        glDeleteProgram(shader.handle);
    }

    void deleteProgram(ProgramHandle program)
    {
        glDeleteProgram(program.handle);
    }
}
