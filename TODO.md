# campello_input - Production Readiness TODO

> Version: 0.1.0  
> Last updated: 2026-03-27

## Current Status

| Phase | Status | Description |
|-------|--------|-------------|
| Phase 0 | ✅ Complete | Unified Public API Design |
| Phase 1 | ✅ Complete | Critical Fixes & Core Infrastructure |
| Phase 2 | ✅ Complete | Platform Implementations |
| Phase 3 | ✅ Complete | Testing Infrastructure |
| Phase 4 | ⏳ Pending | Features & Enhancements |
| Phase 5 | 🚧 In Progress | Packaging & Distribution |
| Phase 6 | ⏳ Pending | CI/CD & Automation |
| Phase 7 | 🚧 In Progress | Examples & Ecosystem |

## Summary

This is a cross-platform C++17 game input library supporting gamepads, keyboards, mice, touch, and haptics. All major desktop and mobile platforms are now fully implemented.

## Architecture Philosophy

**Single Unified API**: Applications use one platform-independent interface regardless of target platform.

```cpp
// Same code works on macOS, iOS, Android, Windows, Linux...
auto input = InputSystem::create(platformContext);

for (int i = 0; i < input->gamepadCount(); i++) {
    GamepadState state;
    if (input->gamepadAt(i)->getState(state)) {
        // Process input...
    }
}
```

- **Public API**: Pure virtual device interfaces (`GamepadDevice`, `KeyboardDevice`, etc.) accessed through unified `InputSystem`
- **Platform Layer**: Each platform provides concrete implementations, but these are **private** - users never include platform headers
- **Build System**: CMake automatically selects correct platform implementation based on `CMAKE_SYSTEM_NAME`

---

## Platform Implementation Status

**Note**: The public API (`InputSystem`, `GamepadDevice`, etc.) is platform-independent. This table tracks the **internal platform implementations**.

| Platform | Status | Gamepad | Keyboard | Mouse | Touch | Haptics |
|----------|--------|---------|----------|-------|-------|---------|
| macOS | ✅ Ready | ✅ Complete | ✅ Complete | ✅ Complete | ❌ N/A | ✅ Complete |
| iOS/tvOS | ✅ Ready | ✅ Complete | ✅ Complete | ✅ Complete | ✅ Complete | ✅ Complete |
| Android | ✅ Ready | ✅ Complete | ✅ Complete | ✅ Complete | ✅ Complete | ✅ Complete |
| Windows/Xbox | ✅ Ready | ✅ Complete | ✅ Complete | ✅ Complete | ❌ N/A | ✅ Complete |
| Linux | ✅ Ready | ✅ Complete | ✅ Complete | ✅ Complete | ❌ N/A | ⏳ FF Pending |
| PlayStation | 🚧 Stubs | ❌ Empty | ❌ Empty | ❌ Empty | ❌ N/A | ❌ Empty |
| Nintendo | 🚧 Stubs | ❌ Empty | ❌ Empty | ❌ Empty | ❌ N/A | ❌ Empty |

**Note**: Console platforms (PlayStation, Nintendo) require NDA'd SDKs and cannot be implemented without developer access.

---

## Phase 0: Unified Public API Design ✅ COMPLETE

The unified cross-platform interface has been implemented. Applications now use identical code on all platforms.

### 0.1 InputSystem Interface ✅
The unified API is available in `inc/campello_input/input_system.hpp`:

```cpp
#include <campello_input/input_system.hpp>

// Same code works on all platforms
auto input = InputSystem::create({.native = android_app_ptr}); // Android
auto input = InputSystem::create(); // macOS, iOS, Windows, Linux

while (running) {
    input->update();  // Poll/process input events
    
    for (int i = 0; i < input->gamepadCount(); i++) {
        GamepadState state;
        if (input->gamepadAt(i)->getState(state)) {
            // Process input...
        }
    }
}
```

**Completed:**
- ✅ `InputSystem` interface designed and implemented
- ✅ `PlatformContext` structure for platform-specific initialization
- ✅ No platform headers leak into public API
- ✅ Apple implementation refactored to `AppleInputSystem`
- ✅ Android implementation refactored to `AndroidInputSystem`
- ✅ Linux implementation implemented as `LinuxInputSystem`
- ✅ Windows/Xbox implementation implemented as `GdkInputSystem`
- ✅ Factory method with platform auto-detection
- ✅ Legacy `Manager` API deprecated (still functional)

