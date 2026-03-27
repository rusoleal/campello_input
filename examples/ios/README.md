# iOS Example - campello_input

A native iOS application demonstrating the campello_input library using UIKit.

## Features

This example provides a visual input monitor optimized for iOS devices:

- **Gamepad State** (supports multiple gamepads):
  - Analog sticks with position visualization
  - Face buttons and shoulder buttons
  - Real-time button press indicators

- **Keyboard State** (iPad/external keyboards):
  - Modifier keys (Shift, Ctrl, Alt, Cmd)
  - Key press tracking

- **Mouse State** (iPad with mouse/trackpad):
  - Cursor position
  - Button states

- **Touch State** (iOS-specific):
  - Multi-touch count
  - Touch phase tracking

## Prerequisites

- macOS with Xcode 14+
- iOS 14.0+ device or simulator
- Valid Apple Developer account (for device deployment)

## Building

### Using Xcode

```bash
cd examples/ios
mkdir build && cd build
cmake .. -G Xcode
open campello_input_ios_example.xcodeproj
```

Then build and run from Xcode.

### Using Command Line

```bash
cd examples/ios
mkdir build && cd build
cmake .. -DCMAKE_SYSTEM_NAME=iOS \
         -DCMAKE_OSX_SYSROOT=iphoneos \
         -DCMAKE_OSX_ARCHITECTURES=arm64
cmake --build .
```

## Running

1. Connect an iOS device or use the Simulator
2. Build and run the app
3. Connect a gamepad via Bluetooth (Settings > Bluetooth)
4. Use the app to monitor input in real-time

## Connecting a Gamepad

### Xbox Wireless Controller
1. Press and hold the Xbox button to turn on
2. Press and hold the pairing button (small button next to LB)
3. On iOS: Settings > Bluetooth > Select your controller

### PlayStation DualShock/DualSense
1. Press and hold PS + Share buttons until light bar flashes
2. On iOS: Settings > Bluetooth > Select your controller

### MFi Controllers
1. Follow manufacturer's pairing instructions
2. Should appear in Settings > Bluetooth

## Controls

- **Gamepad**: All buttons and axes are displayed in real-time
- **Touch**: Displayed in the touch section
- **Keyboard**: Shows modifier states (iPad with keyboard)
- **Mouse**: Shows cursor position (iPad with mouse/trackpad)

## Implementation Details

This example uses:
- **UIKit** for native iOS UI
- **CADisplayLink** for 60fps updates
- **campello_input unified API** for cross-platform input
- **Auto Layout** for responsive design

Key differences from macOS version:
- Portrait orientation support
- Touch input display (iOS-specific)
- Optimized layout for smaller screens
- Support for both iPhone and iPad

## Troubleshooting

### Controller not appearing
- Make sure controller is paired in iOS Settings > Bluetooth
- Some controllers require firmware updates for iOS support
- Try disconnecting and reconnecting

### Build errors
- Make sure you have the latest Xcode
- Check that CMAKE_OSX_SYSROOT points to a valid iOS SDK
- For device builds, set a valid DEVELOPMENT_TEAM in CMake

### App crashes on launch
- Check that GameController and CoreHaptics frameworks are linked
- Verify the Info.plist has proper bundle identifier

## Notes

- iOS 14+ is required for GameController keyboard/mouse support
- Touch input is always available (this is iOS!)
- External keyboards require iPad or iOS 15+ with keyboard support
