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

    GLuint dinoTex = engine->LoadTexture("data/graph/dino.png");
    GLuint boy1 = engine->LoadTexture("data/graph/boy1.png");
    GLuint boy2 = engine->LoadTexture("data/graph/boy2.png");
    GLuint boy3 = engine->LoadTexture("data/graph/boy3.png");
    GLuint boy4 = engine->LoadTexture("data/graph/boy4.png");
    GLuint boy5 = engine->LoadTexture("data/graph/boy5.png");
    GLuint boy6 = engine->LoadTexture("data/graph/boy6.png");

    player = engine->CreateAnimatedGameObject(40.0f, 40.0f, 131.0f, 251.0f);
    player->AddFrame(0.15f, boy1);
    player->AddFrame(0.3f, boy2);
    player->AddFrame(0.45f, boy3);
    player->AddFrame(0.6f, boy4);
    player->AddFrame(0.75f, boy5);
    player->AddFrame(0.9f, boy6);
    player->mirrored = false; // Player is not mirrored

    enemy = engine->CreatePlane(10.0f, 10.0f, 64.0f, 64.0f, dinoTex); // Reusing dino texture for simplicity
    return true;
}

bool cGame::UpdateInput(float deltaTime)
{
    // Poll Keyboard
    if (input->GetKey(engine->window, GLFW_KEY_W) == PRESSED) {
        player->y -= deltaTime * 100.0f; // Move up
    }
    if (input->GetKey(engine->window, GLFW_KEY_S) == PRESSED) {
        player->y += deltaTime * 100.0f; // Move down
    }
    if (input->GetKey(engine->window, GLFW_KEY_D) == PRESSED) {
        player->x += deltaTime * 100.0f; // Move right
        player->mirrored = false;
    }
    if (input->GetKey(engine->window, GLFW_KEY_A) == PRESSED) {
        player->x -= deltaTime * 100.0f; // Move left
        player->mirrored = true;
        sound->PlaySound(SOUND_END); // Play sound on pressing 'A'
    }

    if (input->GetKey(engine->window, GLFW_KEY_SPACE) == PRESSED) {
       player->SetAnimating(!player->GetAnimating()); // Toggle animation on spacebar
    }

    //Exit?
    if (input->GetKey(engine->window, GLFW_KEY_ESCAPE) == PRESSED) {
        return true; // Signal to quit
    }

    // Poll Gamepad
    ControllerState pad = input->GetGamepad(GLFW_JOYSTICK_1);
    if (pad.connected) 
    {
        if (pad.leftStickX < 0.1f && pad.leftStickX > -0.1f) 
        {
            pad.leftStickX = 0.0f; // Deadzone
        } else
        {
            player->x += pad.leftStickX * deltaTime * 100.0f; // Adjust speed as needed
            player->mirrored = (pad.leftStickX < 0.0f); // Mirror if moving left
        }

        if (pad.leftStickY < 0.08f && pad.leftStickY > -0.08f) 
        {
            pad.leftStickY = 0.0f; // Deadzone
        } else
        {
            player->y += pad.leftStickY * deltaTime * 100.0f; // Y is usually inverted on sticks
        }
    }

    return false;
}

bool cGame::Run()
{
    bool quit = false;
    while (!glfwWindowShouldClose(engine->window) && !quit) 
    {
        float dTime = engine->UpdateStart();
        engine->UpdateAllPlanes(dTime);
        engine->RenderAllPlanes();
        engine->UpdateEnd();

        quit = UpdateInput(dTime);

        if (player->Intersects((Plane&)*enemy)) {
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