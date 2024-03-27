// #include "SceneGraph.h"
// #define CGLTF_IMPLEMENTATION
// #include <cgltf.h>

// #define STB_IMAGE_IMPLEMENTATION
// #include <stb_image.h>

// #include <spdlog/spdlog.h>
// #include "GLDevice.h"
// void loadHDRTexture(const char *path, HDRTexture &hdrTexture)
// {

//     stbi_set_flip_vertically_on_load(true);
//     int w, h, c;
//     float *data = stbi_loadf(path, &w, &h, &c, STBI_rgb);
//     if (!data)
//     {
//         fprintf(stderr, "Failed to load HDR texture: %s\n", path);
//     }
//     hdrTexture.width = w;
//     hdrTexture.height = h;
//     hdrTexture.data.resize(w * h * 3);
//     // copy the data from the stbi image to the vector
//     memcpy(hdrTexture.data.data(), data, w * h * 3 * sizeof(float));
//     stbi_image_free(data);
// }

// void loadTexture(const char *path, Texture &texture)
// {
//     stbi_set_flip_vertically_on_load(true);
//     int w, h, c;
//     stbi_uc *data = stbi_load(path, &w, &h, &c, STBI_rgb_alpha);
//     if (!data)
//     {
//         fprintf(stderr, "Failed to load texture: %s\n", path);
//     }
//     uint8_t *src = (uint8_t *)data;
//     texture.width = w;
//     texture.height = h;
//     texture.channels = STBI_rgb_alpha;
//     texture.data.resize(w * h * 4);
//     // copy the data from the stbi image to the vector
//     memcpy(texture.data.data(), src, w * h * 4);
//     stbi_image_free(data);
// }

// cgltf_data *loadGLTFFile(const char *path)
// {
//     cgltf_options options;
//     memset(&options, 0, sizeof(cgltf_options));
//     cgltf_data *data = NULL;
//     cgltf_result result = cgltf_parse_file(&options, path, &data);
//     if (result != cgltf_result_success)
//     {
//         spdlog::error("Could not load gltf file: {}", path);
//         return 0;
//     }
//     result = cgltf_load_buffers(&options, data, path);
//     if (result != cgltf_result_success)
//     {
//         cgltf_free(data);
//         spdlog::error("Could not load gltf file: {}", path);
//         return 0;
//     }
//     result = cgltf_validate(data);
//     if (result != cgltf_result_success)
//     {
//         cgltf_free(data);
//         spdlog::error("invalid gltf file: {}", path);
//         return 0;
//     }
//     return data;
// }

// void GetScalarValues(std::vector<float> &outScalars, unsigned int inComponentCount, const cgltf_accessor &inAccessor)
// {
//     outScalars.resize(inAccessor.count * inComponentCount);
//     for (cgltf_size i = 0; i < inAccessor.count; ++i)
//     {
//         cgltf_accessor_read_float(&inAccessor, i, &outScalars[i * inComponentCount], inComponentCount);
//     }
// }

// bool loadMeshes(cgltf_data *data, SceneGraph &sceneGraph)
// {

//     // cgltf_node *nodes = data->nodes;
//     // U32 nodeCount = data->nodes_count;
//     cgltf_mesh *meshes = data->meshes;
//     U32 meshCount = data->meshes_count;
//     sceneGraph.meshes.resize(meshCount);
//     for (U32 i = 0; i < meshCount; i++)
//     {
//         cgltf_mesh *mesh = &meshes[i];
//         // if (strcmp(mesh->name, "prt_2_8_t_SOLIDS_1") == 0)
//         // {
//         //     printf("found prt_2_8_t_SOLIDS_1\n");
//         //     printf("mesh index: %d\n", std::distance(meshes, mesh));
//         //     printf("primitive count: %d\n", mesh->primitives_count);
//         // }
//         U32 numPrimitives = mesh->primitives_count;
//         sceneGraph.meshes[i].primitives.resize(numPrimitives);
//         for (U32 j = 0; j < numPrimitives; j++)
//         {
//             BBox bbox;
//             cgltf_primitive *primitive = &mesh->primitives[j];

//             U32 numAttributes = primitive->attributes_count;
//             std::vector<float> positions;
//             std::vector<float> normals;
//             std::vector<float> uvs;
//             for (U32 k = 0; k < numAttributes; ++k)
//             {
//                 cgltf_attribute *attribute = &primitive->attributes[k];

//                 cgltf_attribute_type type = attribute->type;
//                 cgltf_accessor &accessor = *attribute->data;

//                 U32 componentCount = 0;
//                 if (accessor.type == cgltf_type_vec2)
//                 {
//                     componentCount = 2;
//                 }
//                 else if (accessor.type == cgltf_type_vec3)
//                 {
//                     componentCount = 3;
//                 }
//                 else if (accessor.type == cgltf_type_vec4)
//                 {
//                     componentCount = 4;
//                 }

