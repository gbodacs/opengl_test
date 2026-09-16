#pragma once

#include "engine.h"
#include "input.h"
#include "plane.h"
#include "input.h"
#include "sound.h"
#include "player.h"
#include "enemy.h"
#include "menu.h"


enum class GameStates
{
    Menu,
    Playing,
    Paused,
    GameOver
};

class cGame
{
protected:
    Enemy*              enemy;
    Player*             player;
    Mesh3D*             demoMesh = nullptr;
    GameStates          gameState=GameStates::Playing;

    bool UpdateInput(float deltaTime);

public:
    Engine*     engine;
    Input*      input;
    Sound*      sound;
    BitmapFont  uiFont;
    Menu*       menu;

    bool playerPrevUp = false;
    bool playerPrevDown = false;
    bool playerPrevLeft = false;
    bool playerPrevRight = false;
    bool playerPrevJump = false;
    float demoMeshSpin = 0.0f;
    
    bool Init();
    bool Run();
    // Runs exactly one frame (input poll, game logic, render, present). Used
    // directly by Run()'s loop on desktop, and driven by the Android event
    // loop's per-iteration tick on Android (see game/android_main.cpp).
    void Step();
    bool Done();
};