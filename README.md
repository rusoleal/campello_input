# campello_input

[![Version](https://img.shields.io/badge/version-0.2.1-blue.svg)](CHANGELOG.md)
[![License](https://img.shields.io/badge/license-MIT-green.svg)](LICENSE)

A cross-platform C++20 game input library supporting gamepads, keyboards, mice, touch, and haptics on macOS, iOS, Android, Windows, Linux, and game consoles.

## Platforms

| OS | Underline API | Status |
|----|---------------|--------|
| Windows/Xbox | GameInput / GDK | ✅ Implemented |
| macOS/iOS/ipadOS/tvOS | Game Controller | ✅ Implemented |
| Android | AGDK GameActivity | ✅ Implemented |
| Linux | evdev + libevdev + libudev | ✅ Implemented |
| Nintendo | TBD | Stubs only |
| Playstation | TBD | Stubs only |

## Minimum requirements

| OS | Target |
|----|--------|
| Windows | Windows 10 (19H1) |
| Xbox | ? |
| macOS | 10.9+ |
| iOS | 7.0+ |
| ipadOS | 7.0+ |
| tvOS | 9.0+ |
| Android | API level 19 / NDK 21+ |
| Linux | Kernel 3.0+ / libevdev |
| Nintendo | TBD |
| Playstation | TBD |

## Quick Start

```cpp
#include <campello_input/input_system.hpp>

// Create input system (platform auto-detection)
auto input = systems::leal::campello_input::InputSystem::create();

// In your game loop:
while (running) {
    input->update();  // Poll input devices
    
    // Check gamepads
    for (int i = 0; i < input->gamepadCount(); i++) {
        auto* gamepad = input->gamepadAt(i);
        systems::leal::campello_input::GamepadState state;
        if (gamepad->getState(state)) {
            // Use state.axes[] and state.buttons...
        }
    }
}
```

See the [examples/](examples/) directory for complete working examples.

## Examples

- **[minimal](examples/minimal/)** - Simple command-line input demo (no dependencies)
- **[glfw_integration](examples/glfw_integration/)** - Full game-like example with GLFW
- **[windows](examples/windows/)** - Win32 GUI input monitor (Windows only)
- **[macos](examples/macos/)** - macOS AppKit GUI input monitor (macOS only)
- **[ios](examples/ios/)** - iOS UIKit input monitor with touch support (iOS only)
- **[android](examples/android/)** - Android Studio project with GameActivity

## Documentation

### API Reference

Generate API documentation with Doxygen:

```bash
mkdir build && cd build
cmake ..
cmake --build . --target docs
# Open docs/html/index.html
```

### Usage Guide

See [docs/pages/usage_guide.md](docs/pages/usage_guide.md) for detailed usage patterns including:
- Gamepad input with deadzones
- Keyboard and mouse handling
- Touch input for mobile
- Haptics/force feedback
- Device hot-plugging
- Best practices

## Changelog

See [CHANGELOG.md](CHANGELOG.md) for version history and release notes.
