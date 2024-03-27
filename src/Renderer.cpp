#include "Renderer.h"
#include "Geometry.h"
#include "Window.h"
#include "GLDevice.h"
#include "gui.h"
void debugMessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, const void *userParam)
{
    fprintf(stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
            (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""),
            type, severity, message);
}

const char *testVS = R"(
#version 460 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aUV;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat3 normalMatrix;

void main() {
    FragPos = vec3(model * vec4(aPos, 1.0));
    Normal = normalize(normalMatrix * aNormal);
    TexCoords = aUV;
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}
)";

const char *testFS = R"(
#version 460 core

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

out vec4 FragColor;

void main() {
    FragColor = vec4(1.0, 0.5, 0.2, 1.0);
}
)";

glm::mat4 Renderer::CAPTURE_PROJECTION = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
glm::mat4 Renderer::CAPTURE_VIEW[6] = {
    glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
    glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
    glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
    glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)),
    glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
    glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f))};

VAOHandle Renderer::CUBE_HANDLE = VAOHandle{};

std::vector<std::string> modelPaths = {
    "resources/gltf/helmet/DamagedHelmet.gltf",
    "resources/gltf/buggy/Buggy.gltf",
    "resources/gltf/corset/Corset.gltf",
    "resources/gltf/engine/2CylinderEngine.gltf",
    "resources/gltf/scifihelmet/SciFiHelmet.gltf",
    "resources/gltf/waterbottle/WaterBottle.gltf",
};

Renderer::Renderer(Window *window) : mWindow(window)
{
    // glfwSwapInterval(1);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    // glEnable(GL_CULL_FACE);
    // enable faceculling
    
    // glCullFace(GL_BACK);
    // glFrontFace(GL_CCW);

    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(debugMessageCallback, 0);
    glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
    int width, height;
    mWindow->getSize(&width, &height);
    mWidth = static_cast<U32>(width);
    mHeight = static_cast<U32>(height);

    mMenuState.renderMode = 0;
    mMenuState.renderModes = {"pbr",
                              "albedo",
                              "normal",
                              "metallic",
                              "roughness",
                              "emissive",
                              "ao"};
    mMenuState.modelPath = {
        "DamagedHelmet.gltf",
        "Buggy.gltf",
        "Corset.gltf",
        "2CylinderEngine.gltf",
        "SciFiHelmet.gltf",
        "Sponza.gltf",
        "WaterBottle.gltf"};

    mMenuState.modelIndex = mModelIndex;

    mMenuState.environmentIndex = 0;
    mMenuState.environmentPath = {
        "test",
        "test"};
    GUI::init(mWindow->getWindow());
}

void Renderer::initScene(SceneGraph &sceneGraph)
{

    sceneGraph.initialized = true;
    auto cube = createCube(2.0f);
    CUBE_HANDLE = GLDevice::createVAO(cube.vertices, cube.indices);
    mDefaultProgram = GLDevice::createProgram(testVS, testFS);

    mHDRRenderPass.init(sceneGraph);
    mHDRRenderPass.render(sceneGraph);

    auto envCubeMap = mHDRRenderPass.getEnvCubeMap();

    mIrradianceRenderPass.init(sceneGraph);
    mIrradianceRenderPass.setEnvCubemap(envCubeMap);
    mIrradianceRenderPass.render(sceneGraph);

    mPrefilterRenderPass.init(sceneGraph);
    mPrefilterRenderPass.setEnvCubemap(envCubeMap);
    mPrefilterRenderPass.render(sceneGraph);

    mBrdfRenderPass.init(sceneGraph);
    mBrdfRenderPass.render(sceneGraph);

    mPBRRenderPass.init(sceneGraph);
    mPBRRenderPass.setIrradianceMap(mIrradianceRenderPass.getIrradianceMap());
    mPBRRenderPass.setBrdfLut(mBrdfRenderPass.getBrdfLut());
    mPBRRenderPass.setPrefilterMap(mPrefilterRenderPass.getPrefilterMap());

    mSkyboxRenderPass.init(sceneGraph);
    mSkyboxRenderPass.setEnvCubeMap(envCubeMap);

    mBoundingboxPass.init(sceneGraph);
}

void Renderer::render(SceneGraph &sceneGraph)
{
    // glBindFramebuffer(GL_FRAMEBUFFER, 0);

    if (mMenuState.modelIndex != mModelIndex)
    {
        sceneGraph.clear();
        sceneGraph.load(modelPaths[mMenuState.modelIndex].c_str());
        mModelIndex = mMenuState.modelIndex;
    }
    if (!sceneGraph.initialized)
    {
        initScene(sceneGraph);
    }

    glViewport(0, 0, mWidth, mHeight);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    mPBRRenderPass.setRenderMode(mMenuState.renderMode);
    mPBRRenderPass.render(sceneGraph);
    // mBoundingboxPass.render(sceneGraph);
    mSkyboxRenderPass.render(sceneGraph);
    GUI::newFrame();

    GUI::drawMenu(mMenuState);

    GUI::endFrame();
    GUI::draw();
}

void Renderer::resize(U32 width, U32 height)
{
    mWidth = width;
    mHeight = height;
}