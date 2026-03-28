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
#include <string>

using namespace systems::leal::campello_input;

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
    if (info && info->displayName && info->displayName->data) {
        _name = std::string(info->displayName->data, info->displayName->sizeInBytes);
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
    float newX = static_cast<float>(mouseState.positionX);
    float newY = static_cast<float>(mouseState.positionY);
    
    // Calculate delta
    if (_hasLastPos) {
        _state.deltaX = newX - _lastX;
        _state.deltaY = newY - _lastY;
    } else {
        _state.deltaX = 0;
        _state.deltaY = 0;
        _hasLastPos = true;
    }
    
    _lastX = newX;
    _lastY = newY;
    _state.x = newX;
    _state.y = newY;
    
    // Update wheel
    _state.scrollX = static_cast<float>(mouseState.wheelX);
    _state.scrollY = static_cast<float>(mouseState.wheelY);
    
    // Update buttons - GameInputMouseButtons uses bit flags
    // Map directly to our MouseButton bit positions
    _state.buttons = 0;
    uint32_t buttons = static_cast<uint32_t>(mouseState.buttons);
    
    if (buttons & GameInputMouseLeftButton) {
        _state.buttons |= (1u << static_cast<uint32_t>(MouseButton::left));
    }
    if (buttons & GameInputMouseRightButton) {
        _state.buttons |= (1u << static_cast<uint32_t>(MouseButton::right));
    }
    if (buttons & GameInputMouseMiddleButton) {
        _state.buttons |= (1u << static_cast<uint32_t>(MouseButton::middle));
    }
    if (buttons & GameInputMouseButton4) {
        _state.buttons |= (1u << static_cast<uint32_t>(MouseButton::extra1));
    }
    if (buttons & GameInputMouseButton5) {
        _state.buttons |= (1u << static_cast<uint32_t>(MouseButton::extra2));
    }
    
    // Update timestamp
    _state.timestamp = reading->GetTimestamp();
}

void GdkMouse::getState(MouseState& outState) const {
    outState = _state;
}

bool GdkMouse::isButtonDown(MouseButton button) const noexcept {
    return (_state.buttons & (1u << static_cast<uint32_t>(button))) != 0;
}

void GdkMouse::setCursorLocked(bool locked) {
    _locked = locked;
    // Note: Cursor locking/clipping would require platform-specific handling
    // using ClipCursor() on Windows. This implementation tracks the state only.
}

void GdkMouse::resetDeltas() {
    _state.deltaX = 0;
    _state.deltaY = 0;
    _state.scrollX = 0.0f;
    _state.scrollY = 0.0f;
}
