#pragma once

#include "engine.h"
#include "input.h"
#include "plane.h"
#include "input.h"
#include "sound.h"
#include "player.h"
#include "enemy.h"



class cGame
{
protected:
    Enemy*              enemy;
    Player*             player;

    bool UpdateInput(float deltaTime);

public:
    Engine*     engine;
    Input*      input;
    Sound*      sound;
    BitmapFont  uiFont;

    bool playerPrevUp = false;
    bool playerPrevDown = false;
    bool playerPrevLeft = false;
    bool playerPrevRight = false;
    
    bool Init();
    bool Run();
    bool Done();
};