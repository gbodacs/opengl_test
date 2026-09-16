#pragma once

#include <GL/glew.h>

#include <vector>

struct Vec2f3D
{
    float x = 0.0f;
    float y = 0.0f;
};

struct Vec3f3D
{
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
};

struct MeshVertex
{
    Vec3f3D position;
    Vec2f3D uv;
    Vec3f3D normal;
};

class Mesh3D
{
public:
    std::vector<MeshVertex> vertices;
    std::vector<unsigned int> indices;
    GLuint textureID = 0;
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    float rotationX = 0.0f;
    float rotationY = 0.0f;
    float rotationZ = 0.0f;
    float scaleX = 1.0f;
    float scaleY = 1.0f;
    float scaleZ = 1.0f;
    float red = 1.0f;
    float green = 1.0f;
    float blue = 1.0f;

    bool IsValid() const
    {
        return !vertices.empty() && !indices.empty();
    }

    void SetTexture(GLuint texID)
    {
        textureID = texID;
    }
};