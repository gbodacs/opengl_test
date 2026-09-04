#pragma once
#include <GLFW/glfw3.h>
#include <cstdio>
#include <iterator>
#include <fstream>
#include <string>

// Key state enumeration
enum KeyState { IDLE, PRESSED };

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

public:
    void Init()
    {
        // Load updated mappings (fixes glfwJoystickIsGamepad returning false)
        std::string mappings = load_file("data/input/gamecontrollerdb.txt");
        glfwUpdateGamepadMappings(mappings.c_str());
    }

    static KeyState GetKey(GLFWwindow* window, int key) 
    {
        if (glfwGetKey(window, key) == GLFW_PRESS) 
        {
            return PRESSED;
        }
        return IDLE;
    }

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
