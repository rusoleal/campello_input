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

#include "inc/campello_input/mouse_gdk.hpp"
#include <GameInput.h>
#include <cstring>

using namespace systems::leal::campello_input;

// Map GameInput mouse buttons to MouseButton
static MouseButton gameInputButtonToMouseButton(uint32_t buttonIndex) {
    switch (buttonIndex) {
        case 0: return MouseButton::left;
        case 1: return MouseButton::right;
        case 2: return MouseButton::middle;
        case 3: return MouseButton::extra_1;
        case 4: return MouseButton::extra_2;
        default: return MouseButton::left;
    }
}

GdkMouse::GdkMouse(uint32_t id, IGameInputDevice* device)
    : _id(id)
    , _device(device)
{
    if (_device) {
        _device->AddRef();
    }
}

GdkMouse::~GdkMouse() {
    if (_device) {
        _device->Release();
        _device = nullptr;
    }
}

bool GdkMouse::initialize() {
    if (!_device) return false;
    
    // Get device info for name
    const GameInputDeviceInfo* info = _device->GetDeviceInfo();
    if (info && info->displayName) {
        _name = info->displayName;
    } else {
        _name = "Mouse " + std::to_string(_id);
    }
    
    return true;
}

void GdkMouse::updateState(IGameInputReading* reading) {
    if (!reading) return;
    
    // Get mouse state
    GameInputMouseState mouseState;
    if (!reading->GetMouseState(&mouseState)) return;
    
    // Update position
    int32_t newX = static_cast<int32_t>(mouseState.positionX);
    int32_t newY = static_cast<int32_t>(mouseState.positionY);
    
    // Calculate delta
    if (_hasLastPos) {
        _state.delta_x = newX - _lastX;
        _state.delta_y = newY - _lastY;
    } else {
        _state.delta_x = 0;
        _state.delta_y = 0;
        _hasLastPos = true;
    }
    
    _lastX = newX;
    _lastY = newY;
    _state.pos_x = newX;
    _state.pos_y = newY;
    
    // Update wheel
    _state.scroll_delta_x = mouseState.wheelX;
    _state.scroll_delta_y = mouseState.wheelY;
    
    // Update buttons
    _state.buttons = 0;
    uint32_t buttonCount = mouseState.buttonCount;
    if (buttonCount > 5) buttonCount = 5;  // Max 5 buttons supported
    
    for (uint32_t i = 0; i < buttonCount; ++i) {
        if (mouseState.buttons[i]) {
            _state.buttons |= static_cast<uint32_t>(gameInputButtonToMouseButton(i));
        }
    }
    
    // Update timestamp
    _state.timestamp = reading->GetTimestamp();
}

void GdkMouse::getState(MouseState& outState) const {
    outState = _state;
}

bool GdkMouse::isButtonDown(MouseButton button) const noexcept {
    return (_state.buttons & static_cast<uint32_t>(button)) != 0;
}

void GdkMouse::setCursorLocked(bool locked) {
    _locked = locked;
    // Note: Cursor locking/clipping would require platform-specific handling
    // using ClipCursor() on Windows. This implementation tracks the state only.
}

void GdkMouse::resetDeltas() {
    _state.delta_x = 0;
    _state.delta_y = 0;
    _state.scroll_delta_x = 0.0f;
    _state.scroll_delta_y = 0.0f;
}
