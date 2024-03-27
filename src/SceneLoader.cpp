#include "SceneGraph.h"
#include "GLDevice.h"
#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/pbrmaterial.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <string>

void loadTexture(const char *path, Texture &texture)
{
    stbi_set_flip_vertically_on_load(true);
    int w, h, c;
    stbi_uc *data = stbi_load(path, &w, &h, &c, STBI_rgb_alpha);
    if (!data)
    {
        fprintf(stderr, "Failed to load texture: %s\n", path);
    }
    uint8_t *src = (uint8_t *)data;
    texture.width = w;
    texture.height = h;
    texture.channels = STBI_rgb_alpha;
    texture.data.resize(w * h * 4);
    // copy the data from the stbi image to the vector
    memcpy(texture.data.data(), src, w * h * 4);
    stbi_image_free(data);
}

void loadHDRTexture(const char *path, HDRTexture &hdrTexture)
{

    stbi_set_flip_vertically_on_load(true);
    int w, h, c;
    float *data = stbi_loadf(path, &w, &h, &c, STBI_rgb);
    if (!data)
    {
        fprintf(stderr, "Failed to load HDR texture: %s\n", path);
    }
    hdrTexture.width = w;
    hdrTexture.height = h;
    hdrTexture.data.resize(w * h * 3);
    // copy the data from the stbi image to the vector
    memcpy(hdrTexture.data.data(), data, w * h * 3 * sizeof(float));
    stbi_image_free(data);
}

glm::mat4 aiToGlm(const aiMatrix4x4 &from)
{
    glm::mat4 to;
    to[0][0] = (float)from.a1;
    to[0][1] = (float)from.b1;
    to[0][2] = (float)from.c1;
    to[0][3] = (float)from.d1;
    to[1][0] = (float)from.a2;
    to[1][1] = (float)from.b2;
    to[1][2] = (float)from.c2;
    to[1][3] = (float)from.d2;
    to[2][0] = (float)from.a3;
    to[2][1] = (float)from.b3;
    to[2][2] = (float)from.c3;
    to[2][3] = (float)from.d3;
    to[3][0] = (float)from.a4;
    to[3][1] = (float)from.b4;
    to[3][2] = (float)from.c4;
    to[3][3] = (float)from.d4;
    return to;
}

void loadNode(const aiScene *scene, aiNode *node, SceneGraph &sceneGraph, U32 parentIndex = U32_MAX) {
    int nodeIndex = sceneGraph.nodes.size();
    sceneGraph.nodes.push_back(Node());
    sceneGraph.localMatrices.push_back(aiToGlm(node->mTransformation));
    sceneGraph.worldMatrices.push_back(aiToGlm(node->mTransformation));
    sceneGraph.bboxes.push_back(BBox());
    auto &newNode = sceneGraph.nodes[nodeIndex];
    newNode.parent = parentIndex;
    if (parentIndex != U32_MAX) {
        sceneGraph.nodes[parentIndex].children.push_back(nodeIndex);
        // get parent world matrix
        auto parentWorldMatrix = sceneGraph.worldMatrices[parentIndex];
        // set this node's world matrix
        sceneGraph.worldMatrices[nodeIndex] = parentWorldMatrix * sceneGraph.localMatrices[nodeIndex];
    } else {
        // this is the root node, its global matrix is the same as its local matrix
        sceneGraph.worldMatrices[nodeIndex] = sceneGraph.localMatrices[nodeIndex];
    }
    if (node->mNumMeshes > 0) {
        newNode.meshes.resize(node->mNumMeshes);
        for (int i = 0; i < node->mNumMeshes; i++) {
            newNode.meshes[i] = node->mMeshes[i];
        }
    }
    for(U32 i = 0; i < node->mNumChildren; i++){
        loadNode(scene, node->mChildren[i], sceneGraph, nodeIndex);
    }
}

void updateNodeBBox(SceneGraph &sceneGraph, U32 nodeIndex) {
    auto &node = sceneGraph.nodes[nodeIndex];
    auto &bbox = sceneGraph.bboxes[nodeIndex];
    if (node.meshes.size() > 0) {
        for (auto meshIndex : node.meshes) {
            auto &mesh = sceneGraph.meshes[meshIndex];
            bbox = bbox.expand(mesh.bbox);
        }
        // transform to world coordinate
        bbox = bbox.transform(sceneGraph.worldMatrices[nodeIndex]);
        return;
    }
    for(U32 child : node.children){
        updateNodeBBox(sceneGraph, child);
        bbox = bbox.expand(sceneGraph.bboxes[child]);
    }
}