//                 if (strcmp(attribute->name, "POSITION") == 0)
//                 {
//                     assert(accessor.type == cgltf_type_vec3);
//                     GetScalarValues(positions, componentCount, accessor);
//                 }
//                 else if (strcmp(attribute->name, "NORMAL") == 0)
//                 {
//                     assert(accessor.type == cgltf_type_vec3);
//                     GetScalarValues(normals, componentCount, accessor);
//                 }
//                 else if (strcmp(attribute->name, "TEXCOORD_0") == 0)
//                 {
//                     assert(accessor.type == cgltf_type_vec2);
//                     GetScalarValues(uvs, componentCount, accessor);
//                 }
//                 else if (strcmp(attribute->name, "TANGENT") == 0)
//                 {
//                     // TODO: implement
//                 }
//             }

//             assert(positions.size() % 3 == 0);
//             assert(normals.size() % 3 == 0);
//             assert(uvs.size() % 2 == 0);
//             if (normals.size() > 0)
//             {
//                 assert(normals.size() == positions.size());
//             }
//             if (uvs.size() > 0)
//             {
//                 assert(uvs.size() / 2 == positions.size() / 3);
//             }
//             std::vector<Vertex> vertices(positions.size() / 3);
//             for (size_t l = 0; l < positions.size() / 3; l += 1)
//             {
//                 Vertex &vertex = vertices[l];
//                 vertex.position = glm::vec3(positions[l * 3], positions[l * 3 + 1], positions[l * 3 + 2]);
//                 bbox = bbox.expand(vertex.position);
//                 if (normals.size() > 0)
//                 {
//                     vertex.normal = glm::vec3(normals[l * 3], normals[l * 3 + 1], normals[l * 3 + 2]);
//                 }
//                 if (uvs.size() > 0)
//                 {
//                     vertex.uv = glm::vec2(uvs[l * 2], 1.0f - uvs[l * 2 + 1]);
//                 }
//             }

//             cgltf_accessor &accessor = *primitive->indices;
//             assert(accessor.count != 0);
//             std::vector<U32> indices(accessor.count);
//             for (U32 ind = 0; ind < accessor.count; ind++)
//             {
//                 indices[ind] = static_cast<U32>(cgltf_accessor_read_index(&accessor, ind));
//             }
//             U32 materialIndex = std::distance(data->materials, primitive->material);
//             Primitive resultPrimitive{.vertices = vertices, .indices = indices, .material = materialIndex, .bbox = bbox};
//             U32 primitiveIndex = sceneGraph.primitives.size();
//             sceneGraph.primitives.push_back(resultPrimitive);
//             sceneGraph.meshes[i].primitives[j] = primitiveIndex;
//         }
//     }

//     return true;
// }

// bool loadNodes(cgltf_data *data, SceneGraph &sceneGraph)
// {
//     cgltf_node *nodes = data->nodes;

//     U32 nodeCount = data->nodes_count;
//     sceneGraph.nodes.resize(nodeCount);
//     for (U32 i = 0; i < nodeCount; i++)
//     {
//         cgltf_node *node = &nodes[i];
//         // if (strcmp(node->name, "Camera") == 0)
//         // {
//         //     U32 cameraNodeIndex = std::distance(nodes, node);
//         //     sceneGraph.cameraNodes.push_back(cameraNodeIndex);
//         // }
//         if (node->mesh)
//         {
//             U32 meshIndex = std::distance(data->meshes, node->mesh);
//             sceneGraph.nodes[i].mesh = meshIndex;
//         }
//         for (U32 j = 0; j < node->children_count; j++)
//         {
//             U32 childIndex = std::distance(nodes, node->children[j]);
//             sceneGraph.nodes[i].children.push_back(childIndex);
//             sceneGraph.nodes[childIndex].parent = i;
//         }
//     }

//     // test
//     for (U32 i = 0; i < nodeCount; i++)
//     {
//         auto currentNode = sceneGraph.nodes[i];
//         for (U32 j = 0; j < currentNode.children.size(); j++)
//         {
//             auto childNode = sceneGraph.nodes[currentNode.children[j]];
//             assert(childNode.parent == i);
//         }
//     }

//     return true;
// }

// void loadLocalMatrices(cgltf_data *data, SceneGraph &sceneGraph)
// {
//     cgltf_node *nodes = data->nodes;
//     U32 nodeCount = data->nodes_count;
//     sceneGraph.localMatrices.resize(nodeCount);
//     for (U32 i = 0; i < nodeCount; i++)
//     {
//         cgltf_node *node = &nodes[i];
//         sceneGraph.localMatrices[i] = glm::mat4(1.0f);

