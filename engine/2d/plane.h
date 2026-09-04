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

    virtual void Update(float deltaTime) 
    {
        // Default implementation does nothing
    }
};