bool SceneGraph::load(const char *path){
    std::string dir = path;
    dir = dir.substr(0, dir.find_last_of("/"));
    const unsigned int flags = 0 |
                               aiProcess_JoinIdenticalVertices |
                               aiProcess_Triangulate |
                               aiProcess_GenSmoothNormals |
                               aiProcess_LimitBoneWeights |
                               aiProcess_SplitLargeMeshes |
                               aiProcess_ImproveCacheLocality |
                               aiProcess_RemoveRedundantMaterials |
                               aiProcess_FindDegenerates |
                               aiProcess_FindInvalidData |
                               aiProcess_GenUVCoords;
    const aiScene *scene = aiImportFile(path, flags);

    if (!scene || !scene->HasMeshes()) {
        fprintf(stderr, "Failed to load scene\n");
        return false;
    }

    meshes.resize(scene->mNumMeshes);
    printf("Loadding %d meshes\n", scene->mNumMeshes);
    for(U32 i = 0; i < meshes.size(); i++){
        auto &mesh = scene->mMeshes[i];
        auto &primitive = meshes[i];
        primitive.vertices.resize(mesh->mNumVertices);
        for(U32 j = 0; j < mesh->mNumVertices; j++){
            auto &v = mesh->mVertices[j];
            auto &n = mesh->mNormals[j];

            primitive.vertices[j].position = {v.x, v.y, v.z};
            primitive.vertices[j].normal = {n.x, n.y, n.z};

            if (mesh->HasTextureCoords(0)) {
                auto &uv = mesh->mTextureCoords[0][j];
                primitive.vertices[j].uv = {uv.x, uv.y};
            }



            primitive.bbox = primitive.bbox.expand(primitive.vertices[j].position);
        }

        primitive.indices.resize(mesh->mNumFaces * 3);
        for(U32 j = 0; j < mesh->mNumFaces; j++){
            auto &f = mesh->mFaces[j];
            primitive.indices[j * 3 + 0] = f.mIndices[0];
            primitive.indices[j * 3 + 1] = f.mIndices[1];
            primitive.indices[j * 3 + 2] = f.mIndices[2];
        }
        primitive.material = mesh->mMaterialIndex;
    }

    printf("Loading %d nodes\n", scene->mRootNode->mNumChildren);
    loadNode(scene, scene->mRootNode, *this);

    materials.resize(scene->mNumMaterials);
    for(U32 i = 0; i < scene->mNumMaterials; i++){ 
        auto &M = scene->mMaterials[i];
        auto &material = materials[i];
        aiColor4D color;
        if (aiGetMaterialColor(M, AI_MATKEY_COLOR_AMBIENT, &color) == AI_SUCCESS)
        {
            material.emissive= glm::vec3(color.r, color.g, color.b);

        }

        if (aiGetMaterialColor(M, AI_MATKEY_COLOR_DIFFUSE, &color) == AI_SUCCESS)
        {
            material.albedo = glm::vec3(color.r, color.g, color.b);

        }

        if (aiGetMaterialColor(M, AI_MATKEY_COLOR_EMISSIVE, &color) == AI_SUCCESS)
        {
            material.emissive.x += color.r;
            material.emissive.y += color.g;
            material.emissive.z += color.b;
        }

        // float opacity = 1.0f;

        // if (aiGetMaterialFloat(M, AI_MATKEY_OPACITY, &opacity) == AI_SUCCESS) {
        //     material.opacityFactor = glm::clamp(opacity, 0.0f, 1.0f);
        // }

        // if (aiGetMaterialColor(M, AI_MATKEY_COLOR_TRANSPARENT, &color) == AI_SUCCESS) {
        //     const float opacity = std::max(std::max())
        // }

        float tmp = 1.0f;
        if (aiGetMaterialFloat(M, AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_METALLIC_FACTOR, &tmp) == AI_SUCCESS)
            material.metallic = tmp;

        if (aiGetMaterialFloat(M, AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_ROUGHNESS_FACTOR, &tmp) == AI_SUCCESS)
            material.roughness = tmp;

        aiString path;
        aiTextureMapping mapping;

        unsigned int UVIndex = 0;
        float blend = 1.0f;
        aiTextureOp textureOp = aiTextureOp_Add;
        aiTextureMapMode textureMapMode[2] = {aiTextureMapMode_Wrap, aiTextureMapMode_Wrap};

        unsigned int textureFlags = 0;

        if (aiGetMaterialTexture(M, aiTextureType_EMISSIVE, 0, &path, &mapping, &UVIndex, &blend, &textureOp, textureMapMode, &textureFlags) == AI_SUCCESS)
        {
            printf("Emissive texture: %s\n", path.C_Str());
            std::string pathString = path.C_Str();
            std::string fullPath = dir + "/" + pathString;
            Texture texture;
            loadTexture(fullPath.c_str(), texture);
            auto textureIndex = textures.size();
            textures.push_back(std::move(texture));
            material.emissiveMap = textureIndex;
            material.hasEmissiveMap = true;
        }

        if (aiGetMaterialTexture(M, aiTextureType_DIFFUSE, 0, &path, &mapping, &UVIndex, &blend, &textureOp, textureMapMode, &textureFlags) == AI_SUCCESS)
        {
            printf("Diffuse texture: %s\n", path.C_Str());
            std::string pathString = path.C_Str();
            std::string fullPath = dir + "/" + pathString;
            Texture texture;
            loadTexture(fullPath.c_str(), texture);
            auto textureIndex = textures.size();
            textures.push_back(std::move(texture));
            material.albedoMap = textureIndex;
            material.hasAlbedoMap = true;
        }

        if (aiGetMaterialTexture(M, aiTextureType_NORMALS, 0, &path, &mapping, &UVIndex, &blend, &textureOp, textureMapMode, &textureFlags) == AI_SUCCESS) {
            printf("Normal texture: %s\n", path.C_Str());
            std::string pathString = path.C_Str();
            std::string fullPath = dir + "/" + pathString;
            Texture texture;
            loadTexture(fullPath.c_str(), texture);
            auto textureIndex = textures.size();
            textures.push_back(std::move(texture));
            material.normalMap = textureIndex;
            material.hasNormalMap = true;
        }

        if (aiGetMaterialTexture(M, aiTextureType_METALNESS, 0, &path, &mapping, &UVIndex, &blend, &textureOp, textureMapMode, &textureFlags) == AI_SUCCESS) {
            printf("Metalness texture: %s\n", path.C_Str());
            std::string pathString = path.C_Str();
            std::string fullPath = dir + "/" + pathString;
            Texture texture;
            loadTexture(fullPath.c_str(), texture);
            auto textureIndex = textures.size();
            textures.push_back(std::move(texture));
            material.metallicRoughnessMap = textureIndex;
            material.hasMetallicRoughnessMap = true;
        }

        if (aiGetMaterialTexture(M, aiTextureType_LIGHTMAP, 0, &path, &mapping, &UVIndex, &blend, &textureOp, textureMapMode, &textureFlags) == AI_SUCCESS) {
            printf("Occlusion texture: %s\n", path.C_Str());
            std::string pathString = path.C_Str();
            std::string fullPath = dir + "/" + pathString;
            Texture texture;
            loadTexture(fullPath.c_str(), texture);
            auto textureIndex = textures.size();
            textures.push_back(std::move(texture));
            material.aoMap = textureIndex;
            material.hasAOMap = true;
        }

    }

    updateNodeBBox(*this, 0);

    const char *environmentPath = "resources/hdr/piazza_bologni_1k.hdr";
    loadHDRTexture(environmentPath, environment);

    return true;
}

void SceneGraph::releaseGPUResource()
{
    {
        for (auto &texture : textures)
        {
            if (texture.handle.handle != INVALID_RESOURCE_HANDLE)
            {
                GLDevice::deleteTexture(texture.handle);
            }
        }

        for (auto &mesh : meshes)
        {
            if (mesh.vao.handle != INVALID_RESOURCE_HANDLE)
            {
                GLDevice::deleteVAO(mesh.vao);
            }
        }
    }
}