## Phase 1: Critical Fixes & Core Infrastructure ✅ COMPLETE

### 1.1 Build System & Configuration ✅
- ✅ Fix `campello_input_config.h.in` - renamed to `CAMPELLO_INPUT_VERSION_*`
- ✅ Add install target to CMake (GNUInstallDirs, export targets)
- ✅ Create package config files for CMake find_package support
- ✅ Add version compatibility checking (SOVERSION for shared libs)
- ⏳ Compiler feature detection for C++17 optional features (future)
- ⏳ Sanitizer options (ASan, UBSan, TSan) for debug builds (future)
- ✅ Fix SHARED vs STATIC library selection (platform-specific defaults)
- ⏳ Unity build support option for faster compilation (future)

### 1.2 Code Quality & Documentation ✅
- ✅ Add LICENSE headers to all source files
- ⏳ Create CONTRIBUTING.md with coding standards (future)
- ✅ Add clang-format configuration file
- ⏳ Add clang-tidy configuration and fix warnings (future)
- ✅ Document thread-safety guarantees for all public APIs
- ⏳ Add noexcept specifications where appropriate (future)
- ⏳ Review and document exception safety guarantees (future)
- ✅ Add nodiscard attributes to relevant functions

### 1.3 Unified Cross-Platform API ✅
The library exposes a **single, platform-independent API** that works identically across all platforms.

- ✅ Design unified `InputSystem` class that hides platform differences
  - ✅ `InputSystem::create()` factory that returns platform-specific implementation
  - ✅ Same API for device enumeration, state polling, and event handling on all platforms
  - ✅ Unified header (`input_system.hpp`) - no platform-specific includes needed by users
- ✅ Migrate from legacy `Manager` class to new unified `InputSystem`
  - ✅ Keep `Manager` as deprecated alias during transition
  - ✅ `ControllerData` deprecated in favor of `GamepadState`
- ✅ Standardize initialization
  - ✅ `PlatformContext` for platform-specific data
  - ✅ macOS/iOS: automatic detection
  - ✅ Android: `android_app*` via PlatformContext
  - ✅ Windows: automatic detection via GameInput
  - ✅ Linux: automatic detection via udev/evdev
- ✅ Add `[[nodiscard]]` to all relevant methods
- ✅ Thread-safety documented in public headers

---

## Phase 2: Platform Implementations ✅ COMPLETE

These implementations are **private** to the library. They implement the unified public API (`InputSystem`, `GamepadDevice`, etc.) but are not exposed to library users.

### 2.1 Windows/Xbox (GDK/GameInput API) ✅ COMPLETE
Implement private platform classes that implement the public interfaces:
- ✅ Implement `GdkGamepad` (internal, implements `GamepadDevice`)
- ✅ Implement `GdkKeyboard` (internal, implements `KeyboardDevice`)
- ✅ Implement `GdkMouse` (internal, implements `MouseDevice`)
- ✅ Implement `GdkHaptics` with impulse triggers support
- ✅ Create `GdkInputSystem` class (internal, implements `InputSystem`)
- ✅ Support Xbox-specific features (Guide button, share)
- ⏳ Test with Xbox controllers and third-party gamepads (manual)
- ✅ Handle hot-plugging via polling in update()

**Files:** `src/gdk/*`

### 2.2 Linux (evdev) ✅ COMPLETE
Implement private platform classes that implement the public interfaces:
- ✅ Implement `LinuxGamepad` (internal, implements `GamepadDevice`)
- ✅ Implement `LinuxKeyboard` (internal, implements `KeyboardDevice`)
- ✅ Implement `LinuxMouse` (internal, implements `MouseDevice`)
- ✅ Create `LinuxInputSystem` class (internal, implements `InputSystem`)
- ✅ Handle udev for device hot-plugging
- ⏳ Support common controller mappings (SDL2 gamecontrollerdb integration)
- ⏳ Handle force feedback (FF) for haptics
- ⏳ Test on major distros (Ubuntu, Fedora, Arch)
- ⏳ Consider Wayland vs X11 implications for mouse/keyboard

**Files:** `src/linux/*`

