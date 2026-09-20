#include "game.h"
#include "player.h"

ma_sound startSound;

bool cGame::Init()
{
#if defined(__ANDROID__)
    // On Android, android_main.cpp already constructed `engine` and called
    // Engine::InitAndroid() before invoking Init(), since the EGL surface
    // requires a live ANativeWindow that only exists once the app is resumed.
    if (engine == nullptr)
        return false;
#else
    engine = new Engine();
    if (!engine->Init(1024, 768, "My 2D Engine")) 
        return false;
#endif
    engine->cameraDistance = 6.0f;

    input = new Input();
    input->Init();

    sound = new Sound();
    if (!sound->Init())
        return false;

    if (!sound->LoadSounds())
        return false;

    sound->PlaySound(SOUND_START);

    menu = new Menu({
    { "Start Game", {}, [](){ /* ... */ }, 0, true, true },
    { "Options", { { "Sound", {}, [](){}, 0, true, false }, { "Back", {}, nullptr } } },
    { "Exit", {}, [this](){ engine->RequestClose(); }, 0, true, true }
});

    uiFont.LoadMetadata("data/graph/font/font.fnt");
    uiFont.SetTexture(engine->LoadTexture("data/graph/font/font.png"));

    GLuint dinoTex = engine->LoadTexture("data/graph/dino.png");
    GLuint boy1 = engine->LoadTexture("data/graph/boy1.png");
    GLuint boy2 = engine->LoadTexture("data/graph/boy2.png");
    GLuint boy3 = engine->LoadTexture("data/graph/boy3.png");
    GLuint boy4 = engine->LoadTexture("data/graph/boy4.png");
    GLuint boy5 = engine->LoadTexture("data/graph/boy5.png");
    GLuint boy6 = engine->LoadTexture("data/graph/boy6.png");
    GLuint boyIdle = engine->LoadTexture("data/graph/boy_idle.png");

    demoMesh = engine->LoadMesh("data/models/test_cube.obj");
    if (demoMesh == nullptr)
        return false;

    demoMesh->rotationX = -20.0f;
    demoMesh->rotationY = 30.0f;
    demoMesh->scaleX = 1.25f;
    demoMesh->scaleY = 1.25f;
    demoMesh->scaleZ = 1.25f;

    std::string animationError;
    demoAnimatedMesh = engine->LoadAnimatedMesh("data/models/animated_character.glb", &animationError);
    if (demoAnimatedMesh != nullptr)
    {
        demoAnimatedMesh->x = 2.0f;
        demoAnimatedMesh->PlayClip("Idle", true);
    }
    else
    {
        std::cerr << "Animated character not loaded: " << animationError << std::endl;
    }

    player = new Player(40.0f, 40.0f, 131.0f, 251.0f);
    engine->AddPlane(player->GetPlane());

    const int animID = player->AddAnimation();
    //__assert(animID == PlayerAnims::Idle, "Animation ID should be Idle");
    player->AddFrame(PlayerAnims::Idle, 0.15f, boyIdle);

    const int animID2 = player->AddAnimation();
    //__assert(animID2 == PlayerAnims::Jump, "Animation ID should be Jump");
    player->AddFrame(PlayerAnims::Jump, 0.15f, boy1);
    player->AddFrame(PlayerAnims::Jump, 0.3f, boy2);
    player->AddFrame(PlayerAnims::Jump, 0.45f, boy3);
    player->AddFrame(PlayerAnims::Jump, 0.6f, boy4);
    player->AddFrame(PlayerAnims::Jump, 0.75f, boy5);
    player->AddFrame(PlayerAnims::Jump, 0.9f, boy6);
    player->SetCurrentAnimation(PlayerAnims::Jump);

    player->GetPlane()->mirrored = false; // Player is not mirrored

    enemy = new Enemy(10.0f, 10.0f, 64.0f, 64.0f, dinoTex); // Reusing dino texture for simplicity
    enemy->SetCurrentAnimation(0);
    engine->AddPlane(enemy->GetPlane());
    return true;
}

