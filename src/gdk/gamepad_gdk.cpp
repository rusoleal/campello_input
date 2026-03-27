// MIT License
//
// Copyright (c) 2025 Ruben Leal Mirete
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include "inc/campello_input/gamepad_gdk.hpp"
#include "inc/campello_input/haptics_gdk.hpp"
#include <GameInput.h>
#include <cstring>

using namespace systems::leal::campello_input;

// Map GameInput buttons to GamepadButton
static uint64_t gameInputButtonToGamepadButton(GameInputGamepadButtons btn) {
    switch (btn) {
        case GameInputGamepadA:              return static_cast<uint64_t>(GamepadButton::face_south);
        case GameInputGamepadB:              return static_cast<uint64_t>(GamepadButton::face_east);
        case GameInputGamepadX:              return static_cast<uint64_t>(GamepadButton::face_west);
        case GameInputGamepadY:              return static_cast<uint64_t>(GamepadButton::face_north);
        case GameInputGamepadLeftShoulder:   return static_cast<uint64_t>(GamepadButton::shoulder_left);
        case GameInputGamepadRightShoulder:  return static_cast<uint64_t>(GamepadButton::shoulder_right);
        case GameInputGamepadLeftThumbstick: return static_cast<uint64_t>(GamepadButton::thumb_left);
        case GameInputGamepadRightThumbstick:return static_cast<uint64_t>(GamepadButton::thumb_right);
        case GameInputGamepadMenu:           return static_cast<uint64_t>(GamepadButton::start);
        case GameInputGamepadView:           return static_cast<uint64_t>(GamepadButton::select);
        case GameInputGamepadDPadUp:         return static_cast<uint64_t>(GamepadButton::dpad_up);
        case GameInputGamepadDPadDown:       return static_cast<uint64_t>(GamepadButton::dpad_down);
        case GameInputGamepadDPadLeft:       return static_cast<uint64_t>(GamepadButton::dpad_left);
        case GameInputGamepadDPadRight:      return static_cast<uint64_t>(GamepadButton::dpad_right);
        default:                             return 0;
    }
}

GdkGamepad::GdkGamepad(uint32_t id, IGameInputDevice* device)
    : _id(id)
    , _device(device)
{
    if (_device) {
        _device->AddRef();
    }
}

GdkGamepad::~GdkGamepad() {
    if (_device) {
        _device->Release();
        _device = nullptr;
    }
}

bool GdkGamepad::initialize() {
    if (!_device) return false;
    
    // Get device info for name
    const GameInputDeviceInfo* info = _device->GetDeviceInfo();
    if (info && info->displayName) {
        _name = info->displayName;
    } else {
        _name = "Gamepad " + std::to_string(_id);
    }
    
    // Initialize haptics
    _haptics = std::make_unique<GdkHaptics>(_device);
    if (!_haptics->initialize()) {
        _haptics.reset();
    }
    
    return true;
}

void GdkGamepad::updateState(IGameInputReading* reading) {
    if (!reading) return;
    
    GameInputGamepadState gamepadState;
    if (!reading->GetGamepadState(&gamepadState)) return;
    
    // Update axes
    _state.axes[static_cast<int>(GamepadAxis::left_x)].value = gamepadState.leftThumbstickX;
    _state.axes[static_cast<int>(GamepadAxis::left_y)].value = -gamepadState.leftThumbstickY;  // Invert Y
    _state.axes[static_cast<int>(GamepadAxis::right_x)].value = gamepadState.rightThumbstickX;
    _state.axes[static_cast<int>(GamepadAxis::right_y)].value = -gamepadState.rightThumbstickY;  // Invert Y
    _state.axes[static_cast<int>(GamepadAxis::trigger_left)].value = gamepadState.leftTrigger;
    _state.axes[static_cast<int>(GamepadAxis::trigger_right)].value = gamepadState.rightTrigger;
    
    // Update buttons
    _state.buttons = 0;
    
    // Check each button bit
    uint32_t buttons = gamepadState.buttons;
    for (uint32_t i = 0; i < 32; ++i) {
        if (buttons & (1u << i)) {
            uint64_t btn = gameInputButtonToGamepadButton(static_cast<GameInputGamepadButtons>(1u << i));
            if (btn != 0) {
                _state.buttons |= btn;
            }
        }
    }
    
    // Update timestamp
    _state.timestamp = reading->GetTimestamp();
}

bool GdkGamepad::getState(GamepadState& outState) const {
    outState = _state;
    return true;
}

Haptics* GdkGamepad::haptics() noexcept {
    return _haptics.get();
}
