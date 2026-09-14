#pragma once

#include "objects/animatedgameobject.h"

//-----------------------------------------------
class Enemy : public AnimatedGameObject
{
public:
    void Move(float deltaX, float deltaY)
    {
        GetPlane()->x += deltaX;
        GetPlane()->y += deltaY;
    }

    Enemy(float x, float y, float width, float height, GLuint textureID=0)
    : AnimatedGameObject(x, y, width, height, textureID)
    {
    }
    
    virtual ~Enemy() {
    }
};