### 2.3 Android Completion ✅ COMPLETE
Implement private `AndroidInputSystem` class (implements `InputSystem` interface):
- ✅ Implement gamepad support via GameActivity motion/key events
- ✅ Implement `AndroidKeyboard` (internal class) for soft/physical keyboard
- ✅ Implement `AndroidMouse` for pointer capture (Android 12+)
- ✅ Implement `AndroidHaptics` using Vibrator/VibrationEffect
- ✅ Add proper device disconnection handling
- ⏳ Support TV/leanback input modes
- ⏳ Test with various controller types (Xbox, PS, Switch Pro)
- ⏳ Handle configuration changes (orientation, keyboard)
- ✅ Remove legacy `Manager` Android implementation, migrate to `InputSystem`

**Files:** `src/android/*`

### 2.4 PlayStation (when SDK available) ⏳ PENDING
Implement private platform classes that implement the public interfaces:
- [ ] Implement `PSGamepad` (internal, implements `GamepadDevice`)
- [ ] Implement `PSKeyboard` and `PSMouse` if supported
- [ ] Implement adaptive trigger effects (DualSense)
- [ ] Support haptic feedback (DualSense haptics)
- [ ] Create `PSInput` class (internal, implements `InputSystem`)

**Status**: Blocked - requires Sony Developer Program access

### 2.5 Nintendo (when SDK available) ⏳ PENDING
Implement private platform classes that implement the public interfaces:
- [ ] Implement `NintendoGamepad` (internal, implements `GamepadDevice`)
- [ ] Handle Joy-Con pairing/combination modes
- [ ] Support HD Rumble if API available
- [ ] Create `NintendoInput` class (internal, implements `InputSystem`)

**Status**: Blocked - requires Nintendo Developer Program access

---

## Phase 3: Testing Infrastructure ✅ COMPLETE

### 3.1 Unit Tests ✅
- ✅ **Test Framework**: Google Test via CMake FetchContent
- ✅ **Test CMake Target**: `campello_input_tests`
- ✅ **Enum Tests**: `test_enums.cpp` - GamepadButton, KeyModifier, HapticsCapability
- ✅ **State Structure Tests**: `test_state_structs.cpp` - sizes, alignment, initialization
- ✅ **Mock Implementations**: MockGamepad, MockKeyboard, MockMouse, MockInputSystem
- ✅ **Observer Pattern Tests**: `test_observer.cpp` - connect/disconnect callbacks

**Test Summary**: 63 unit tests covering:
- 16 enum/bitmask tests
- 17 state structure tests  
- 17 mock device tests
- 12 observer pattern tests

Run tests with:
```bash
cmake -B build -S . -DBUILD_TESTS=ON
cmake --build build
ctest --test-dir build --output-on-failure
```

### 3.2 Integration Tests ⏳ (Future)
- [ ] Create test harness application
- [ ] Test actual hardware on CI (if possible) or manual checklist
- [ ] Test hot-plugging scenarios
- [ ] Test multiple simultaneous devices
- [ ] Test edge cases (disconnection during poll, etc.)

### 3.3 Platform-Specific Tests ⏳ (Future)
- [ ] macOS/iOS: XCTest integration tests
- [ ] Android: Instrumented tests with Espresso
- [ ] Windows: Google Test with hardware
- [ ] Linux: Headless tests with virtual devices

### 3.4 Performance Tests ⏳ (Future)
- [ ] Benchmark poll latency per device type
- [ ] Benchmark memory allocations (should be zero after init)
- [ ] Test with 16+ simultaneous devices
- [ ] Profile callback overhead

---

## Phase 4: Features & Enhancements

### 4.1 Input Features
- [ ] Add event-based input API (callbacks on input events)
- [ ] Add input recording/playback for testing
- [ ] Add gesture recognition for touch (tap, swipe, pinch)
- [ ] Add mouse cursor customization API
- [ ] Add text input/composition support for CJK
- [ ] Add gamepad battery level reporting
- [ ] Add gamepad LED/color control API
- [ ] Add motion sensor support (gyro, accelerometer) where available

### 4.2 Haptics Enhancements
- [ ] Complete trigger haptics for Apple platforms (per-trigger engines)
- [ ] Add Linux force feedback (FF) support for gamepads
- [ ] Add haptics pattern playback API
- [ ] Add haptics composition API
- [ ] Document haptics best practices

### 4.3 Configuration & Mapping
- [ ] Create controller database (similar to SDL2 gamecontrollerdb)
- [ ] Add runtime button/axis remapping API
- [ ] Add configuration file support (JSON/YAML)
- [ ] Add deadzone configuration API
- [ ] Support Steam Input integration

