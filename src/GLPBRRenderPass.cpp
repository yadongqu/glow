#include "GLPBRRenderPass.h"

const char *pbrVS = R"(
#version 460 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoord;
layout (location = 3) in vec3 tangent;
layout (location = 4) in vec3 bitangent;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;
uniform mat3 normalMat;



out vec3 WorldPos;
out vec3 Normal;
out vec2 TexCoord;

void main() {

    WorldPos = vec3(model * vec4(position, 1.0));
    Normal = normalize(normalMat * normal);
    TexCoord = texCoord;
    gl_Position = proj * view * model * vec4(position, 1.0);
}
)";

const char *pbrFS = R"(
#version 460 core

in vec3 WorldPos;
in vec3 Normal;
in vec2 TexCoord;

uniform bool hasAlbedoMap;
uniform bool hasNormalMap;
uniform bool hasMetallicRoughnessMap;
uniform bool hasAoMap;
uniform bool hasEmissiveMap;

uniform vec3 albedo_;
uniform float metallic_;
uniform float roughness_;
uniform float ao_;
uniform vec3 emissive_;


layout (binding = 0) uniform sampler2D albedoMap;
layout (binding = 1) uniform sampler2D normalMap;
layout (binding = 2) uniform sampler2D metallicRoughnessMap;
layout (binding = 3) uniform sampler2D aoMap;
layout (binding = 4) uniform sampler2D emissiveMap;

layout (binding = 5) uniform samplerCube irradianceMap;
layout (binding = 6) uniform samplerCube prefilterMap;
layout (binding = 7) uniform sampler2D brdfLUT;

uniform vec3 camPos;
uniform int renderMode;

const float PI = 3.14159265359;

vec3 getNormalFromMap()
{
    vec3 tangentNormal = texture(normalMap, TexCoord).xyz * 2.0 - 1.0;

    vec3 Q1  = dFdx(WorldPos);
    vec3 Q2  = dFdy(WorldPos);
    vec2 st1 = dFdx(TexCoord);
    vec2 st2 = dFdy(TexCoord);

    vec3 N   = normalize(Normal);
    vec3 T  = normalize(Q1*st2.t - Q2*st1.t);
    vec3 B  = -normalize(cross(N, T));
    mat3 TBN = mat3(T, B, N);

    return normalize(TBN * tangentNormal);
}

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness*roughness;
    float a2 = a*a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;

    float nom   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}
// ----------------------------------------------------------------------------
vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
} 


void main() {
    vec3 albedo = albedo_;
    if (hasAlbedoMap) {
        albedo = pow(texture(albedoMap, TexCoord).xyz, vec3(2.2));
    }
    float metallic = metallic_;
    float roughness = roughness_;
    if (hasMetallicRoughnessMap) {
        vec3 metallicRoughness = texture(metallicRoughnessMap, TexCoord).xyz;
        metallic = metallicRoughness.b;
        roughness = metallicRoughness.g;
    }
    float ao = ao_;
    if (hasAoMap) {
        ao = texture(aoMap, TexCoord).r;
    }

    vec3 emissive = emissive_;
    if (hasEmissiveMap) {
        emissive = texture(emissiveMap, TexCoord).rgb;

    }
    vec3 N = Normal;
    if (hasNormalMap) {
        N = getNormalFromMap();
    }
    vec3 V = normalize(camPos - WorldPos);
    vec3 R = reflect(-V, N);
    vec3 color = vec3(0.0);
    if (renderMode == 0) {
        vec3 F0 = vec3(0.04);
        F0 = mix(F0, albedo, metallic);

        vec3 F = fresnelSchlickRoughness(max(dot(V, N), 0.0), F0, roughness);

        vec3 kS = F;
        vec3 kD = vec3(1.0) - kS;
        // kD *= (1.0 - ao);

        vec3 irradiance = texture(irradianceMap, N).rgb;
        vec3 diffuse = irradiance * albedo;

        const float MAX_REFLECTION_LOD = 4.0;
        vec3 prefilteredColor = textureLod(prefilterMap, R, roughness * MAX_REFLECTION_LOD).rgb;
        vec2 brdf = texture(brdfLUT, vec2(max(dot(N, V), 0.0), roughness)).rg;
        vec3 specular = prefilteredColor * (F * brdf.x + brdf.y);

        vec3 ambient = (kD * diffuse + specular + emissive) * ao;


        color = ambient;
        color = color / (color + vec3(1.0));
        color = pow(color, vec3(1.0 / 2.2));
    } else if (renderMode == 1) {
        color = albedo;
    } else if (renderMode == 2) {
        color = N;
    } else if (renderMode == 3) {
        color = vec3(metallic, metallic, metallic);
    } else if (renderMode == 4) {
        color = vec3(roughness, roughness, roughness);
    } else if (renderMode == 5) {
        color = emissive;
    } else if (renderMode == 6) {
        color = vec3(ao, ao, ao);
    }


   

   

    gl_FragColor = vec4(color, 1.0);
}
)";

