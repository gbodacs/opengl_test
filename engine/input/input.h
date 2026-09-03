#pragma once
#include <GLFW/glfw3.h>
#include <cstdio>
#include <iterator>
#include <fstream>
#include <string>

enum KeyState { IDLE, PRESSED };

// Keyboard input handling
class InputHandler {
public:
    static KeyState GetKey(GLFWwindow* window, int key) {
        if (glfwGetKey(window, key) == GLFW_PRESS) {
            return PRESSED;
        }
        return IDLE;
    }
};

// Controller input handling
struct ControllerState {
    bool connected;
    float leftStickX;
    float leftStickY;
    bool buttonA;
};

std::string load_file(const std::string& path) {
    std::ifstream f(path);
    return std::string((std::istreambuf_iterator<char>(f)),
                        std::istreambuf_iterator<char>());
}

    
void InitInput()
{
    // Load updated mappings (fixes glfwJoystickIsGamepad returning false)
    std::string mappings = load_file("data/input/gamecontrollerdb.txt");
    glfwUpdateGamepadMappings(mappings.c_str());
}

ControllerState PollGamepad(int jid = GLFW_JOYSTICK_1) {
    ControllerState state = { false, 0.0f, 0.0f, false };
    GLFWgamepadstate gpState;

    if (glfwJoystickIsGamepad(jid))
    {
        if (glfwGetGamepadState(jid, &gpState)) 
        {
            printf("22Gamepad connected: %s\n", glfwGetGamepadName(jid));
            state.connected = true;
            // Axes are usually -1.0 to 1.0
            state.leftStickX = gpState.axes[GLFW_GAMEPAD_AXIS_LEFT_X];
            state.leftStickY = gpState.axes[GLFW_GAMEPAD_AXIS_LEFT_Y];
            
            // Buttons are GLFW_PRESS or GLFW_RELEASE
            state.buttonA = gpState.buttons[GLFW_GAMEPAD_BUTTON_A] == GLFW_PRESS;
        }
    } else if (glfwJoystickPresent(jid)) 
    {
        // Present but still no mapping — print GUID for debugging
        printf("No mapping for: %s (%s)\n", glfwGetJoystickName(jid), glfwGetJoystickGUID(jid));
    }
    return state;
}