//         // TODO: for some models, the matrix is not set correctly, need to investigate.
//         if (node->has_matrix)
//         {
//             sceneGraph.localMatrices[i] = glm::make_mat4(node->matrix);
//         }
//         else if (node->has_rotation || node->has_scale || node->has_translation)
//         {
//             glm::quat rotation{0.0f, 0.0f, 0.0f, 1.0f};
//             if (node->has_rotation)
//             {
//                 rotation = glm::make_quat(node->rotation);
//             }
//             glm::vec3 scale{1.0f, 1.0f, 1.0f};
//             if (node->has_scale)
//             {
//                 scale = glm::make_vec3(node->scale);
//             }
//             glm::vec3 translation{0.0f, 0.0f, 0.0f};
//             if (node->has_translation)
//             {
//                 translation = glm::make_vec3(node->translation);
//             }

//             auto T = glm::translate(glm::mat4(1.0f), translation);
//             auto R = glm::mat4_cast(rotation);
//             auto S = glm::scale(glm::mat4(1.0f), scale);
//             sceneGraph.localMatrices[i] = T * R * S;
//         }
//     }
// }

// void loadMaterials(cgltf_data *data, SceneGraph &sceneGraph)
// {
//     cgltf_material *materials = data->materials;
//     U32 materialCount = data->materials_count;
//     sceneGraph.materials.resize(materialCount);
//     for (U32 i = 0; i < materialCount; i++)
//     {
//         cgltf_material *material = &materials[i];
//         sceneGraph.materials[i].emissive = glm::vec3(material->emissive_factor[0], material->emissive_factor[1], material->emissive_factor[2]);
//         if (material->has_pbr_metallic_roughness)
//         {
//             cgltf_pbr_metallic_roughness *pbr = &material->pbr_metallic_roughness;

//             sceneGraph.materials[i].albedo = glm::vec3(pbr->base_color_factor[0], pbr->base_color_factor[1], pbr->base_color_factor[2]);
//             sceneGraph.materials[i].metallic = pbr->metallic_factor;
//             sceneGraph.materials[i].roughness = pbr->roughness_factor;
//             if (pbr->base_color_texture.texture)
//             {
//                 U32 textureIndex = std::distance(data->textures, pbr->base_color_texture.texture);
//                 sceneGraph.materials[i].albedoMap = textureIndex;
//                 sceneGraph.materials[i].hasAlbedoMap = true;
//             }
//             if (pbr->metallic_roughness_texture.texture)
//             {
//                 U32 textureIndex = std::distance(data->textures, pbr->metallic_roughness_texture.texture);
//                 sceneGraph.materials[i].metallicRoughnessMap = textureIndex;
//                 sceneGraph.materials[i].hasMetallicRoughnessMap = true;
//             }
//         }
//         if (material->normal_texture.texture)
//         {
//             U32 textureIndex = std::distance(data->textures, material->normal_texture.texture);
//             sceneGraph.materials[i].normalMap = textureIndex;
//             sceneGraph.materials[i].hasNormalMap = true;
//         }

//         if (material->emissive_texture.texture)
//         {
//             U32 textureIndex = std::distance(data->textures, material->emissive_texture.texture);
//             sceneGraph.materials[i].emissiveMap = textureIndex;
//             sceneGraph.materials[i].hasEmissiveMap = true;
//         }

//         if (material->occlusion_texture.texture)
//         {
//             U32 textureIndex = std::distance(data->textures, material->occlusion_texture.texture);
//             sceneGraph.materials[i].aoMap = textureIndex;
//             sceneGraph.materials[i].hasAOMap = true;
//         }
//     }
// }

// void loadTextures(cgltf_data *data, SceneGraph &sceneGraph, std::string &basePath)
// {
//     cgltf_texture *textures = data->textures;
//     U32 textureCount = data->textures_count;
//     sceneGraph.textures.resize(textureCount);
//     for (U32 i = 0; i < textureCount; i++)
//     {
//         cgltf_texture *texture = &textures[i];
//         if (texture->image)
//         {
//             cgltf_image *image = texture->image;
//             if (image->uri)
//             {
//                 std::string uri = basePath + image->uri;
//                 loadTexture(uri.c_str(), sceneGraph.textures[i]);
//             }
//             else
//             {
//                 spdlog::error("Currently only loading images from disk is supported");
//             }
//         }
//     }
// }

// glm::mat4 getNodeWorldMatrix(SceneGraph &sceneGraph, U32 nodeIndex)
// {
//     U32 parent = sceneGraph.nodes[nodeIndex].parent;
//     if (parent != U32_MAX)
//     {
//         return getNodeWorldMatrix(sceneGraph, parent) * sceneGraph.localMatrices[nodeIndex];
//     }
//     return sceneGraph.localMatrices[nodeIndex];
// }

