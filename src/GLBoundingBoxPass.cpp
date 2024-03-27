#include "GLBoundingBoxPass.h"

const char *bboxVS = R"(
#version 460 core
layout (location = 0) in vec3 position;
uniform mat4 view;
uniform mat4 proj;
uniform mat4 model;

void main() {
    gl_Position = proj * view * model * vec4(position, 1.0);
}
)";

const char *bboxFS = R"( 
#version 460 core
out vec4 FragColor;

void main() {
    FragColor = vec4(1.0, 0.0, 0.0, 1.0);
}
)";

void GLBoundingBoxPass::init(SceneGraph &scene)
{

    mProgram = GLDevice::createProgram(bboxVS, bboxFS);

    GLfloat vertices[] = {
        -0.5,
        -0.5,
        -0.5,
        1.0,
        0.5,
        -0.5,
        -0.5,
        1.0,
        0.5,
        0.5,
        -0.5,
        1.0,
        -0.5,
        0.5,
        -0.5,
        1.0,
        -0.5,
        -0.5,
        0.5,
        1.0,
        0.5,
        -0.5,
        0.5,
        1.0,
        0.5,
        0.5,
        0.5,
        1.0,
        -0.5,
        0.5,
        0.5,
        1.0,
    };
    glCreateVertexArrays(1, &mVAO);
    glCreateBuffers(1, &mVBO);
    glNamedBufferStorage(mVBO, sizeof(vertices), vertices, GL_DYNAMIC_STORAGE_BIT);

    glVertexArrayVertexBuffer(mVAO, 0, mVBO, 0, 4 * sizeof(GLfloat));

    glEnableVertexArrayAttrib(mVAO, 0);
    glVertexArrayAttribFormat(mVAO, 0, 4, GL_FLOAT, GL_FALSE, 0);
    glVertexArrayAttribBinding(mVAO, 0, 0);

    GLuint elements[] = {
        0, 1, 2, 3,
        4, 5, 6, 7,
        0, 4, 1, 5,
        2, 6, 3, 7};
    glCreateBuffers(1, &mEBO);
    glNamedBufferStorage(mEBO, sizeof(elements), elements, GL_DYNAMIC_STORAGE_BIT);
    glVertexArrayElementBuffer(mVAO, mEBO);
}

void GLBoundingBoxPass::renderNode(SceneGraph &scene, int32_t index)
{
    auto &node = scene.nodes[index];
    auto model = scene.worldMatrices[index];
    if (node.meshes.size() > 0) {
        for(auto meshIndex: node.meshes) {
            auto &mesh = scene.meshes[meshIndex];
            auto bbox = mesh.bbox;
            glm::vec3 size = bbox.getSize();
            glm::vec3 center = bbox.center;
            glm::mat4 bboxMatrix = glm::translate(glm::mat4(1.0f), center) * glm::scale(glm::mat4(1.0f), size);
            GLDevice::setUniform(mProgram, "model", model * bboxMatrix);
            glBindVertexArray(mVAO);
            glDrawElements(GL_LINE_LOOP, 4, GL_UNSIGNED_INT, 0);
            glDrawElements(GL_LINE_LOOP, 4, GL_UNSIGNED_INT, (void *)(4 * sizeof(GLuint)));
            glDrawElements(GL_LINES, 8, GL_UNSIGNED_INT, (void *)(8 * sizeof(GLuint)));
        }       
    }
   
    for (U32 i = 0; i < node.children.size(); ++i)
    {
        renderNode(scene, node.children[i]);
    }
}

void GLBoundingBoxPass::render(SceneGraph &scene)
{
    GLDevice::useProgram(mProgram);
    GLDevice::setUniform(mProgram, "view", scene.camera.view);
    GLDevice::setUniform(mProgram, "proj", scene.camera.proj);
    renderNode(scene, 0);
    glBindVertexArray(0);
}