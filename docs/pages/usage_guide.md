# Usage Guide

This guide covers common patterns for using campello_input in your application.

## Table of Contents

- [Basic Setup](#basic-setup)
- [Gamepad Input](#gamepad-input)
- [Keyboard Input](#keyboard-input)
- [Mouse Input](#mouse-input)
- [Touch Input](#touch-input)
- [Haptics](#haptics)
- [Device Management](#device-management)
- [Best Practices](#best-practices)

## Basic Setup {#basic-setup}

### Initialization

Create the InputSystem at application startup:

```cpp
#include <campello_input/input_system.hpp>

using namespace systems::leal::campello_input;

// Platform-specific context (can be empty for most platforms)
PlatformContext ctx{};

// Android: Pass android_app pointer
// PlatformContext ctx{.native = android_app_ptr};

auto input = InputSystem::create(ctx);
if (!input) {
    // Platform not supported
    return -1;
}
```

### Main Loop

Call `update()` once per frame:

```cpp
while (running) {
    // Poll all input devices
    input->update();
    
    // Process input...
    
    render();
}
```

### Cleanup

The InputSystem cleans up automatically when destroyed:

```cpp
// InputSystem destructor handles cleanup
input.reset();  // or let it go out of scope
```

## Gamepad Input {#gamepad-input}

### Checking for Gamepads

```cpp
int count = input->gamepadCount();
if (count > 0) {
    std::cout << "Found " << count << " gamepad(s)" << std::endl;
}
```

### Reading Gamepad State

```cpp
for (int i = 0; i < input->gamepadCount(); i++) {
    GamepadDevice* gamepad = input->gamepadAt(i);
    
    GamepadState state;
    if (gamepad->getState(state)) {
        // Left stick - movement
        float moveX = state.axes[(int)GamepadAxis::left_x].value;
        float moveY = state.axes[(int)GamepadAxis::left_y].value;
        
        // Right stick - camera/aim
        float aimX = state.axes[(int)GamepadAxis::right_x].value;
        float aimY = state.axes[(int)GamepadAxis::right_y].value;
        
        // Triggers
        float leftTrigger = state.axes[(int)GamepadAxis::trigger_left].value;
        float rightTrigger = state.axes[(int)GamepadAxis::trigger_right].value;
        
        // Face buttons
        bool aButton = state.buttons & (uint64_t)GamepadButton::face_south;
        bool bButton = state.buttons & (uint64_t)GamepadButton::face_east;
        bool xButton = state.buttons & (uint64_t)GamepadButton::face_west;
        bool yButton = state.buttons & (uint64_t)GamepadButton::face_north;
        
        // Shoulder buttons
        bool leftBumper = state.buttons & (uint64_t)GamepadButton::shoulder_left;
        bool rightBumper = state.buttons & (uint64_t)GamepadButton::shoulder_right;
        
        // D-pad
        bool dpadUp = state.buttons & (uint64_t)GamepadButton::dpad_up;
        bool dpadDown = state.buttons & (uint64_t)GamepadButton::dpad_down;
        bool dpadLeft = state.buttons & (uint64_t)GamepadButton::dpad_left;
        bool dpadRight = state.buttons & (uint64_t)GamepadButton::dpad_right;
        
        // Menu buttons
        bool start = state.buttons & (uint64_t)GamepadButton::start;
        bool select = state.buttons & (uint64_t)GamepadButton::select;
        bool home = state.buttons & (uint64_t)GamepadButton::home;
    }
}
```

### Applying Deadzones

Gamepad sticks often have small drift. Filter it out:

```cpp
float applyDeadzone(float value, float threshold = 0.15f) {
    if (std::abs(value) < threshold) {
        return 0.0f;
    }
    // Rescale to maintain full range
    return (value - std::copysign(threshold, value)) / (1.0f - threshold);
}

float moveX = applyDeadzone(state.axes[(int)GamepadAxis::left_x].value);
float moveY = applyDeadzone(state.axes[(int)GamepadAxis::left_y].value);
```

## Keyboard Input {#keyboard-input}

### Checking for Keyboard

```cpp
if (input->hasKeyboard()) {
    KeyboardDevice* keyboard = input->keyboard();
    // Use keyboard...
}
```

### Checking Key States

```cpp
KeyboardDevice* kb = input->keyboard();

// Movement
if (kb->isKeyDown(KeyCode::w)) { /* Forward */ }
if (kb->isKeyDown(KeyCode::a)) { /* Left */ }
if (kb->isKeyDown(KeyCode::s)) { /* Backward */ }
if (kb->isKeyDown(KeyCode::d)) { /* Right */ }

// Actions
if (kb->isKeyDown(KeyCode::space)) { /* Jump */ }
if (kb->isKeyDown(KeyCode::enter)) { /* Accept */ }
if (kb->isKeyDown(KeyCode::escape)) { /* Pause/Menu */ }

// Modifiers
KeyModifier mods = kb->modifiers();
if (mods & KeyModifier::shift) { /* Shift is held */ }
if (mods & KeyModifier::ctrl) { /* Ctrl is held */ }
if (mods & KeyModifier::alt) { /* Alt is held */ }
```

### Getting Full State

```cpp
KeyboardState state;
kb->getState(state);

// Check any key from the bitmap
uint32_t keyIndex = static_cast<uint32_t>(KeyCode::space);
bool spacePressed = (state.keysDown[keyIndex >> 6] >> (keyIndex & 63)) & 1;
```

## Mouse Input {#mouse-input}

### Checking for Mouse

```cpp
if (input->hasMouse()) {
    MouseDevice* mouse = input->mouse();
    // Use mouse...
}
```

### Reading Mouse State

```cpp
MouseDevice* mouse = input->mouse();

MouseState state;
mouse->getState(state);

// Position (screen coordinates)
int x = state.pos_x;
int y = state.pos_y;

// Movement since last frame
int deltaX = state.delta_x;
int deltaY = state.delta_y;

// Scroll wheel
float scrollX = state.scroll_delta_x;
float scrollY = state.scroll_delta_y;

// Buttons
bool leftClick = state.buttons & (uint32_t)MouseButton::left;
bool rightClick = state.buttons & (uint32_t)MouseButton::right;
bool middleClick = state.buttons & (uint32_t)MouseButton::middle;
```

### Cursor Locking (FPS-style games)

```cpp
// Lock cursor to window center
mouse->setCursorLocked(true);

// In your update loop, use delta for camera rotation
cameraYaw += state.delta_x * sensitivity;
cameraPitch += state.delta_y * sensitivity;
```

## Touch Input {#touch-input}

Touch is primarily for mobile platforms (iOS, Android).

### Checking for Touch

```cpp
if (input->hasTouch()) {
    TouchDevice* touch = input->touch();
    // Use touch...
}
```

### Reading Touch State

```cpp
TouchDevice* touch = input->touch();

TouchState state;
touch->getState(state);

std::cout << "Active touches: " << state.count << std::endl;

for (uint32_t i = 0; i < state.count; i++) {
    const TouchPoint& point = state.points[i];
    
    std::cout << "Touch " << point.id 
              << " at (" << point.x << ", " << point.y << ")"
              << " phase: ";
    
    switch (point.phase) {
        case TouchPhase::began: std::cout << "began"; break;
        case TouchPhase::moved: std::cout << "moved"; break;
        case TouchPhase::ended: std::cout << "ended"; break;
        case TouchPhase::cancelled: std::cout << "cancelled"; break;
    }
    
    std::cout << " pressure: " << point.pressure << std::endl;
}
```

### Virtual Joystick (Touch)

```cpp
// Simple virtual joystick for touch
struct VirtualJoystick {
    float centerX, centerY;
    float currentX, currentY;
    uint32_t touchId = UINT32_MAX;
    bool active = false;
};

VirtualJoystick vjoystick;

void updateVirtualJoystick(const TouchState& touch) {
    if (!vjoystick.active) {
        // Look for new touch in joystick area
        for (uint32_t i = 0; i < touch.count; i++) {
            if (touch.points[i].phase == TouchPhase::began) {
                float dx = touch.points[i].x - vjoystick.centerX;
                float dy = touch.points[i].y - vjoystick.centerY;
                if (std::sqrt(dx*dx + dy*dy) < 100.0f) {  // Within radius
                    vjoystick.active = true;
                    vjoystick.touchId = touch.points[i].id;
                }
            }
        }
    } else {
        // Track existing touch
        bool found = false;
        for (uint32_t i = 0; i < touch.count; i++) {
            if (touch.points[i].id == vjoystick.touchId) {
                found = true;
                vjoystick.currentX = touch.points[i].x;
                vjoystick.currentY = touch.points[i].y;
                
                if (touch.points[i].phase == TouchPhase::ended ||
                    touch.points[i].phase == TouchPhase::cancelled) {
                    vjoystick.active = false;
                }
                break;
            }
        }
        if (!found) vjoystick.active = false;
    }
}
```

## Haptics {#haptics}

Haptics provides force feedback (rumble) on supported controllers.

### Checking Haptics Support

```cpp
GamepadDevice* gamepad = input->gamepadAt(0);
if (gamepad) {
    Haptics* haptics = gamepad->haptics();
    if (haptics) {
        if (haptics->supports(HapticsCapability::rumble)) {
            // Basic rumble available
        }
        if (haptics->supports(HapticsCapability::trigger_left)) {
            // Left trigger feedback available
        }
    }
}
```

### Playing Rumble

```cpp
// Simple rumble
haptics->playRumble(0.5f, 0.5f, 200);  // 50% intensity, 200ms

// Different intensities for left/right motors
haptics->playRumble(1.0f, 0.3f, 500);  // Strong low-freq, weak high-freq

// Stop rumble early
haptics->stopRumble();
```

### Trigger Effects (Xbox Series X|S)

```cpp
// Vibration in left trigger
haptics->playTriggerEffect(
    TriggerEffect::vibration,
    TriggerIntensity::medium,
    100  // ms
);

// Stop trigger effect
haptics->stopTriggerEffect(true);   // Left trigger
haptics->stopTriggerEffect(false);  // Right trigger
```

## Device Management {#device-management}

### Handling Device Connection/Disconnection

```cpp
class MyGame : public DeviceObserver {
public:
    void onDeviceConnected(const DeviceInfo& info) override {
        std::cout << "Device connected: " << info.name << std::endl;
        
        switch (info.type) {
            case DeviceType::gamepad:
                // Assign to player 1 if no gamepad assigned
                break;
            case DeviceType::keyboard:
                // Enable keyboard input
                break;
            case DeviceType::mouse:
                // Show cursor
                break;
        }
    }
    
    void onDeviceDisconnected(uint32_t deviceId) override {
        std::cout << "Device disconnected: " << deviceId << std::endl;
        // Handle disconnection (pause game, show message, etc.)
    }
};

// Register observer
MyGame game;
input->addObserver(&game);

// Don't forget to remove before destruction
input->removeObserver(&game);
```

### Multiple Gamepads (Local Multiplayer)

```cpp
struct Player {
    GamepadDevice* gamepad = nullptr;
    float x = 0, y = 0;
};

std::vector<Player> players;

void assignGamepads(InputSystem* input) {
    players.clear();
    
    for (int i = 0; i < input->gamepadCount(); i++) {
        Player player;
        player.gamepad = input->gamepadAt(i);
        player.x = i * 100.0f;  // Spread out players
        players.push_back(player);
    }
}

void updatePlayers() {
    for (auto& player : players) {
        if (!player.gamepad) continue;
        
        GamepadState state;
        if (player.gamepad->getState(state)) {
            player.x += state.axes[(int)GamepadAxis::left_x].value * 5.0f;
            player.y += state.axes[(int)GamepadAxis::left_y].value * 5.0f;
        }
    }
}
```

## Best Practices {#best-practices}

### 1. Single Update Point

Call `input->update()` exactly once per frame:

```cpp
// Good
void gameLoop() {
    input->update();
    processInput();
    updateGame();
    render();
}

// Bad - multiple updates
void gameLoop() {
    input->update();
    processPlayerInput();
    input->update();  // Don't do this!
    processMenuInput();
}
```

### 2. Check Device Availability

Always check before using:

```cpp
// Good
if (input->hasKeyboard()) {
    auto* kb = input->keyboard();
    // Use keyboard
}

// Bad - will crash if no keyboard
auto* kb = input->keyboard();
kb->isKeyDown(KeyCode::space);  // nullptr dereference!
```

### 3. Support Multiple Input Methods

Allow players to use gamepad OR keyboard/mouse:

```cpp
void processInput() {
    if (input->gamepadCount() > 0) {
        processGamepadInput();
    } else {
        processKeyboardMouseInput();
    }
}
```

### 4. Apply Deadzones

Always filter stick drift:

```cpp
float stickX = applyDeadzone(
    state.axes[(int)GamepadAxis::left_x].value, 
    0.15f
);
```

### 5. Handle Disconnection Gracefully

Don't crash if a device is unplugged:

```cpp
GamepadState state;
if (!gamepad->getState(state)) {
    // Device may be disconnected
    return;
}
```

### 6. Thread Safety

Only call from one thread:

```cpp
// Main thread
void mainLoop() {
    input->update();  // OK
}

// Don't do this from another thread!
void physicsThread() {
    // input->update();  // ERROR!
}
```

### 7. Haptics Duration

Stop haptics appropriately:

```cpp
// Fire weapon
haptics->playRumble(0.5f, 0.5f, 100);

// For continuous rumble (engine, damage), stop when effect ends
if (!isEngineRunning) {
    haptics->stopRumble();
}
```
