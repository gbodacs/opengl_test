#pragma once
#include <GLFW/glfw3.h>
#include <cstdio>
#include <iterator>
#include <fstream>
#include <string>

// Per-key edge-detected state (down = currently held, pressed/released = edges this frame)
struct KeyState
{
    bool down = false;
    bool pressed = false;
    bool released = false;
};

// Snapshot of the keys the game cares about (WASD, space, esc)
struct InputSnapshot
{
    KeyState w, a, s, d, space, esc;
};

// Controller input handling
struct ControllerState 
{
    bool connected;
    float leftStickX;
    float leftStickY;
    bool buttonA;
    bool buttonB;
    bool buttonX;
    bool buttonY;
    bool buttonLeftBumper;
    bool buttonRightBumper;
    bool buttonBack;
    bool buttonStart;
    bool buttonGuide;
    bool buttonLeftThumb;
    bool buttonRightThumb;
    bool buttonDpadUp;
    bool buttonDpadRight;
    bool buttonDpadDown;
    bool buttonDpadLeft;
};

// Keyboard input handling
class Input 
{
protected:
    std::string load_file(const std::string& path) 
    {
        std::ifstream f(path);
        return std::string((std::istreambuf_iterator<char>(f)),
                            std::istreambuf_iterator<char>());
    }

    InputSnapshot snapshot;

    static void UpdateKey(GLFWwindow* window, int key, KeyState& state)
    {
        bool isDown = glfwGetKey(window, key) == GLFW_PRESS;
        state.pressed = isDown && !state.down;
        state.released = !isDown && state.down;
        state.down = isDown;
    }

public:
    void Init()
    {
        // Load updated mappings (fixes glfwJoystickIsGamepad returning false)
        std::string mappings = load_file("data/input/gamecontrollerdb.txt");
        glfwUpdateGamepadMappings(mappings.c_str());
    }

    // Polls the keyboard and updates the pressed/down/released edges for this frame.
    void Update(GLFWwindow* window)
    {
        UpdateKey(window, GLFW_KEY_W, snapshot.w);
        UpdateKey(window, GLFW_KEY_A, snapshot.a);
        UpdateKey(window, GLFW_KEY_S, snapshot.s);
        UpdateKey(window, GLFW_KEY_D, snapshot.d);
        UpdateKey(window, GLFW_KEY_SPACE, snapshot.space);
        UpdateKey(window, GLFW_KEY_ESCAPE, snapshot.esc);
    }

    const InputSnapshot& GetState() const { return snapshot; }

    static ControllerState GetGamepad(int jid = GLFW_JOYSTICK_1) 
    {
        ControllerState state = { false, 0.0f, 0.0f, false };
        GLFWgamepadstate gpState;

        if (glfwJoystickIsGamepad(jid))
        {
            if (glfwGetGamepadState(jid, &gpState)) 
            {
                state.connected = true;
                // Axes are usually -1.0 to 1.0
                state.leftStickX = gpState.axes[GLFW_GAMEPAD_AXIS_LEFT_X];
                state.leftStickY = gpState.axes[GLFW_GAMEPAD_AXIS_LEFT_Y];
                
                // Buttons are GLFW_PRESS or GLFW_RELEASE
                state.buttonA = gpState.buttons[GLFW_GAMEPAD_BUTTON_A] == GLFW_PRESS;
                state.buttonB = gpState.buttons[GLFW_GAMEPAD_BUTTON_B] == GLFW_PRESS;
                state.buttonX = gpState.buttons[GLFW_GAMEPAD_BUTTON_X] == GLFW_PRESS;
                state.buttonY = gpState.buttons[GLFW_GAMEPAD_BUTTON_Y] == GLFW_PRESS;
                state.buttonLeftBumper = gpState.buttons[GLFW_GAMEPAD_BUTTON_LEFT_BUMPER] == GLFW_PRESS;
                state.buttonRightBumper = gpState.buttons[GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER] == GLFW_PRESS;
                state.buttonBack = gpState.buttons[GLFW_GAMEPAD_BUTTON_BACK] == GLFW_PRESS;
                state.buttonStart = gpState.buttons[GLFW_GAMEPAD_BUTTON_START] == GLFW_PRESS;
                state.buttonGuide = gpState.buttons[GLFW_GAMEPAD_BUTTON_GUIDE] == GLFW_PRESS;
                state.buttonLeftThumb = gpState.buttons[GLFW_GAMEPAD_BUTTON_LEFT_THUMB] == GLFW_PRESS;
                state.buttonRightThumb = gpState.buttons[GLFW_GAMEPAD_BUTTON_RIGHT_THUMB] == GLFW_PRESS;
                state.buttonDpadUp = gpState.buttons[GLFW_GAMEPAD_BUTTON_DPAD_UP] == GLFW_PRESS;
                state.buttonDpadRight = gpState.buttons[GLFW_GAMEPAD_BUTTON_DPAD_RIGHT] == GLFW_PRESS;
                state.buttonDpadDown = gpState.buttons[GLFW_GAMEPAD_BUTTON_DPAD_DOWN] == GLFW_PRESS;
                state.buttonDpadLeft = gpState.buttons[GLFW_GAMEPAD_BUTTON_DPAD_LEFT] == GLFW_PRESS;
            }
        } else if (glfwJoystickPresent(jid)) 
        {
            // Present but still no mapping — print GUID for debugging
            printf("Unknown controller - no mapping for: %s (%s)\n", glfwGetJoystickName(jid), glfwGetJoystickGUID(jid));
        }
        return state;
    }
};
