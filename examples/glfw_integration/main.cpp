// MIT License
// Copyright (c) 2025 Ruben Leal Mirete
//
// GLFW Integration Example - campello_input
//
// This example demonstrates integrating campello_input with GLFW for windowing.
// Shows best practices for game input handling.
//
// Prerequisites:
//   - GLFW3 development libraries
//
// Build:
//   mkdir build && cd build
//   cmake .. -DCMAKE_BUILD_TYPE=Release
//   cmake --build .

#include <campello_input/input_system.hpp>
#include <campello_input/gamepad.hpp>
#include <campello_input/keyboard.hpp>
#include <campello_input/mouse.hpp>
#include <campello_input/haptics.hpp>

#include <GLFW/glfw3.h>

#include <iostream>
#include <cmath>

using namespace systems::leal::campello_input;

// Game state
struct GameState {
    // Player position
    float playerX = 400.0f;
    float playerY = 300.0f;
    float playerSpeed = 5.0f;
    
    // Input state
    bool usingGamepad = false;
    float aimX = 0.0f;
    float aimY = 0.0f;
    
    // Game state
    bool firePressed = false;
    bool jumpPressed = false;
    bool paused = false;
};

// Haptic feedback helper
void triggerHapticPulse(GamepadDevice* gamepad, float intensity, uint32_t durationMs) {
    if (!gamepad) return;
    
    auto* haptics = gamepad->haptics();
    if (haptics && haptics->supports(HapticsCapability::rumble)) {
        haptics->playRumble(intensity, intensity, durationMs);
    }
}

// Process gamepad input
void processGamepadInput(InputSystem* input, GameState& state) {
    if (input->gamepadCount() == 0) {
        state.usingGamepad = false;
        return;
    }
    
    state.usingGamepad = true;
    
    // Use first gamepad
    auto* gamepad = input->gamepadAt(0);
    if (!gamepad) return;
    
    GamepadState gpState;
    if (!gamepad->getState(gpState)) return;
    
    // Movement - Left stick
    float moveX = gpState.axes[(int)GamepadAxis::left_x].value;
    float moveY = gpState.axes[(int)GamepadAxis::left_y].value;
    
    // Apply deadzone
    const float deadzone = 0.15f;
    if (std::abs(moveX) < deadzone) moveX = 0.0f;
    if (std::abs(moveY) < deadzone) moveY = 0.0f;
    
    // Update player position
    state.playerX += moveX * state.playerSpeed;
    state.playerY += moveY * state.playerSpeed;
    
    // Aim - Right stick
    float aimX = gpState.axes[(int)GamepadAxis::right_x].value;
    float aimY = gpState.axes[(int)GamepadAxis::right_y].value;
    
    if (std::abs(aimX) > deadzone || std::abs(aimY) > deadzone) {
        state.aimX = aimX;
        state.aimY = aimY;
    }
    
    // Fire button (A/X on most controllers)
    bool fireButton = gpState.buttons & (uint64_t)GamepadButton::face_south;
    if (fireButton && !state.firePressed) {
        // Button just pressed - trigger haptics
        triggerHapticPulse(gamepad, 0.5f, 100);
    }
    state.firePressed = fireButton;
    
    // Jump button (B/O on most controllers)
    bool jumpButton = gpState.buttons & (uint64_t)GamepadButton::face_east;
    if (jumpButton && !state.jumpPressed) {
        triggerHapticPulse(gamepad, 0.3f, 50);
    }
    state.jumpPressed = jumpButton;
    
    // Pause (Start button)
    if (gpState.buttons & (uint64_t)GamepadButton::start) {
        state.paused = !state.paused;
    }
}