void GLPBRRenderPass::init(SceneGraph &scene)
{
    mProgram = GLDevice::createProgram(pbrVS, pbrFS);
    GLDevice::useProgram(mProgram);
    GLDevice::setUniform(mProgram, "view", scene.camera.view);
    GLDevice::setUniform(mProgram, "proj", scene.camera.proj);
    GLDevice::setUniform(mProgram, "camPos", scene.camera.eye);
    GLDevice::setUniform(mProgram, "renderMode", 0);

    auto rootNode = scene.nodes[0];

    for (int i = 0; i < scene.meshes.size(); ++i)
    {
        auto &primitive = scene.meshes[i];

        primitive.vao = GLDevice::createVAO(primitive.vertices, primitive.indices);
    }

    TextureDescriptor textureDescriptor({
        .target = GL_TEXTURE_2D,
        .internalFormat = GL_RGBA8,
        .minFilterMode = GL_LINEAR_MIPMAP_LINEAR,
        .magFilterMode = GL_LINEAR,
        .wrapSMode = GL_REPEAT,
        .wrapTMode = GL_REPEAT,
    });

    for (int i = 0; i < scene.materials.size(); ++i)
    {
        auto &material = scene.materials[i];
        if (material.hasAlbedoMap)
        {
            auto &texture = scene.textures[material.albedoMap];
            textureDescriptor.data = texture.data.data();
            textureDescriptor.width = texture.width;
            textureDescriptor.height = texture.height;
            texture.handle = GLDevice::createTexture(textureDescriptor);
            GLDevice::setUniform(mProgram, "hasAlbedoMap", true);
        }
        else
        {
            GLDevice::setUniform(mProgram, "hasAlbedoMap", false);
            GLDevice::setUniform(mProgram, "albedo_", material.albedo);
        }
        if (material.hasNormalMap)
        {
            auto &texture = scene.textures[material.normalMap];
            textureDescriptor.data = texture.data.data();
            textureDescriptor.width = texture.width;
            textureDescriptor.height = texture.height;
            texture.handle = GLDevice::createTexture(textureDescriptor);
            GLDevice::setUniform(mProgram, "hasNormalMap", true);
        }
        else
        {
            GLDevice::setUniform(mProgram, "hasNormalMap", false);
        }

        if (material.hasMetallicRoughnessMap)
        {
            auto &texture = scene.textures[material.metallicRoughnessMap];
            textureDescriptor.data = texture.data.data();
            textureDescriptor.width = texture.width;
            textureDescriptor.height = texture.height;
            texture.handle = GLDevice::createTexture(textureDescriptor);
            GLDevice::setUniform(mProgram, "hasMetallicRoughnessMap", true);
        }
        else
        {
            GLDevice::setUniform(mProgram, "hasMetallicRoughnessMap", false);
            GLDevice::setUniform(mProgram, "metallic_", material.metallic);
            GLDevice::setUniform(mProgram, "roughness_", material.roughness);
        }

        if (material.hasEmissiveMap)
        {
            auto &texture = scene.textures[material.emissiveMap];
            textureDescriptor.data = texture.data.data();
            textureDescriptor.width = texture.width;
            textureDescriptor.height = texture.height;
            texture.handle = GLDevice::createTexture(textureDescriptor);
            GLDevice::setUniform(mProgram, "hasEmissiveMap", true);
        }
        else
        {
            GLDevice::setUniform(mProgram, "hasEmissiveMap", false);
            GLDevice::setUniform(mProgram, "emissive_", material.emissive);
        }

        if (material.hasAOMap)
        {
            auto &texture = scene.textures[material.aoMap];
            textureDescriptor.data = texture.data.data();
            textureDescriptor.width = texture.width;
            textureDescriptor.height = texture.height;
            texture.handle = GLDevice::createTexture(textureDescriptor);
            GLDevice::setUniform(mProgram, "hasAoMap", true);
        }
        else
        {
            GLDevice::setUniform(mProgram, "hasAoMap", false);
            GLDevice::setUniform(mProgram, "ao_", material.ao);
        }
    }
}

