#include "Geometry.h"
Mesh createCube(float size)
    {
        auto side2 = size / 2.0f;
        std::vector<float> p = {
            // Front
            -side2, -side2, side2, side2, -side2, side2, side2, side2, side2, -side2, side2, side2,
            // Right
            side2, -side2, side2, side2, -side2, -side2, side2, side2, -side2, side2, side2, side2,
            // Back
            -side2, -side2, -side2, -side2, side2, -side2, side2, side2, -side2, side2, -side2, -side2,
            // Left
            -side2, -side2, side2, -side2, side2, side2, -side2, side2, -side2, -side2, -side2, -side2,
            // Bottom
            -side2, -side2, side2, -side2, -side2, -side2, side2, -side2, -side2, side2, -side2, side2,
            // Top
            -side2, side2, side2, side2, side2, side2, side2, side2, -side2, -side2, side2, -side2};

        std::vector<float> n = {
            // Front
            0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
            // Right
            1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
            // Back
            0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f,
            // Left
            -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,
            // Bottom
            0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f,
            // Top
            0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f};

        std::vector<float> tex = {
            // Front
            0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f,
            // Right
            0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f,
            // Back
            0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f,
            // Left
            0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f,
            // Bottom
            0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f,
            // Top
            0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f};

        std::vector<Vertex> vertices(p.size() / 3);

        for (int i = 0; i < vertices.size(); i++)
        {
            vertices[i].position = glm::vec3(p[i * 3], p[i * 3 + 1], p[i * 3 + 2]);
            vertices[i].normal = glm::vec3(n[i * 3], n[i * 3 + 1], n[i * 3 + 2]);
            vertices[i].uv = glm::vec2(tex[i * 2], tex[i * 2 + 1]);
        }

        std::vector<unsigned int> el = {
            0, 1, 2, 0, 2, 3,
            4, 5, 6, 4, 6, 7,
            8, 9, 10, 8, 10, 11,
            12, 13, 14, 12, 14, 15,
            16, 17, 18, 16, 18, 19,
            20, 21, 22, 20, 22, 23};

        return {.vertices = vertices, .indices = el};
    }

    Mesh createQuad()
    {
        std::vector<float> p = {
            -1.0f, 1.0f, 0.0f,
            -1.0f, -1.0f, 0.0f,
            1.0f, -1.0f, 0.0f,
            1.0f, 1.0f, 0.0f};

        std::vector<float> n = {
            0.0f, 0.0f, 1.0f,
            0.0f, 0.0f, 1.0f,
            0.0f, 0.0f, 1.0f,
            0.0f, 0.0f, 1.0f};

        std::vector<float> tex = {
            0.0f, 1.0f,
            0.0f, 0.0f,
            1.0f, 0.0f,
            1.0f, 1.0f};

        std::vector<Vertex> vertices(4);
        for (int i = 0; i < vertices.size(); i++)
        {
            vertices[i].position = glm::vec3(p[i * 3], p[i * 3 + 1], p[i * 3 + 2]);
            vertices[i].normal = glm::vec3(n[i * 3], n[i * 3 + 1], n[i * 3 + 2]);
            vertices[i].uv = glm::vec2(tex[i * 2], tex[i * 2 + 1]);
        }

        std::vector<unsigned int>
            indices = {0, 1, 2, 0, 2, 3};

        return {
            vertices,
            indices,
        };
    }