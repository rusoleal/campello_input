// MIT License
// Copyright (c) 2025 Ruben Leal Mirete
//
// Minimal Example - campello_input
//
// This example demonstrates basic usage of the campello_input library.
// It works on all supported platforms (macOS, iOS, Android, Windows, Linux).
//
// Build:
//   mkdir build && cd build
//   cmake .. -DCMAKE_BUILD_TYPE=Release
//   cmake --build .
//
// Run:
//   ./minimal_example

#include <campello_input/input_system.hpp>
#include <campello_input/gamepad.hpp>
#include <campello_input/keyboard.hpp>
#include <campello_input/mouse.hpp>
#include <iostream>
#include <thread>
#include <chrono>

using namespace systems::leal::campello_input;

// Simple observer to get notified when devices connect/disconnect
class ExampleObserver : public DeviceObserver {
public:
    void onDeviceConnected(const DeviceInfo& info) override {
        std::cout << "[CONNECTED] " << deviceTypeToString(info.type) 
                  << " - " << info.name << " (ID: " << info.id << ")" << std::endl;
    }
    
    void onDeviceDisconnected(uint32_t deviceId) override {
        std::cout << "[DISCONNECTED] Device ID: " << deviceId << std::endl;
    }
    
private:
    const char* deviceTypeToString(DeviceType type) {
        switch (type) {
            case DeviceType::gamepad: return "Gamepad";
            case DeviceType::keyboard: return "Keyboard";
            case DeviceType::mouse: return "Mouse";
            case DeviceType::touch: return "Touch";
            default: return "Unknown";
        }
    }
};

// Print gamepad state
void printGamepadState(GamepadDevice* gamepad) {
    GamepadState state;
    if (!gamepad->getState(state)) return;
    
    // Left stick
    std::cout << "  Left Stick: (" 
              << state.axes[(int)GamepadAxis::left_x].value << ", "
              << state.axes[(int)GamepadAxis::left_y].value << ")";
    
    // Right stick
    std::cout << "  Right Stick: (" 
              << state.axes[(int)GamepadAxis::right_x].value << ", "
              << state.axes[(int)GamepadAxis::right_y].value << ")";
    
    // Triggers
    std::cout << "  Triggers: L=" 
              << state.axes[(int)GamepadAxis::trigger_left].value
              << " R=" 
              << state.axes[(int)GamepadAxis::trigger_right].value;
    
    // Buttons (check a few common ones)
    std::cout << "  Buttons: ";
    if (state.buttons & (uint64_t)GamepadButton::face_south) std::cout << "A ";
    if (state.buttons & (uint64_t)GamepadButton::face_east) std::cout << "B ";
    if (state.buttons & (uint64_t)GamepadButton::face_west) std::cout << "X ";
    if (state.buttons & (uint64_t)GamepadButton::face_north) std::cout << "Y ";
    if (state.buttons & (uint64_t)GamepadButton::shoulder_left) std::cout << "LB ";
    if (state.buttons & (uint64_t)GamepadButton::shoulder_right) std::cout << "RB ";
    if (state.buttons & (uint64_t)GamepadButton::start) std::cout << "START ";
    if (state.buttons & (uint64_t)GamepadButton::select) std::cout << "SELECT ";
    
    std::cout << std::endl;
}

// Print keyboard state
void printKeyboardState(KeyboardDevice* keyboard) {
    if (!keyboard) return;
    
    std::cout << "  Keyboard: ";
    
    // Check for some common keys
    if (keyboard->isKeyDown(KeyCode::w)) std::cout << "W ";
    if (keyboard->isKeyDown(KeyCode::a)) std::cout << "A ";
    if (keyboard->isKeyDown(KeyCode::s)) std::cout << "S ";
    if (keyboard->isKeyDown(KeyCode::d)) std::cout << "D ";
    if (keyboard->isKeyDown(KeyCode::space)) std::cout << "SPACE ";
    if (keyboard->isKeyDown(KeyCode::enter)) std::cout << "ENTER ";
    if (keyboard->isKeyDown(KeyCode::escape)) std::cout << "ESC ";
    
    // Modifiers
    auto mods = keyboard->modifiers();
    if (mods != KeyModifier::none) {
        std::cout << "[Mod: ";
        if (mods & KeyModifier::shift) std::cout << "SHIFT ";
        if (mods & KeyModifier::ctrl) std::cout << "CTRL ";
        if (mods & KeyModifier::alt) std::cout << "ALT ";
        std::cout << "]";
    }
    
    std::cout << std::endl;
}

