#include "GLPrefilterRenderPass.h"
#include "Renderer.h"
const char *prefilterVS = R"(
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

const char *prefilterFS = R"(
#version 460 core
out vec4 FragColor;
in vec3 WorldPos;

layout (binding = 0) uniform samplerCube environmentMap;
uniform float roughness;

const float PI = 3.14159265359;
// ----------------------------------------------------------------------------
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
// ----------------------------------------------------------------------------
// http://holger.dammertz.org/stuff/notes_HammersleyOnHemisphere.html
// efficient VanDerCorpus calculation.
float RadicalInverse_VdC(uint bits) 
{
     bits = (bits << 16u) | (bits >> 16u);
     bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
     bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
     bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
     bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
     return float(bits) * 2.3283064365386963e-10; // / 0x100000000
}
// ----------------------------------------------------------------------------
vec2 Hammersley(uint i, uint N)
{
	return vec2(float(i)/float(N), RadicalInverse_VdC(i));
}
// ----------------------------------------------------------------------------
vec3 ImportanceSampleGGX(vec2 Xi, vec3 N, float roughness)
{
	float a = roughness*roughness;
	
	float phi = 2.0 * PI * Xi.x;
	float cosTheta = sqrt((1.0 - Xi.y) / (1.0 + (a*a - 1.0) * Xi.y));
	float sinTheta = sqrt(1.0 - cosTheta*cosTheta);
	
	// from spherical coordinates to cartesian coordinates - halfway vector
	vec3 H;
	H.x = cos(phi) * sinTheta;
	H.y = sin(phi) * sinTheta;
	H.z = cosTheta;
	
	// from tangent-space H vector to world-space sample vector
	vec3 up          = abs(N.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
	vec3 tangent   = normalize(cross(up, N));
	vec3 bitangent = cross(N, tangent);
	
	vec3 sampleVec = tangent * H.x + bitangent * H.y + N * H.z;
	return normalize(sampleVec);
}
// ----------------------------------------------------------------------------
void main()
{		
    vec3 N = normalize(WorldPos);
    
    // make the simplifying assumption that V equals R equals the normal 
    vec3 R = N;
    vec3 V = R;

    const uint SAMPLE_COUNT = 1024u;
    vec3 prefilteredColor = vec3(0.0);
    float totalWeight = 0.0;
    
    for(uint i = 0u; i < SAMPLE_COUNT; ++i)
    {
        // generates a sample vector that's biased towards the preferred alignment direction (importance sampling).
        vec2 Xi = Hammersley(i, SAMPLE_COUNT);
        vec3 H = ImportanceSampleGGX(Xi, N, roughness);
        vec3 L  = normalize(2.0 * dot(V, H) * H - V);

        float NdotL = max(dot(N, L), 0.0);
        if(NdotL > 0.0)
        {
            // sample from the environment's mip level based on roughness/pdf
            float D   = DistributionGGX(N, H, roughness);
            float NdotH = max(dot(N, H), 0.0);
            float HdotV = max(dot(H, V), 0.0);
            float pdf = D * NdotH / (4.0 * HdotV) + 0.0001; 

            float resolution = 512.0; // resolution of source cubemap (per face)
            float saTexel  = 4.0 * PI / (6.0 * resolution * resolution);
            float saSample = 1.0 / (float(SAMPLE_COUNT) * pdf + 0.0001);

            float mipLevel = roughness == 0.0 ? 0.0 : 0.5 * log2(saSample / saTexel); 
            
            prefilteredColor += textureLod(environmentMap, L, mipLevel).rgb * NdotL;
            totalWeight      += NdotL;
        }
    }

    prefilteredColor = prefilteredColor / totalWeight;

    FragColor = vec4(prefilteredColor, 1.0);
}
)";
void GLPrefilterRenderPass::init(SceneGraph &scene)
{
    prefilterMap = GLDevice::createTexture(TextureDescriptor{
        .width = 128,
        .height = 128,
        .mipmaps = maxMipLevels,
        .target = GL_TEXTURE_CUBE_MAP,
        .internalFormat = GL_RGB16F,
        .minFilterMode = GL_LINEAR_MIPMAP_LINEAR,
        .magFilterMode = GL_LINEAR,
        .wrapSMode = GL_CLAMP_TO_EDGE,
        .wrapTMode = GL_CLAMP_TO_EDGE,
        .wrapRMode = GL_CLAMP_TO_EDGE,
    });
    // glTextureParameteri(prefilterMap.handle(), GL_TEXTURE_MAX_LEVEL, 0);
    // glTextureParameteri(prefilterMap.handle(), GL_TEXTURE_MAX_LEVEL, 5);
    GLDevice::generateMipmap(prefilterMap);

    // check mipmap completeness

    mProgram = GLDevice::createProgram(prefilterVS, prefilterFS);

    FramebufferDescriptor descriptor{
        .width = 128,
        .height = 128,
        .depthFormat = GL_DEPTH_COMPONENT24};
    captureFBO = GLDevice::createFramebuffer(descriptor);
}

void GLPrefilterRenderPass::render(SceneGraph &scene)
{

    // query the maximum number of mip levels
    GLDevice::useProgram(mProgram);
    GLDevice::setUniform(mProgram, "proj", Renderer::CAPTURE_PROJECTION);
    GLDevice::bindTexture(mCubeMap, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO.handle);
    for (unsigned int mip = 0; mip < maxMipLevels; ++mip)
    {
        unsigned int mipWidth = static_cast<unsigned int>(128 * std::pow(0.5, mip));
        unsigned int mipHeight = static_cast<unsigned int>(128 * std::pow(0.5, mip));
        glBindRenderbuffer(GL_RENDERBUFFER, captureFBO.depthbuffer.handle);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mipWidth, mipHeight);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, captureFBO.depthbuffer.handle);
        glViewport(0, 0, mipWidth, mipHeight);
        float roughness = static_cast<float>(mip) / static_cast<float>(maxMipLevels - 1);
        GLDevice::setUniform(mProgram, "roughness", roughness);
        for (unsigned int i = 0; i < 6; ++i)
        {
            GLDevice::setUniform(mProgram, "view", Renderer::CAPTURE_VIEW[i]);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, prefilterMap.handle, mip);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            GLDevice::drawVAO(Renderer::CUBE_HANDLE);
        }
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}