// Process keyboard input
void processKeyboardInput(InputSystem* input, GameState& state, GLFWwindow* window) {
    if (!input->hasKeyboard()) return;
    
    auto* keyboard = input->keyboard();
    
    // Movement (WASD)
    float moveX = 0.0f;
    float moveY = 0.0f;
    
    if (keyboard->isKeyDown(KeyCode::w)) moveY -= 1.0f;
    if (keyboard->isKeyDown(KeyCode::s)) moveY += 1.0f;
    if (keyboard->isKeyDown(KeyCode::a)) moveX -= 1.0f;
    if (keyboard->isKeyDown(KeyCode::d)) moveX += 1.0f;
    
    // Normalize diagonal movement
    if (moveX != 0.0f && moveY != 0.0f) {
        moveX *= 0.707f;  // 1/sqrt(2)
        moveY *= 0.707f;
    }
    
    state.playerX += moveX * state.playerSpeed;
    state.playerY += moveY * state.playerSpeed;
    
    // Fire (Space)
    bool fireButton = keyboard->isKeyDown(KeyCode::space);
    if (fireButton && !state.firePressed) {
        // Just pressed
        if (input->gamepadCount() > 0) {
            triggerHapticPulse(input->gamepadAt(0), 0.5f, 100);
        }
    }
    state.firePressed = fireButton;
    
    // Jump (Shift)
    state.jumpPressed = keyboard->isKeyDown(KeyCode::shift_left) || 
                        keyboard->isKeyDown(KeyCode::shift_right);
    
    // Pause (Escape)
    if (keyboard->isKeyDown(KeyCode::escape)) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
}

// Process mouse input
void processMouseInput(InputSystem* input, GameState& state, GLFWwindow* window) {
    if (!input->hasMouse()) return;
    
    auto* mouse = input->mouse();
    MouseState mouseState;
    mouse->getState(mouseState);
    
    // Aim towards mouse position
    int windowWidth, windowHeight;
    glfwGetWindowSize(window, &windowWidth, &windowHeight);
    
    float centerX = windowWidth / 2.0f;
    float centerY = windowHeight / 2.0f;
    
    state.aimX = (mouseState.pos_x - centerX) / centerX;
    state.aimY = (mouseState.pos_y - centerY) / centerY;
    
    // Fire on left click
    bool fireButton = mouseState.buttons & (uint32_t)MouseButton::left;
    if (fireButton && !state.firePressed) {
        if (input->gamepadCount() > 0) {
            triggerHapticPulse(input->gamepadAt(0), 0.5f, 100);
        }
    }
    state.firePressed = fireButton;
    
    // Jump on right click
    state.jumpPressed = mouseState.buttons & (uint32_t)MouseButton::right;
}

// GLFW error callback
void glfwErrorCallback(int error, const char* description) {
    std::cerr << "GLFW Error " << error << ": " << description << std::endl;
}

// GLFW key callback (for window close)
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    (void)scancode;
    (void)mods;
    
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
}

// Render a simple frame
void renderFrame(GLFWwindow* window, const GameState& state) {
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    
    // Simple OpenGL setup
    glViewport(0, 0, width, height);
    glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    
    // Orthographic projection
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, width, height, 0, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    // Draw player (green circle)
    glColor3f(0.2f, 0.8f, 0.2f);
    glBegin(GL_TRIANGLE_FAN);
    const float radius = 20.0f;
    const int segments = 32;
    for (int i = 0; i <= segments; i++) {
        float angle = 2.0f * 3.14159f * i / segments;
        glVertex2f(state.playerX + std::cos(angle) * radius, 
                   state.playerY + std::sin(angle) * radius);
    }
    glEnd();
    
    // Draw aim direction (red line)
    if (std::abs(state.aimX) > 0.1f || std::abs(state.aimY) > 0.1f) {
        glColor3f(0.9f, 0.2f, 0.2f);
        glLineWidth(3.0f);
        glBegin(GL_LINES);
        glVertex2f(state.playerX, state.playerY);
        glVertex2f(state.playerX + state.aimX * 50.0f, 
                   state.playerY + state.aimY * 50.0f);
        glEnd();
    }
    
    // Draw fire effect (yellow burst)
    if (state.firePressed) {
        glColor3f(1.0f, 0.9f, 0.2f);
        glBegin(GL_TRIANGLE_FAN);
        float fireRadius = radius * 1.5f;
        for (int i = 0; i <= segments; i++) {
            float angle = 2.0f * 3.14159f * i / segments;
            glVertex2f(state.playerX + state.aimX * 40.0f + std::cos(angle) * fireRadius * 0.3f, 
                       state.playerY + state.aimY * 40.0f + std::sin(angle) * fireRadius * 0.3f);
        }
        glEnd();
    }
}