void GLPBRRenderPass::renderNode(SceneGraph &scene, int32_t index)
{
    if (!frustum.IsBoxVisible(scene.bboxes[index])) {
        printf(" not in frustum\n");
        return;
    }
    auto &node = scene.nodes[index];
    auto &model = scene.worldMatrices[index];
    
    if (node.meshes.size() > 0)
    {
        for (U32 i = 0; i < node.meshes.size(); ++i) {

           

            auto &mesh = scene.meshes[node.meshes[i]];

            if(!frustum.IsBoxVisible(mesh.bbox.transform(model))) {
                printf("mesh not in frustum\n");
                continue;
            }

            auto &material = scene.materials[mesh.material];
            auto &vao = mesh.vao;
            GLDevice::setUniform(mProgram, "model", model);
            glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(model)));
            GLDevice::setUniform(mProgram, "normalMat", normalMatrix);
            GLDevice::setUniform(mProgram, "proj", scene.camera.proj);
            // TODO: check camera dirty flag only set this when camera is dirty
            if (material.hasAlbedoMap)
            {
                GLDevice::setUniform(mProgram, "hasAlbedoMap", true);
                auto handle = scene.textures[material.albedoMap].handle;
                GLDevice::bindTexture(handle, 0);
            }
            else
            {
                GLDevice::setUniform(mProgram, "hasAlbedoMap", false);
                GLDevice::setUniform(mProgram, "albedo_", material.albedo);
            }
            if (material.hasNormalMap)
            {
                GLDevice::setUniform(mProgram, "hasNormalMap", true);
                auto handle = scene.textures[material.normalMap].handle;
                GLDevice::bindTexture(handle, 1);
            }
            else
            {
                GLDevice::setUniform(mProgram, "hasNormalMap", false);
            }
            if (material.hasMetallicRoughnessMap)
            {
                GLDevice::setUniform(mProgram, "hasMetallicRoughnessMap", true);
                auto handle = scene.textures[material.metallicRoughnessMap].handle;
                GLDevice::bindTexture(handle, 2);
            }
            else
            {
                GLDevice::setUniform(mProgram, "hasMetallicRoughnessMap", false);
                GLDevice::setUniform(mProgram, "metallic_", material.metallic);
                GLDevice::setUniform(mProgram, "roughness_", material.roughness);
            }

            if (material.hasAOMap)
            {
                GLDevice::setUniform(mProgram, "hasAoMap", true);
                auto handle = scene.textures[material.aoMap].handle;
                GLDevice::bindTexture(handle, 3);
            }
            else
            {
                GLDevice::setUniform(mProgram, "hasAoMap", false);
                GLDevice::setUniform(mProgram, "ao_", material.ao);
            }

            if (material.hasEmissiveMap)
            {
                GLDevice::setUniform(mProgram, "hasEmissiveMap", true);
                auto handle = scene.textures[material.emissiveMap].handle;
                GLDevice::bindTexture(handle, 4);
            }
            else
            {
                GLDevice::setUniform(mProgram, "hasEmissiveMap", false);
                GLDevice::setUniform(mProgram, "emissive_", material.emissive);
            }
            GLDevice::drawVAO(vao);
        }
        
    }
    for (int i = 0; i < node.children.size(); ++i)
    {
        renderNode(scene, node.children[i]);
    }
}

void GLPBRRenderPass::render(SceneGraph &scene)
{
    glEnable(GL_CULL_FACE);
    rotation += 0.1f;
    auto rot = glm::rotate(glm::mat4(1.0f), glm::radians(rotation), glm::vec3(0.0f, 1.0f, 0.0f));
    // auto finalModel = rot * model;
    GLDevice::useProgram(mProgram);
    GLDevice::setUniform(mProgram, "view", scene.camera.view);
    GLDevice::setUniform(mProgram, "camPos", scene.camera.eye);
    GLDevice::setUniform(mProgram, "renderMode", renderMode);
    GLDevice::bindTexture(irrdianceMap, 5);
    GLDevice::bindTexture(prefilterMap, 6);
    GLDevice::bindTexture(brdfLut, 7);
    frustum = Frustum(scene.camera.proj * scene.camera.view);
    renderNode(scene, 0);
    glDisable(GL_CULL_FACE);
}