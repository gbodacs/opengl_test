#pragma once
#include <GL/glew.h>
#include <GLFW/glfw3.h>

class Plane 
{
public:
    float   x, y, width, height;
    GLuint  textureID=0;
    bool    mirrored = false;
    float   red=1.0f, green=1.0f, blue=1.0f; // White
    float   u0 = 0.0f, v0 = 0.0f, u1 = 1.0f, v1 = 1.0f;

    Plane(float x, float y, float width, float height, GLuint textureID=0) {
        this->x = x;
        this->y = y;
        this->width = width;
        this->height = height;
        this->textureID = textureID;
    }

    virtual ~Plane() {
        // Destructor implementation (if needed)
    }

    bool Intersects(const Plane& other) 
    {
        return (x < other.x + other.width &&
                x + width > other.x &&
                y < other.y + other.height &&
                y + height > other.y);
    }

    void SetColor(float r, float g, float b) 
    {
        red = r;
        green = g;
        blue = b;
    }

    void SetTexture(GLuint texID) 
    {
        textureID = texID;
    }

    void SetUV(float _u0, float _v0, float _u1, float _v1)
    {
        u0 = _u0;
        v0 = _v0;
        u1 = _u1;
        v1 = _v1;
    }
};