# campello_input Examples

This directory contains example applications demonstrating how to use the `campello_input` library.

## Examples

### 1. Minimal Example (`minimal/`)

A simple command-line example that demonstrates basic input polling across all supported platforms. This example has no external dependencies beyond the library itself.

**Features:**
- Gamepad input (sticks, triggers, buttons)
- Keyboard input (key states, modifiers)
- Mouse input (position, buttons, scroll)
- Device connection/disconnection notifications
- Cross-platform (works on all supported platforms)

**Build:**
```bash
cd minimal
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build .
./minimal_example
```

**Controls:**
- Connect a gamepad, keyboard, or mouse to see live input
- Press **ESC** on keyboard to exit
- Press **Ctrl+C** to exit on terminal

---

### 2. GLFW Integration Example (`glfw_integration/`)

A complete game-like example showing best practices for integrating `campello_input` with a real windowing library. This example demonstrates:

**Features:**
- Windowing with GLFW
- Player movement with both gamepad and keyboard/mouse
- Haptic feedback (rumble) on gamepad
- Dynamic input switching (gamepad vs keyboard/mouse)
- Real-time rendering with OpenGL

**Prerequisites:**
- GLFW3 development libraries
- OpenGL support

**macOS:**
```bash
brew install glfw
```

**Ubuntu/Debian:**
```bash
sudo apt-get install libglfw3-dev
```

**Fedora:**
```bash
sudo dnf install glfw-devel
```

**Windows (vcpkg):**
```bash
vcpkg install glfw3
```

**Build:**
```bash
cd glfw_integration
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build .
./glfw_example
```

**Controls:**

*Gamepad:*
- Left Stick: Move player
- Right Stick: Aim
- A/Cross (South face button): Fire (with haptic feedback)
- B/Circle (East face button): Jump
- Start: Toggle pause
- Back/Select: Menu

*Keyboard/Mouse:*
- WASD: Move player
- Mouse: Aim
- Left Click: Fire
- Right Click: Jump
- SPACE: Alternative fire
- SHIFT: Alternative jump
- ESC: Exit

---

### 3. Windows Example (`windows/`)

A native Win32 GUI application that provides a visual input monitor for Windows. This example uses only Windows APIs (no external dependencies) to display real-time input state.

**Features:**
- Visual gamepad state (sticks, triggers, buttons)
- Keyboard key press visualization
- Mouse position and button tracking
- 60fps real-time updates
- No external dependencies (uses Win32 + GDI)

**Build (Visual Studio):**
```cmd
cd windows
mkdir build && cd build
cmake .. -G "Visual Studio 17 2022"
cmake --build . --config Release
.\Release\campello_input_windows_example.exe
```

**Build (MinGW):**
```cmd
cd windows
mkdir build && cd build
cmake .. -G "MinGW Makefiles"
cmake --build .
.\campello_input_windows_example.exe
```

**Quick Run:**
```cmd
# From repo root
run_windows_example.bat
```

---

### 4. iOS Example (`ios/`)

A native iOS application using UIKit that demonstrates input handling on iPhone and iPad. This example shows touch input along with gamepad support.

**Features:**
- Gamepad input (sticks, buttons, triggers)
- Touch input visualization (iOS-specific)
- Keyboard support (iPad with external keyboard)
- Mouse support (iPad with trackpad)
- 60fps real-time updates
- Native iOS UI with Auto Layout

**Prerequisites:**
- macOS with Xcode 14+
- iOS 14.0+ device or simulator

**Build:**
```bash
cd ios
mkdir build && cd build
cmake .. -G Xcode
open campello_input_ios_example.xcodeproj
```

Then build and run from Xcode.

**Connecting Gamepads:**
- Xbox/PlayStation controllers: Pair in Settings > Bluetooth
- MFi controllers: Follow manufacturer instructions
- Supported on iOS 13+ with Bluetooth controllers

---

## Platform-Specific Notes

### Android

For Android, use the dedicated example project in `examples/android/`. This uses Android Studio and the GameActivity API.

### iOS/tvOS

See the `ios/` example for a complete UIKit-based input monitor. It demonstrates gamepad, touch, keyboard, and mouse input on iOS devices.

### Windows

On Windows, both examples work with Visual Studio or MinGW. Make sure to install GLFW and set the appropriate CMake prefix path if needed.

### Linux

Both examples work on Linux with X11 or Wayland. GLFW handles the display server differences internally.

---

## Integration Guide

To integrate `campello_input` into your own project:

### Method 1: FetchContent (Recommended)

```cmake
include(FetchContent)
FetchContent_Declare(
    campello_input
    GIT_REPOSITORY https://github.com/yourusername/campello_input.git
    GIT_TAG v0.1.0
)
FetchContent_MakeAvailable(campello_input)

target_link_libraries(your_target campello_input)
```

### Method 2: Installed Package

```bash
# Install campello_input first
cmake --build build --target install
```

Then in your CMake:

```cmake
find_package(campello_input 0.1.0 REQUIRED)
target_link_libraries(your_target campello_input::campello_input)
```

### Method 3: Subdirectory

```cmake
add_subdirectory(third_party/campello_input)
target_link_libraries(your_target campello_input)
```

---

## Best Practices

1. **Call `update()` once per frame** - This polls for new input events
2. **Check device availability** - Always check `hasKeyboard()`, `hasMouse()`, etc. before using devices
3. **Handle disconnection** - Use `DeviceObserver` to handle hot-plugging
4. **Support multiple input methods** - Allow players to use gamepad OR keyboard/mouse
5. **Apply deadzones** - Gamepad sticks often have small drift; filter values < 0.15
6. **Normalize axes** - The library normalizes all axes to [-1, 1] or [0, 1]
7. **Thread safety** - Only call input methods from the thread that created the InputSystem

---

## Troubleshooting

**No devices detected:**
- On Linux: Make sure your user is in the `input` group
- On Windows: Try running as Administrator (for some devices)
- On macOS: Grant input monitoring permissions in System Preferences

**Haptics not working:**
- Not all gamepads support rumble
- Check `haptics()->supports(HapticsCapability::rumble)` before playing

**Build errors:**
- Make sure you have a C++17 compatible compiler
- Check that CMake can find the library (set `campello_input_DIR` if needed)

---

## More Information

- [Main README](../README.md)
- [API Documentation](../CLAUDE.md)
- [Integration Guide](https://github.com/yourusername/campello_input/wiki)
