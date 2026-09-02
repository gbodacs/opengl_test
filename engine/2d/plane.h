#pragma once
#include <GL/glew.h>
#include <GLFW/glfw3.h>

struct Plane {
    float x, y, width, height;
    GLuint textureID=0;
    float red=1.0f, green=1.0f, blue=1.0f; // Alapértelmezett szín fehér

    bool Intersects(const Plane& other) {
        return (x < other.x + other.width &&
                x + width > other.x &&
                y < other.y + other.height &&
                y + height > other.y);
    }

    void SetColor(float r, float g, float b) {
        red = r;
        green = g;
        blue = b;
    }
};