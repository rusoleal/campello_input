# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

`campello_input` is a cross-platform C++20 game input library (version 0.2.1). It abstracts platform-specific input APIs behind a unified interface in the `systems::leal::campello_input` namespace. The project is in early development.

## Build

The project uses CMake (minimum 3.22.1). The build system automatically selects the correct platform implementation based on `CMAKE_SYSTEM_NAME`.

Standard CMake configure and build:
```sh
cmake -B build -S .
cmake --build build
```

For Android, build via the Android example project using Android Studio or Gradle:
```sh
cd examples/android
./gradlew assembleDebug
```

Tests are configured via Google Test. Run with `cmake --build build && ctest --test-dir build`.

## Architecture

### Public API (`inc/campello_input/`)

Device abstraction headers — all pure virtual interfaces:
- `device.hpp` — `Device` base class; `DeviceType` and `ConnectionStatus` enums
- `gamepad.hpp` — `GamepadDevice`, `GamepadButton`/`GamepadAxis` enums, `GamepadState`
- `keyboard.hpp` — `KeyboardDevice`, `KeyCode` (USB HID page 0x07 values), `KeyModifier`, `KeyEvent`, `KeyboardState`
- `mouse.hpp` — `MouseDevice`, `MouseButton`, `MouseState`
- `touch.hpp` — `TouchDevice`, `TouchPoint`, `TouchState`, `TouchPhase`
- `haptics.hpp` — `Haptics` interface (output); `RumbleEffect`, `TriggerEffect`, `HapticsCapability`
- `manager.hpp` — `Manager` class (older interface, pre-dates the device abstractions above; pending migration)
- `controller_data.hpp` — legacy data structs used by `Manager`; superseded by `gamepad.hpp`

### Platform Implementations (`src/<platform>/`)

Each platform has source files compiled by its `<platform>.cmake`. Private headers live in `src/<platform>/inc/` and are not part of the public API.

| Platform | cmake file | Native API |
|---|---|---|
| macOS | `macos.cmake` | Game Controller + CoreHaptics |
| iOS / tvOS | `ios.cmake` | Game Controller + CoreHaptics + UIKit |
| Android | `android.cmake` | GameActivity / AGDK |
| Linux | `linux.cmake` | evdev / libevdev + libudev |
| Windows/Xbox | `windows.cmake` | GDK / D3D12 |

#### Apple platform (`src/apple/`)

macOS and iOS share the same source files. Touch (`AppleTouch`) is compiled on all platforms but guarded with `#if TARGET_OS_IOS || TARGET_OS_TV` at file scope. Only `ios.cmake` passes `touch_apple.mm` to the build.

Concrete types (private, in `src/apple/inc/campello_input/`):

| File | Class | Wraps |
|---|---|---|
| `gamepad_apple.mm` | `AppleGamepad` | `GCController` / `GCExtendedGamepad` |
| `haptics_apple.mm` | `AppleHaptics` | `GCDeviceHaptics` / `CHHapticEngine` |
| `keyboard_apple.mm` | `AppleKeyboard` | `GCKeyboard` / `GCKeyboardInput` |
| `mouse_apple.mm` | `AppleMouse` | `GCMouse` / `GCMouseInput` |
| `touch_apple.mm` | `AppleTouch` | `UITouch` (iOS/tvOS only) |

Key implementation notes:
- ObjC objects are stored as `void *` in C++ class members, managed via `CFBridgingRetain` / `CFBridgingRelease`.
- `GCKeyboard` / `GCMouse` require `@available(macOS 11.0, iOS 14.0, *)` guards.
- `AppleGamepad::getState()` inverts the Y axis of both thumbsticks (`-yAxis.value`) because the GC framework uses +Y=up while this library normalises to +Y=down.
- `AppleMouse` accumulates X/Y deltas via `valueChangedHandler` because `GCMouse` axes report velocity, not absolute position.
- `AppleTouch` must be fed events from the app's `UIView` / `UIViewController` via `processTouch*` methods — touch is not polled like the other devices.
- `AppleHaptics` creates one `CHHapticEngine` for `GCHapticsLocalityDefault` (main rumble). Per-trigger engines (`GCHapticsLocalityLeftTrigger` / `Right`) are not yet implemented.

#### Android platform (`src/android/`)

Uses GameActivity from the Android Game Development Kit (AGDK) for input handling. Processes input events via `android_app_swap_input_buffers()`.

Concrete types (private, in `src/android/inc/campello_input/`):

