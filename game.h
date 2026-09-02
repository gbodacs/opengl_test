#include "engine/2d/engine.h"
#include "engine/input/input.h"
#include "engine/2d/plane.h"

class cGame
{
public:
    Engine* engine;

    Plane* player;
    Plane* enemy;
    
    bool Init();
    bool Run();
    bool Done();
};