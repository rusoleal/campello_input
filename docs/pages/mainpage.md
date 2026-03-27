# campello_input

**Version 0.1.0** | [GitHub Repository](https://github.com/rubenleal/campello_input)

A cross-platform C++17 game input library supporting gamepads, keyboards, mice, touch, and haptics on macOS, iOS, Android, Windows, and Linux.

## Features

- **Unified API**: Same code works on all platforms
- **Gamepad Support**: Xbox, PlayStation, Nintendo, and generic controllers
- **Keyboard & Mouse**: Full keyboard state, mouse position/deltas/buttons
- **Touch Input**: Multi-touch support on mobile platforms
- **Haptics**: Rumble and trigger feedback where supported
- **Hot-plugging**: Devices connect/disconnect at runtime
- **Zero Allocations**: No heap allocations during gameplay after initialization
- **Thread-Safe**: Device state reading is thread-safe

## Quick Start {#quickstart}

```cpp
#include <campello_input/input_system.hpp>
#include <campello_input/gamepad.hpp>

// Create input system
auto input = systems::leal::campello_input::InputSystem::create();

// Main loop
while (running) {
    input->update();  // Poll all devices
    
    // Check gamepads
    for (int i = 0; i < input->gamepadCount(); i++) {
        auto* gamepad = input->gamepadAt(i);
        
        systems::leal::campello_input::GamepadState state;
        if (gamepad->getState(state)) {
            // Left stick
            float moveX = state.axes[(int)GamepadAxis::left_x].value;
            float moveY = state.axes[(int)GamepadAxis::left_y].value;
            
            // Face buttons
            if (state.buttons & (uint64_t)GamepadButton::face_south) {
                // A button pressed
            }
        }
    }
}
```

## Supported Platforms

| Platform | Status | Gamepad | Keyboard | Mouse | Touch | Haptics |
|----------|--------|---------|----------|-------|-------|---------|
| macOS | ✅ Ready | ✅ | ✅ | ✅ | N/A | ✅ |
| iOS/tvOS | ✅ Ready | ✅ | ✅ | ✅ | ✅ | ✅ |
| Android | ✅ Ready | ✅ | ✅ | ✅ | ✅ | ✅ |
| Windows/Xbox | ✅ Ready | ✅ | ✅ | ✅ | N/A | ✅ |
| Linux | ✅ Ready | ✅ | ✅ | ✅ | N/A | ⏳ |
| PlayStation | 🚧 Stubs | ❌ | ❌ | ❌ | N/A | ❌ |
| Nintendo | 🚧 Stubs | ❌ | ❌ | ❌ | N/A | ❌ |

## Installation {#installation}

### Using CMake FetchContent (Recommended)

```cmake
include(FetchContent)
FetchContent_Declare(
    campello_input
    GIT_REPOSITORY https://github.com/rubenleal/campello_input.git
    GIT_TAG v0.1.0
)
FetchContent_MakeAvailable(campello_input)

target_link_libraries(your_target campello_input)
```

### Using vcpkg (Coming Soon)

```bash
vcpkg install campello_input
```

### Manual Installation

```bash
git clone https://github.com/rubenleal/campello_input.git
cd campello_input
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build .
sudo cmake --install .
```

## Platform Notes {#platform_notes}

### Android

Requires GameActivity from AGDK. Pass `android_app*` via `PlatformContext`:

```cpp
PlatformContext ctx{.native = android_app_ptr};
auto input = InputSystem::create(ctx);
```

### iOS/tvOS

Requires linking against GameController and CoreHaptics frameworks (handled automatically by CMake).

### Linux

Requires libevdev and libudev:

```bash
# Ubuntu/Debian
sudo apt-get install libevdev-dev libudev-dev

# Fedora
sudo dnf install libevdev-devel systemd-devel

# Arch
sudo pacman -S libevdev systemd-libs
```

### Windows/Xbox

Requires Windows 10 (19H1) or later. Uses GameInput API from GDK.

## Examples

See the `examples/` directory for complete working examples:

- **minimal** - Simple command-line input demo
- **glfw_integration** - Full game-like example with rendering
- **android** - Android Studio project

## API Overview

### Core Classes

| Class | Description |
|-------|-------------|
| @ref systems::leal::campello_input::InputSystem "InputSystem" | Main entry point for all input |
| @ref systems::leal::campello_input::GamepadDevice "GamepadDevice" | Gamepad access and state |
| @ref systems::leal::campello_input::KeyboardDevice "KeyboardDevice" | Keyboard input |
| @ref systems::leal::campello_input::MouseDevice "MouseDevice" | Mouse input |
| @ref systems::leal::campello_input::TouchDevice "TouchDevice" | Touch input (mobile) |
| @ref systems::leal::campello_input::Haptics "Haptics" | Force feedback |

### State Structures

| Structure | Description |
|-----------|-------------|
| @ref systems::leal::campello_input::GamepadState "GamepadState" | Complete gamepad state |
| @ref systems::leal::campello_input::KeyboardState "KeyboardState" | Keyboard key bitmap |
| @ref systems::leal::campello_input::MouseState "MouseState" | Mouse position and buttons |
| @ref systems::leal::campello_input::TouchState "TouchState" | Multi-touch state |

## Thread Safety

- **InputSystem** - Not thread-safe. Create and use from a single thread (typically main/UI thread).
- **Device methods** (getState, isKeyDown, etc.) - Thread-safe for concurrent reads.
- **Observer callbacks** - Called on the thread that called `update()`.

## License

MIT License - See LICENSE file for details.

## Contributing

Contributions are welcome! Please read CONTRIBUTING.md for guidelines.