bool cGame::UpdateInput(float deltaTime)
{
    const InputSnapshot& state = input->GetState();

    // Poll Keyboard
    if (state.w.down)
    {
        player->GetPlane()->y -= deltaTime * 100.0f; // Move up
        playerPrevUp = true;
    }

    if (state.s.down)
    {
        player->GetPlane()->y += deltaTime * 100.0f; // Move down
        playerPrevDown = true;
    }

    if (state.d.down)
    {
        player->GetPlane()->x += deltaTime * 100.0f; // Move right
        player->GetPlane()->mirrored = false;
        playerPrevRight = true;
    }

    if (state.a.down)
    {
        player->GetPlane()->x -= deltaTime * 100.0f; // Move left
        player->GetPlane()->mirrored = true;
        playerPrevLeft = true;
        sound->PlaySound(SOUND_END); // Play sound on pressing 'A'
    }

    if (state.space.down)
    {
        playerPrevJump = true;
    }
    else
    {
        if (playerPrevJump)
        {
            if (player->GetCurrentAnimation() != PlayerAnims::Idle)
                player->SetCurrentAnimation(PlayerAnims::Idle); // Toggle animation on spacebar
            else
                player->SetCurrentAnimation(PlayerAnims::Jump); // Toggle back to jump if not idle
        }
        playerPrevJump = false;
    }

    //Exit?
    if (state.esc.down)
    {
        return true; // Signal to quit
    }

    // Poll Gamepad
    /*ControllerState pad = input->GetGamepad(GLFW_JOYSTICK_1);
    if (pad.connected) 
    {
        if (pad.leftStickX < 0.1f && pad.leftStickX > -0.1f) 
        {
            pad.leftStickX = 0.0f; // Deadzone
        } else
        {
            if (pad.leftStickX > 0.0f) 
            {
                playerPrevRight = true; // Facing right
            } 
            else if (pad.leftStickX < 0.0f) 
            {
                playerPrevLeft = true; // Facing left
            }
            player->GetPlane()->x += pad.leftStickX * deltaTime * 100.0f; // Adjust speed as needed
            player->GetPlane()->mirrored = (pad.leftStickX < 0.0f); // Mirror if moving left
        }

        if (pad.leftStickY < 0.08f && pad.leftStickY > -0.08f) 
        {
            pad.leftStickY = 0.0f; // Deadzone
        } else
        {
            if (pad.leftStickY < 0.0f) 
            {
                playerPrevUp = true; // Facing up
            } 
            else if (pad.leftStickY > 0.0f) 
            {
                playerPrevDown = true; // Facing down
            }
            player->GetPlane()->y += pad.leftStickY * deltaTime * 100.0f; // Y is usually inverted on sticks
        }
    }*/

    return false;
}

bool cGame::Run()
{
    while (!engine->ShouldClose()) 
    {
        Step();
    }
    return true;
}

void cGame::Step()
{
    float dTime = engine->UpdateStart();
    engine->UpdateMeshes(dTime);
#if defined(__ANDROID__)
    input->Update();
#else
    input->Update(engine->window);
#endif
    if (gameState == GameStates::Menu)                  // MENU
    {
        menu->Update(dTime);
        bool closeMenu = menu->HandleInput(input->GetState());
        menu->Render(*engine, uiFont, 50.0f, 50.0f);
    } else if (gameState == GameStates::Playing)        // GAME
    {
        player->Update(dTime);
        enemy->Update(dTime);
        demoMeshSpin += dTime * 45.0f;
        demoMesh->rotationY = demoMeshSpin;

        engine->RenderAllMeshes();
        engine->RenderAllPlanes();

        if (player->GetPlane()->Intersects((Plane&)*enemy->GetPlane())) 
        {
            engine->PrintText(uiFont, 0.0f, 0.0f, "Collision", 1.0f, 1.0f, 1.0f, 1.0f);
        }
    }

    engine->UpdateEnd();

    if (UpdateInput(dTime))
    {
        engine->RequestClose();
    }
}

bool cGame::Done()
{   
    sound->Done();
    delete sound;

    delete input;

    demoMesh = nullptr;
    demoAnimatedMesh = nullptr;

    engine->Done();
    delete engine;

    return true;
}

#if !defined(__ANDROID__)
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
#endif