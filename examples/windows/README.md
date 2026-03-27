# Windows Example - campello_input

A Win32 GUI application demonstrating the campello_input library on Windows.

## Features

This example provides a visual input monitor showing:

- **Gamepad State** (up to 2 gamepads):
  - Left and right analog sticks with position visualization
  - Left and right triggers with fill level
  - Face buttons (A, B, X, Y)
  - Shoulder buttons (LB, RB)
  - D-pad directions
  - Menu buttons (View/Select, Menu/Start)

- **Keyboard State**:
  - Modifier keys (Shift, Ctrl, Alt, Win)
  - Common game keys (W, A, S, D, Space, Enter, Esc)
  - Real-time key press visualization

- **Mouse State**:
  - Cursor position
  - Delta movement
  - Scroll wheel values
  - Button states (Left, Right, Middle)

## Building

### Prerequisites

- Windows 10 (19H1) or later
- Visual Studio 2019/2022 or MinGW-w64
- CMake 3.22.1 or later

### Build with Visual Studio

```cmd
cd examples/windows
mkdir build && cd build
cmake .. -G "Visual Studio 17 2022"
cmake --build . --config Release
```

### Build with MinGW

```cmd
cd examples/windows
mkdir build && cd build
cmake .. -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release
cmake --build .
```

### Quick Build (from repo root)

```cmd
run_windows_example.bat
```

## Running

```cmd
.\build\Release\campello_input_windows_example.exe
```

Or simply double-click the executable.

## Controls

- Connect a gamepad to see its state visualized
- Press keys to see them highlighted
- Move mouse to see position and button states
- Press **ESC** to exit

## Implementation Details

This example uses:
- **Win32 API** for windowing and rendering (no external dependencies)
- **GDI** for simple 2D graphics
- **60fps timer** for smooth updates
- **Direct campello_input usage** via the unified API

The example demonstrates:
- Creating and using `InputSystem`
- Polling gamepad, keyboard, and mouse state
- Visualizing analog stick positions
- Displaying button press states
- Handling window messages alongside input polling

## Troubleshooting

### Gamepad not detected
- Make sure the gamepad is connected before starting the app
- Try disconnecting and reconnecting the gamepad
- Some gamepads may need drivers (especially PlayStation controllers)

### No keyboard/mouse shown
- If a gamepad is connected, keyboard/mouse appears on the right side
- If no gamepad, keyboard/mouse takes the full width

### Build errors
- Ensure you have Windows SDK installed
- For MinGW, use MinGW-w64 (not the old MinGW)
