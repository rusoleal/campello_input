# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added
- Linux Force Feedback (haptics) support
- Controller database for automatic mappings

## [0.1.0] - 2026-03-27

### Added

#### Platforms
- **Linux implementation** using evdev/libevdev/libudev
  - Gamepad support with dynamic axis/button mapping
  - Keyboard with full key code mapping
  - Mouse with relative/absolute positioning
  - udev-based hot-plug detection
- **Windows/Xbox GDK implementation** using GameInput API
  - Gamepad with full Xbox controller support
  - Keyboard with USB HID scan code mapping
  - Mouse with 5-button support
  - Haptics with trigger rumble support (Xbox Series X|S)
- **Android completion**
  - Full `AndroidKeyboard` implementation
  - Full `AndroidMouse` implementation  
  - `AndroidHaptics` using Vibrator service
  - JNI-based haptics with amplitude control (API 26+)

#### Examples
- `examples/minimal/` - Simple cross-platform command-line demo
- `examples/glfw_integration/` - Full game-like example with rendering
- `examples/windows/` - Win32 GUI input monitor
- `examples/ios/` - iOS UIKit input monitor with touch support

#### Documentation
- Doxygen-based API documentation system
- Comprehensive usage guide (`docs/pages/usage_guide.md`)
- Platform-specific implementation notes in CLAUDE.md

#### Testing
- Google Test framework with 63 unit tests
- Enum/bitmask operator tests
- State structure validation tests
- Mock device implementation tests
- Observer pattern tests

#### Build System
- CMake package config files for `find_package()` support
- GNUInstallDirs-based installation layout
- SOVERSION for shared libraries
- Doxygen integration (`docs` target)

### Changed
- Unified cross-platform API (`InputSystem`, device interfaces)
- All platform implementations now use common base interfaces
- License headers added to all source files
- `.clang-format` configuration for consistent code style
- `[[nodiscard]]` attributes added to public API methods

### Fixed
- Config header project name (`campello_gpu` → `campello_input`)
- CTest discovery with `enable_testing()`
- Struct size tests (padding issues)

## [0.0.13] - 2026-03-27 (Initial Development)

### Added
- Project initialization
- macOS/iOS implementation (Apple Game Controller framework)
- Basic Android implementation (GameActivity, partial)
- Core public API headers
- Legacy Manager API (deprecated)
- Initial build system

[Unreleased]: https://github.com/rubenleal/campello_input/compare/v0.1.0...HEAD
[0.1.0]: https://github.com/rubenleal/campello_input/compare/v0.0.13...v0.1.0
[0.0.13]: https://github.com/rubenleal/campello_input/releases/tag/v0.0.13