// void updateChildrenWorldMatrices(SceneGraph &sceneGraph, U32 nodeIndex)
// {
//     for (U32 child : sceneGraph.nodes[nodeIndex].children)
//     {
//         sceneGraph.worldMatrices[child] = sceneGraph.worldMatrices[nodeIndex] * sceneGraph.localMatrices[child];
//         updateChildrenWorldMatrices(sceneGraph, child);
//     }
// }

// void updateWorldMatrices(SceneGraph &sceneGraph)
// {
//     sceneGraph.worldMatrices.resize(sceneGraph.localMatrices.size());
//     for (auto &root : sceneGraph.rootNodes)
//     {
//         sceneGraph.worldMatrices[root] = sceneGraph.localMatrices[root];
//         updateChildrenWorldMatrices(sceneGraph, root);
//     }
// }

// void updateRootNodes(SceneGraph &sceneGraph)
// {
//     sceneGraph.rootNodes.clear();
//     for (U32 i = 0; i < sceneGraph.nodes.size(); i++)
//     {
//         if (sceneGraph.nodes[i].parent == U32_MAX)
//         {
//             sceneGraph.rootNodes.push_back(i);
//         }
//     }
// }
// BBox computeBBox(SceneGraph &sceneGraph, U32 nodeIndex)
// {
//     BBox bbox;
//     auto &node = sceneGraph.nodes[nodeIndex];
//     auto matrix = sceneGraph.localMatrices[nodeIndex];
//     if (node.mesh != U32_MAX)
//     {
//         for (U32 primitive : sceneGraph.meshes[node.mesh].primitives)
//         {
//             BBox primitiveBbox = sceneGraph.primitives[primitive].bbox;

//             // glm::vec3 newCenter = matrix * glm::vec4(primitiveBbox.center, 1.0f);
//             // BBox newNodeBbox;
//             // newNodeBbox.center = newCenter;
//             // newNodeBbox.extent = primitiveBbox.extent;
//             bbox = bbox.expand(primitiveBbox);
//         }
//     }
//     for (U32 child : node.children)
//     {
//         auto childBbox = computeBBox(sceneGraph, child);
//         // glm::vec3 newCenter = matrix * glm::vec4(childBbox.center, 1.0f);
//         // BBox newNodeBbox;
//         // newNodeBbox.center = newCenter;
//         // newNodeBbox.extent = childBbox.extent;
//         bbox = bbox.expand(childBbox);
//     }

//     return bbox;
// }


// bool SceneGraph::load(const char *path)
// {
//     std::string dir = path;
//     dir = dir.substr(0, dir.find_last_of("/"));
//     std::string basePath = dir + "/";
//     cgltf_data *data = loadGLTFFile(path);
//     if (!data)
//     {
//         return false;
//     }
//     loadMeshes(data, *this);
//     loadNodes(data, *this);
//     loadMaterials(data, *this);
//     loadTextures(data, *this, basePath);
//     loadLocalMatrices(data, *this);
//     updateRootNodes(*this);
//     updateWorldMatrices(*this);

//     // auto cameraIndex = cameraNodes[0];
//     // auto localMatrix = localMatrices[cameraIndex];
//     // auto viewMatrix = glm::inverse(localMatrix);
//     // auto eye = glm::vec3(viewMatrix[3]);
//     // camera.eye = eye;
//     // camera.view = viewMatrix;

//     // auto center = rootBBox.center;
//     // auto extent = rootBBox.extent;
//     // printf("center: %f %f %f\n", center.x, center.y, center.z);
//     // printf("extent: %f %f %f\n", extent.x, extent.y, extent.z);
//     // camera.eye = center + glm::vec3(0, 0, extent.z);
//     // camera.target = center;
//     // camera.view = glm::lookAt(camera.eye, center, glm::vec3(0, 1, 0));

//     const char *environmentPath = "resources/hdr/piazza_bologni_1k.hdr";
//     loadHDRTexture(environmentPath, environment);

//     cgltf_free(data);
//     return true;
// }

// void SceneGraph::releaseGPUResource()
// {
//     {
//         for (auto &texture : textures)
//         {
//             if (texture.handle.handle != INVALID_RESOURCE_HANDLE)
//             {
//                 GLDevice::deleteTexture(texture.handle);
//             }
//         }

//         for (auto &primitive : primitives)
//         {
//             if (primitive.vao.handle != INVALID_RESOURCE_HANDLE)
//             {
//                 GLDevice::deleteVAO(primitive.vao);
//             }
//         }
//     }
// }