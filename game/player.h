#pragma once

#include "objects/animatedgameobject.h"

enum PlayerAnims
{
    Idle = 0,
    Jump = 1,
    Run = 2
};

class Player : public AnimatedGameObject
{
public:
    Player(float x, float y, float width, float height, GLuint textureID=0)
    : AnimatedGameObject(x, y, width, height, textureID)
    {
    }

    virtual ~Player() {
    }
};