// Print mouse state
void printMouseState(MouseDevice* mouse) {
    if (!mouse) return;
    
    MouseState state;
    mouse->getState(state);
    
    std::cout << "  Mouse: Pos=(" << state.pos_x << "," << state.pos_y << ")"
              << " Delta=(" << state.delta_x << "," << state.delta_y << ")"
              << " Scroll=(" << state.scroll_delta_x << "," << state.scroll_delta_y << ")"
              << " Buttons=";
    
    if (state.buttons & (uint32_t)MouseButton::left) std::cout << "L ";
    if (state.buttons & (uint32_t)MouseButton::right) std::cout << "R ";
    if (state.buttons & (uint32_t)MouseButton::middle) std::cout << "M ";
    
    std::cout << std::endl;
}

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;
    
    std::cout << "========================================" << std::endl;
    std::cout << "  campello_input - Minimal Example" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << std::endl;
    std::cout << "Press ESC on keyboard or Ctrl+C to exit" << std::endl;
    std::cout << std::endl;
    
    // Create input system
    // For Android, you'd pass PlatformContext{.native = android_app_ptr}
    // For other platforms, default context works fine
    PlatformContext ctx{};
    auto input = InputSystem::create(ctx);
    
    if (!input) {
        std::cerr << "Failed to create InputSystem - platform not supported" << std::endl;
        return 1;
    }
    
    // Add observer for device connect/disconnect notifications
    ExampleObserver observer;
    input->addObserver(&observer);
    
    std::cout << "Input system initialized." << std::endl;
    std::cout << std::endl;
    
    // Main loop
    bool running = true;
    int frameCount = 0;
    
    while (running) {
        // Update input system - call once per frame
        input->update();
        
        // Clear screen (simple terminal clear)
        if (frameCount % 60 == 0) {
            std::cout << "\033[2J\033[H";  // ANSI clear screen
            std::cout << "========================================" << std::endl;
            std::cout << "  campello_input - Device States" << std::endl;
            std::cout << "========================================" << std::endl;
        }
        
        // Poll gamepads
        int gamepadCount = input->gamepadCount();
        if (gamepadCount > 0) {
            std::cout << "Gamepads: " << gamepadCount << std::endl;
            for (int i = 0; i < gamepadCount; i++) {
                auto* gamepad = input->gamepadAt(i);
                if (gamepad) {
                    std::cout << "[" << i << "] " << gamepad->name() << std::endl;
                    printGamepadState(gamepad);
                }
            }
        }
        
        // Poll keyboard
        if (input->hasKeyboard()) {
            printKeyboardState(input->keyboard());
            
            // Check for ESC to exit
            if (input->keyboard()->isKeyDown(KeyCode::escape)) {
                running = false;
            }
        }
        
        // Poll mouse
        if (input->hasMouse()) {
            printMouseState(input->mouse());
        }
        
        // Show device summary every frame
        std::cout << "Devices: " 
                  << input->gamepadCount() << " gamepad(s), "
                  << (input->hasKeyboard() ? "1" : "0") << " keyboard, "
                  << (input->hasMouse() ? "1" : "0") << " mouse"
                  << std::endl;
        
        frameCount++;
        
        // Sleep to avoid burning CPU (16ms = ~60fps)
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }
    
    std::cout << std::endl;
    std::cout << "Exiting..." << std::endl;
    
    // Remove observer before destruction
    input->removeObserver(&observer);
    
    return 0;
}
