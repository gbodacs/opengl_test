#include "engine/2d/engine.h"
#include "engine/input/input.h"
#include "engine/2d/plane.h"
#include "engine/input/input.h"
#include "engine/sound/sound.h"


class cGame
{
protected:
    bool UpdateInput(float deltaTime);

public:
    Engine* engine;
    Input* input;
    Sound* sound;

    AnimatedGameObject* player;
    Plane* enemy;
    
    bool Init();
    bool Run();
    bool Done();
};