#include "game.h"

ma_sound startSound;

bool cGame::Init()
{
    engine = new Engine();
    if (!engine->Init(1024, 768, "My 2D Engine")) 
        return false;

    input = new Input();
    input->Init();

    sound = new Sound();
    if (!sound->Init())
        return false;

    if (!sound->LoadSounds())
        return false;

    sound->PlaySound(SOUND_START);

    GLuint playerTex = engine->LoadTexture("data/graph/player.png");

    player = engine->CreatePlane(40.0f, 40.0f, 304.0f, 304.0f, playerTex);
    enemy = engine->CreatePlane(10.0f, 10.0f, 64.0f, 64.0f, playerTex); // Reusing player texture for simplicity
    return true;
}

bool cGame::UpdateInput(float deltaTime)
{
    // Poll Keyboard
    if (input->GetKey(engine->window, GLFW_KEY_W) == PRESSED) {
        player->y -= deltaTime; // Move up
    }
    if (input->GetKey(engine->window, GLFW_KEY_S) == PRESSED) {
        player->y += deltaTime; // Move down
    }
    if (input->GetKey(engine->window, GLFW_KEY_D) == PRESSED) {
        player->x += deltaTime; // Move right
    }
    if (input->GetKey(engine->window, GLFW_KEY_A) == PRESSED) {
        player->x -= deltaTime; // Move left
        sound->PlaySound(SOUND_END); // Play sound on pressing 'A'
    
    }

    //Exit?
    if (input->GetKey(engine->window, GLFW_KEY_ESCAPE) == PRESSED) {
        return true; // Signal to quit
    }

    // Poll Gamepad
    ControllerState pad = input->GetGamepad(GLFW_JOYSTICK_1);
    if (pad.connected) 
    {
        if (pad.leftStickX < 0.1f && pad.leftStickX > -0.1f) pad.leftStickX = 0.0f; // Deadzone
        if (pad.leftStickY < 0.08f && pad.leftStickY > -0.08f) pad.leftStickY = 0.0f; // Deadzone

        player->x += pad.leftStickX * deltaTime; // Adjust speed as needed
        player->y += pad.leftStickY * deltaTime; // Y is usually inverted on sticks
    }

    return false;
}

bool cGame::Run()
{
    bool quit = false;
    while (!glfwWindowShouldClose(engine->window) && !quit) 
    {
        float dTime = engine->UpdateStart();
        quit = UpdateInput(dTime);

        engine->RenderAllPlanes();

        engine->UpdateEnd();
        
        if (player->Intersects(*enemy)) {
            // Logic for collision
            printf("Collision\n\r");
        }
    }
    return true;
}

bool cGame::Done()
{   
    sound->Done();
    delete sound;

    delete input;

    engine->Done();
    delete engine;

    return true;
}

int main() 
{
    printf("Start\n\r");
    cGame Game;
    if (Game.Init())
    {
        Game.Run();
        Game.Done();
    }
}