| File | Class | Wraps |
|---|---|---|
| `input_android_system.cpp` | `AndroidInputSystem` | `android_app` / GameActivity |
| `gamepad` (in input_android.hpp) | `AndroidGamepad` | `AINPUT_SOURCE_JOYSTICK` |
| `keyboard_android.cpp` | `AndroidKeyboard` | `AINPUT_SOURCE_KEYBOARD` |
| `mouse_android.cpp` | `AndroidMouse` | `AINPUT_SOURCE_MOUSE` |
| `touch_android.cpp` | `AndroidTouch` | `AINPUT_SOURCE_TOUCHSCREEN` |
| `haptics_android.cpp` | `AndroidHaptics` | `Vibrator` service (JNI) |

Key implementation notes:
- Gamepad: Uses `AKEYCODE_BUTTON_*` for buttons, `AMOTION_EVENT_AXIS_*` for axes. Supports standard Xbox/PlayStation layouts.
- Keyboard: Maps Android key codes to USB HID key codes. Supports physical keyboards and software keyboard events.
- Mouse: Supports external mice and trackpads on Chromebooks and tablets. Uses relative motion axes when available.
- Touch: Multi-touch support with up to 10 simultaneous touch points.
- Haptics: Uses Android Vibrator service via JNI. Supports amplitude control on Android 8.0+ (API 26+).

#### Linux platform (`src/linux/`)

Uses `libevdev` for device access and `libudev` for hot-plug detection. All Linux devices use the evdev interface (`/dev/input/event*`).

Concrete types (private, in `src/linux/inc/campello_input/`):

| File | Class | Wraps |
|---|---|---|
| `gamepad_linux.cpp` | `LinuxGamepad` | evdev joystick/gamepad |
| `keyboard_linux.cpp` | `LinuxKeyboard` | evdev keyboard |
| `mouse_linux.cpp` | `LinuxMouse` | evdev mouse |

Key implementation notes:
- Device discovery uses `udev` for both initial enumeration and runtime hot-plugging.
- Gamepad axis/button mappings are discovered dynamically from evdev capabilities.
- Common USB HID gamepads (Xbox, PlayStation) are detected via udev properties; fallback name-based heuristics are used when udev properties are missing.
- Touchpad devices are excluded from mouse detection to avoid treating integrated trackpads as external mice.
- Haptics (force feedback) is not yet implemented; `LinuxGamepad::haptics()` returns `nullptr`.

#### Windows/Xbox platform (`src/gdk/`)

Uses the GameInput API from the Microsoft Game Development Kit (GDK). GameInput is the modern input API for Windows 10/11 and Xbox, replacing XInput and DirectInput.

Concrete types (private, in `src/gdk/inc/campello_input/`):

| File | Class | Wraps |
|---|---|---|
| `input_gdk_system.cpp` | `GdkInputSystem` | `IGameInput` |
| `gamepad_gdk.cpp` | `GdkGamepad` | `IGameInputDevice` (Gamepad) |
| `keyboard_gdk.cpp` | `GdkKeyboard` | `IGameInputDevice` (Keyboard) |
| `mouse_gdk.cpp` | `GdkMouse` | `IGameInputDevice` (Mouse) |
| `haptics_gdk.cpp` | `GdkHaptics` | `IGameInputRumbleDevice` |

Key implementation notes:
- GameInput provides unified access to gamepads, keyboards, and mice through the same API.
- Gamepad: Uses `GameInputGamepadState` for buttons and axes. Y-axes are inverted to match library conventions (+Y=down).
- Keyboard: Maps USB HID scan codes to KeyCode enum. Supports up to 16 simultaneous keys in one reading.
- Mouse: Position and delta tracking, 5-button support, scroll wheel.
- Haptics: Supports low/high frequency rumble and trigger rumble (Xbox Series X|S controllers).
- Device hot-plugging is detected via polling in `update()` - GameInput provides device arrival/removal callbacks but this implementation uses simple polling.

### Android Example (`examples/android/`)

A full GameActivity-based Android Studio project. It pulls in the library via `app/src/main/cpp/campello_input.cmake` using CMake `FetchContent` pointing to the repo root. The native library is loaded as `"example"` from `MainActivity.kt`.

### Key Design Pattern

Platform-specific objects are passed as `void *` into C++ constructors/factories, keeping public headers free of platform SDK types. This is used both in `Manager::init(void *pd)` (old interface) and in the new Apple concrete types.
