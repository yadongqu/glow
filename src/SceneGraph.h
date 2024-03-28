#pragma once
#include <vector>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glad/glad.h>
#include "GLResource.h"

struct PointLight
{
    glm::vec3 position;
    glm::vec3 color;
    F32 intensity;
    F32 range;
};

struct CameraData
{
    glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 0.0f, 5.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 proj = glm::perspective(glm::radians(50.0f), 1.0f, 0.01f, 5000.0f);
    glm::vec3 eye = glm::vec3(0.0f, 0.0f, 5.0f);
    glm::vec3 target = glm::vec3(0.0f, 0.0f, 0.0f);
    F32 aspect = 1.0f;
    F32 fovY = 45.0f;
    F32 zNear = 0.1f;
    F32 zFar = 1000.0f;
};

struct Vertex
{
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 uv;
    // glm::vec3 tangent;
    // glm::vec3 bitangent;

    static std::vector<VertexAttribute> getAttributes()
    {
        auto position = VertexAttribute{
            .location = 0,
            .offset = offsetof(Vertex, position),
            .type = GL_FLOAT,
            .componentPerElement = 3,
        };
        auto normal = VertexAttribute{
            .location = 1,
            .offset = offsetof(Vertex, normal),
            .type = GL_FLOAT,
            .componentPerElement = 3,
        };
        auto uv = VertexAttribute{
            .location = 2,
            .offset = offsetof(Vertex, uv),
            .type = GL_FLOAT,
            .componentPerElement = 2,
        };
        return {position, normal, uv};
    }
};

struct BBox
{
    glm::vec3 center = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 extent = glm::vec3(0.0f, 0.0f, 0.0f);

    BBox() = default;

    BBox(const glm::vec3 &min, const glm::vec3 &max)
        : center{(0.5f * (min + max))}, extent(0.5f * (max - min))
    {
    }

    BBox expand(const glm::vec3 &point) const
    {
        const glm::vec3 min = glm::min(center - extent, point);
        const glm::vec3 max = glm::max(center + extent, point);
        return BBox(min, max);
    }

    BBox expand(const BBox &bbox) const
    {
        const glm::vec3 min = glm::min(center - extent, bbox.center - bbox.extent);
        const glm::vec3 max = glm::max(center + extent, bbox.center + bbox.extent);
        return BBox(min, max);
    }

    glm::vec3 getSize() const
    {
        return 2.0f * extent;
    }

    BBox transform(const glm::mat4 &mat) const
    {
        auto newCenter = glm::vec3(mat * glm::vec4(center, 1.0f));
        BBox result;
        result.center = newCenter;
        result.extent = this->extent;
        return result;
    }
};

struct Texture
{
    U32 width;
    U32 height;
    std::vector<U8> data;
    U8 channels;

    GLenum minFilterMode = GL_LINEAR;
    GLenum magFilterMode = GL_LINEAR;
    GLenum wrapSMode = GL_REPEAT;
    GLenum wrapTMode = GL_REPEAT;
    TextureHandle handle;
};

struct HDRTexture
{
    uint32_t width;
    uint32_t height;
    std::vector<F32> data;

    GLenum minFilterMode = GL_LINEAR;
    GLenum magFilterMode = GL_LINEAR;
    GLenum wrapSMode = GL_CLAMP_TO_EDGE;
    GLenum wrapTMode = GL_CLAMP_TO_EDGE;
};

struct Mesh
{
    std::vector<Vertex> vertices;
    std::vector<U32> indices;
    U32 material{U32_MAX};
    BBox bbox;
    VAOHandle vao;
};

struct Node
{
    std::vector<U32> meshes;
    std::vector<U32> children;
    U32 parent{U32_MAX};
};

struct Material
{
    enum AlphaMode
    {
        OPAQUE,
        BLEND,
        MASK
    };

    U32 albedoMap{U32_MAX};
    U32 normalMap{U32_MAX};
    U32 metallicRoughnessMap{U32_MAX};
    U32 emissiveMap{U32_MAX};
    U32 aoMap{U32_MAX};

    bool hasAlbedoMap = false;
    bool hasNormalMap = false;
    bool hasMetallicRoughnessMap = false;
    bool hasEmissiveMap = false;
    bool hasAOMap = false;

    glm::vec3 albedo{1.0f, 1.0f, 1.0f};
    F32 metallic{1.0f};
    F32 roughness{1.0f};
    glm::vec3 emissive{0.0f, 0.0f, 0.0f};
    F32 ao{1.0f};

    AlphaMode alphaMode = OPAQUE;
    F32 alphaCutoff = 0.5f;
    bool doubleSided = false;
};

struct SceneGraph
{

    bool load(const char *path);

    std::vector<glm::mat4> localMatrices;
    std::vector<glm::mat4> worldMatrices;
    std::vector<Mesh> meshes;
    std::vector<Texture> textures;
    std::vector<Material> materials;
    std::vector<Node> nodes;
    std::vector<BBox> bboxes;
    std::vector<PointLight> pointLights;
    bool initialized = false;

    CameraData camera;

    HDRTexture environment;

    void releaseGPUResource();

    void clear()
    {
        releaseGPUResource();
        textures.clear();
        materials.clear();
        meshes.clear();
        nodes.clear();

        initialized = false;
    }
};