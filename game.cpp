#include "game.h"


bool cGame::Init()
{
    engine = new Engine();
    if (!engine->Init(1024, 768, "My 2D Engine")) 
        return false;

    GLuint playerTex = engine->LoadTexture("data/player.png");

    player = engine->CreatePlane(40.0f, 40.0f, 304.0f, 304.0f, playerTex);
    enemy = engine->CreatePlane(10.0f, 10.0f, 64.0f, 64.0f, playerTex); // Reusing player texture for simplicity
    return true;
}


bool cGame::Run()
{
    bool quit = false;
    while (!glfwWindowShouldClose(engine->window) && !quit) 
    {    
        engine->UpdateStart();
        { //input
            glfwPollEvents(); // Necessary to update internal GLFW states

            // Poll Keyboard
            if (InputHandler::GetKey(engine->window, GLFW_KEY_W) == PRESSED) {
                player->y -= 10.0f; // Move up
            }
            if (InputHandler::GetKey(engine->window, GLFW_KEY_S) == PRESSED) {
                player->y += 10.0f; // Move down
            }
            if (InputHandler::GetKey(engine->window, GLFW_KEY_D) == PRESSED) {
                player->x += 10.0f; // Move right
            }
            if (InputHandler::GetKey(engine->window, GLFW_KEY_A) == PRESSED) {
                player->x -= 10.0f; // Move left
            }
            if (InputHandler::GetKey(engine->window, GLFW_KEY_ESCAPE) == PRESSED) {
                quit = true;
            }

            // Poll Gamepad
            ControllerState pad = PollGamepad(GLFW_JOYSTICK_1);
            if (pad.connected) {
                player->x += pad.leftStickX * 0.01f;
                player->y -= pad.leftStickY * 0.01f; // Y is usually inverted on sticks
            }
        }

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
    engine->Done();
    delete engine;
    return true;
}

int main() {
    printf("Start\n\r");
    cGame Game;
    if (Game.Init())
    {
        Game.Run();
        Game.Done();
    }
}