---

## Phase 5: Packaging & Distribution

### 5.1 Package Managers
- [ ] Create vcpkg port
- [ ] Create Conan recipe
- [ ] Submit to package manager repositories
- [ ] Create pkg-config files

### 5.2 Documentation
- ✅ Set up Doxygen for API documentation
- ✅ Create usage guide with code examples
- [ ] Document platform-specific notes and limitations
- [ ] Create migration guide from legacy Manager API
- [ ] Add troubleshooting guide
- [ ] Create architecture decision records (ADRs)

### 5.3 Release Process
- [ ] Create CHANGELOG.md following Keep a Changelog format
- [ ] Set up automated versioning
- [ ] Create GitHub release workflow
- [ ] Generate release notes automatically
- [ ] Create binary releases for major platforms
- [ ] Sign binaries/releases

---

## Phase 6: CI/CD & Automation

### 6.1 Continuous Integration
- [ ] Set up GitHub Actions or similar
- [ ] Build matrix: macOS (x86_64, arm64), iOS, Android (multiple ABIs), Windows, Linux
- [ ] Run static analysis (clang-static-analyzer, PVS-Studio)
- [ ] Run code coverage and report to codecov/coveralls
- [ ] Build examples in CI to prevent breakage

### 6.2 Quality Gates
- [ ] Enforce clang-format in CI
- [ ] Enforce clang-tidy in CI
- [ ] Require code review for all changes
- [ ] Require tests for new features
- [ ] Documentation updates for API changes

### 6.3 Security
- [ ] Run security scanners (CodeQL, etc.)
- [ ] Fuzz test input parsing
- [ ] Review for unsafe pointer usage in platform code
- [ ] Add SBOM generation

---

## Phase 7: Examples & Ecosystem

### 7.1 Examples
- ✅ Create minimal C++ example (cross-platform)
- ✅ Create GLFW integration example
- [ ] Create SDL integration example
- [ ] Create game engine plugin examples (Unity native, Unreal)
- ✅ Add iOS example project
- ✅ Add Windows example project

### 7.2 Bindings
- [ ] Evaluate C bindings for FFI
- [ ] Evaluate C# bindings
- [ ] Evaluate Python bindings
- [ ] Evaluate Rust bindings

---

## Known Issues to Fix

1. ~~**Config**: Wrong project name in generated config header~~ ✅ FIXED
2. ~~**Android**: Keyboard/mouse/haptics incomplete~~ ✅ FIXED
3. ~~**Linux**: No implementation~~ ✅ FIXED
4. ~~**Windows**: No implementation~~ ✅ FIXED
5. **Apple**: Per-trigger haptics not implemented (low priority)
6. **All platforms**: No handling of device disconnection during active poll (edge case)

---

## Priority Ordering

### P0 (Blocking Release) ✅ COMPLETE
All P0 items are now complete. The library is ready for use.

### P1 (High Priority)
1. ⏳ Linux force feedback support
2. ⏳ CI/CD setup
3. ⏳ Package manager support (vcpkg, Conan)
4. ⏳ Performance benchmarks
5. ⏳ Event-based input API

### P2 (Medium Priority)
1. ⏳ Input recording/playback
2. ⏳ Controller database
3. ⏳ Additional examples
4. ⏳ Gesture recognition

### P3 (Future)
1. ⏳ Console platforms (PlayStation, Nintendo) - blocked by SDK access
2. ⏳ Language bindings
3. ⏳ Advanced haptics patterns
4. ⏳ Motion sensor support

---

## Resources & References

- [USB HID Usage Tables](https://usb.org/sites/default/files/hut1_2.pdf) - For KeyCode values
- [SDL2 GameControllerDB](https://github.com/mdqinc/SDL_GameControllerDB) - Controller mappings reference
- [GameInput API docs](https://docs.microsoft.com/en-us/gaming/gdk/_content/gc/input/overviews/input-overview) - Windows implementation
- [Android GameActivity](https://developer.android.com/games/agdk/game-activity) - Android implementation
- [Apple GameController](https://developer.apple.com/documentation/gamecontroller) - Apple implementation
- [Linux evdev docs](https://www.kernel.org/doc/html/latest/input/input.html) - Linux implementation
