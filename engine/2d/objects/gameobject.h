#pragma once 

#include "plane.h"
#include <vector>

struct AnimFrame
{
public:
    float time;
    GLuint texture;
};

//-----------------------------------------------
class GameObject
{
private:
    Plane* plane;

public:
    GameObject(float x, float y, float width, float height, GLuint textureID=0)  
    {
        // Constructor implementation (if needed)
        plane = new Plane(x, y, width, height, textureID);
    }

    Plane* GetPlane() const { return plane; }

    virtual void Update(float deltaTime)
    {
        // Default implementation does nothing
    }

    virtual ~GameObject() {
        // Destructor implementation (if needed)
        delete plane;
    }

};