// Draw UI overlay
void drawUI(GLFWwindow* window, const GameState& state, InputSystem* input) {
    int width, height;
    glfwGetWindowSize(window, &width, &height);
    
    // Note: In a real game, you'd use a proper text rendering library
    // This is just for demonstration purposes
    
    std::cout << "\033[2J\033[H";  // Clear terminal
    std::cout << "========================================" << std::endl;
    std::cout << "  campello_input - GLFW Example" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << std::endl;
    std::cout << "Controls:" << std::endl;
    if (state.usingGamepad) {
        std::cout << "  Gamepad: Left Stick = Move, Right Stick = Aim" << std::endl;
        std::cout << "           A/Cross = Fire, B/Circle = Jump" << std::endl;
        std::cout << "           Start = Pause, Back/Select = Menu" << std::endl;
    } else {
        std::cout << "  Keyboard: WASD = Move, SPACE = Fire, SHIFT = Jump" << std::endl;
        std::cout << "  Mouse: Move = Aim, Left Click = Fire, Right Click = Jump" << std::endl;
    }
    std::cout << std::endl;
    std::cout << "State:" << std::endl;
    std::cout << "  Position: (" << state.playerX << ", " << state.playerY << ")" << std::endl;
    std::cout << "  Aim: (" << state.aimX << ", " << state.aimY << ")" << std::endl;
    std::cout << "  Input: " << (state.usingGamepad ? "Gamepad" : "Keyboard/Mouse") << std::endl;
    std::cout << std::endl;
    std::cout << "Devices:" << std::endl;
    std::cout << "  Gamepads: " << input->gamepadCount() << std::endl;
    for (int i = 0; i < input->gamepadCount(); i++) {
        auto* gp = input->gamepadAt(i);
        if (gp) {
            std::cout << "    [" << i << "] " << gp->name();
            auto* haptics = gp->haptics();
            if (haptics && haptics->supports(HapticsCapability::rumble)) {
                std::cout << " (haptics)";
            }
            std::cout << std::endl;
        }
    }
    std::cout << "  Keyboard: " << (input->hasKeyboard() ? "Yes" : "No") << std::endl;
    std::cout << "  Mouse: " << (input->hasMouse() ? "Yes" : "No") << std::endl;
    std::cout << std::endl;
    std::cout << "Press ESC to exit" << std::endl;
}

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;
    
    // Initialize GLFW
    glfwSetErrorCallback(glfwErrorCallback);
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return 1;
    }
    
    // Create window
    GLFWwindow* window = glfwCreateWindow(800, 600, "campello_input - GLFW Example", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return 1;
    }
    
    glfwMakeContextCurrent(window);
    glfwSetKeyCallback(window, keyCallback);
    glfwSwapInterval(1);  // Enable vsync
    
    // Initialize input system
    auto input = InputSystem::create();
    if (!input) {
        std::cerr << "Failed to create InputSystem" << std::endl;
        glfwDestroyWindow(window);
        glfwTerminate();
        return 1;
    }
    
    GameState state;
    
    std::cout << "Starting GLFW integration example..." << std::endl;
    
    // Main loop
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        
        // Update input
        input->update();
        
        // Process input based on priority: gamepad > keyboard/mouse
        if (input->gamepadCount() > 0) {
            processGamepadInput(input.get(), state);
        } else {
            processKeyboardInput(input.get(), state, window);
            processMouseInput(input.get(), state, window);
        }
        
        // Clamp player to screen
        int width, height;
        glfwGetWindowSize(window, &width, &height);
        state.playerX = std::max(20.0f, std::min(state.playerX, (float)width - 20.0f));
        state.playerY = std::max(20.0f, std::min(state.playerY, (float)height - 20.0f));
        
        // Render
        renderFrame(window, state);
        glfwSwapBuffers(window);
        
        // Draw UI (to terminal since we don't have a text renderer)
        drawUI(window, state, input.get());
    }
    
    // Cleanup
    glfwDestroyWindow(window);
    glfwTerminate();
    
    return 0;
}
