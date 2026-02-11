#pragma once
#include <GLFW/glfw3.h>

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

ControllerState PollGamepad(int jid = GLFW_JOYSTICK_1) {
    ControllerState state = { false, 0.0f, 0.0f, false };
    GLFWgamepadstate gpState;

    if (glfwJoystickIsGamepad(jid) && glfwGetGamepadState(jid, &gpState)) {
        state.connected = true;
        // Axes are usually -1.0 to 1.0
        state.leftStickX = gpState.axes[GLFW_GAMEPAD_AXIS_LEFT_X];
        state.leftStickY = gpState.axes[GLFW_GAMEPAD_AXIS_LEFT_Y];
        
        // Buttons are GLFW_PRESS or GLFW_RELEASE
        state.buttonA = gpState.buttons[GLFW_GAMEPAD_BUTTON_A] == GLFW_PRESS;
    }
    return state;
}