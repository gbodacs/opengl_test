#pragma once

#include "../platform/gl_platform.h"
#include "math.h"

#include <vector>

struct MeshVertex
{
    Vec3f3D position;
    Vec2f3D uv;
    Vec3f3D normal;
};

class Mesh3D
{
public:
    virtual ~Mesh3D() = default;

    std::vector<MeshVertex> vertices;
    std::vector<unsigned int> indices;
    // Cached 16-bit copy of indices, since GLES 1.1 only guarantees
    // GL_UNSIGNED_SHORT support in glDrawElements. Populated lazily by the renderer.
    std::vector<unsigned short> indices16;
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

    virtual void Update(float deltaSeconds)
    {
        (void)deltaSeconds;
    }
};