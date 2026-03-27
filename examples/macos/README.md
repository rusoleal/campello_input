# macOS Example - campello_input

A native macOS application demonstrating the campello_input library using AppKit.

## Features

This example provides a visual input monitor for macOS:

- **Gamepad State** (up to 3 gamepads):
  - Analog sticks with position visualization
  - Triggers with fill bars
  - All buttons (face, shoulder, D-pad, menu)
  - Connection status indicators

- **Keyboard State**:
  - Modifier keys (Shift, Ctrl, Alt/Option, Cmd)
  - Recent key press history
  - Requires macOS 11.0+ and GameController framework

- **Mouse State**:
  - Position and delta tracking
  - Scroll wheel values
  - All button states (Left, Right, Middle, X1, X2)
  - Requires macOS 11.0+ and GameController framework

## Prerequisites

- macOS 11.0 (Big Sur) or later
- Xcode 12+ or Command Line Tools
- CMake 3.22.1+

## Building

### Using the helper script (recommended)

```bash
./run_macos_example.sh
```

This script will:
1. Configure the project with CMake
2. Build the application
3. Launch the resulting app bundle

### Manual build

```bash
cd examples/macos
mkdir build && cd build
cmake ..
cmake --build .
```

### Build as part of main project

```bash
cd /path/to/campello_input
mkdir build && cd build
cmake .. -DBUILD_EXAMPLES=ON
cmake --build . --target campello_input_macos_example
```

## Running

After building, the app bundle is located at:

```
examples/macos/build/campello_input_macos_example.app
```

You can run it by:
- Double-clicking in Finder
- Using `open` command: `open examples/macos/build/campello_input_macos_example.app`
- Using the helper script: `./run_macos_example.sh`

## Connecting Gamepads

### Xbox Wireless Controller
1. Press and hold the Xbox button to turn on
2. Press and hold the pairing button (next to LB)
3. On Mac: System Preferences > Bluetooth > Connect

### PlayStation DualShock/DualSense
1. Press and hold PS + Share until light bar flashes
2. On Mac: System Preferences > Bluetooth > Connect

### Wired USB Controllers
Most USB gamepads work automatically when plugged in.

## Controls

The app automatically detects and displays:
- Connected gamepads with real-time state
- Keyboard modifiers and recent keys
- Mouse position, movement, and buttons

## Implementation Details

This example demonstrates:
- **AppKit/NSView** for native macOS UI
- **NSTimer** for 60fps polling
- **campello_input unified API**
- **DeviceObserver** for hot-plug notifications
- **GameController integration** for keyboard/mouse on macOS 11+

Key files:
- `main.mm` - Application entry point
- `AppDelegate.mm` - App lifecycle management
- `InputViewController.mm` - Input polling and state management
- `InputView.mm` - Custom drawing of input state

## Troubleshooting

### "No keyboard/mouse detected" message
- Keyboard and mouse require macOS 11.0+ with GameController framework
- Check System Preferences > Security & Privacy > Input Monitoring
- The app needs permission to monitor input devices

### Controller not appearing
- Make sure controller is paired/connected before launching
- Try disconnecting and reconnecting
- Check System Preferences > Bluetooth

### Build errors
- Ensure you have Xcode Command Line Tools: `xcode-select --install`
- Check CMake can find the macOS SDK

## Notes

- The example uses ARC (Automatic Reference Counting)
- Objective-C++ (.mm files) are used for C++ interop
- The UI is custom-drawn using NSView/CoreGraphics (no Storyboard)
