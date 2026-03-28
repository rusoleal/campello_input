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

#include "inc/campello_input/keyboard_gdk.hpp"
#include <GameInput.h>
#include <cstring>
#include <string>

using namespace systems::leal::campello_input;

// Map GameInput scan codes to KeyCode
// GameInput uses USB HID usage page 0x07 (same as our KeyCode)
static KeyCode gameInputScanCodeToKeyCode(uint32_t scanCode) {
    switch (scanCode) {
        // Letters
        case 0x04: return KeyCode::a;
        case 0x05: return KeyCode::b;
        case 0x06: return KeyCode::c;
        case 0x07: return KeyCode::d;
        case 0x08: return KeyCode::e;
        case 0x09: return KeyCode::f;
        case 0x0A: return KeyCode::g;
        case 0x0B: return KeyCode::h;
        case 0x0C: return KeyCode::i;
        case 0x0D: return KeyCode::j;
        case 0x0E: return KeyCode::k;
        case 0x0F: return KeyCode::l;
        case 0x10: return KeyCode::m;
        case 0x11: return KeyCode::n;
        case 0x12: return KeyCode::o;
        case 0x13: return KeyCode::p;
        case 0x14: return KeyCode::q;
        case 0x15: return KeyCode::r;
        case 0x16: return KeyCode::s;
        case 0x17: return KeyCode::t;
        case 0x18: return KeyCode::u;
        case 0x19: return KeyCode::v;
        case 0x1A: return KeyCode::w;
        case 0x1B: return KeyCode::x;
        case 0x1C: return KeyCode::y;
        case 0x1D: return KeyCode::z;

        // Numbers
        case 0x1E: return KeyCode::n1;
        case 0x1F: return KeyCode::n2;
        case 0x20: return KeyCode::n3;
        case 0x21: return KeyCode::n4;
        case 0x22: return KeyCode::n5;
        case 0x23: return KeyCode::n6;
        case 0x24: return KeyCode::n7;
        case 0x25: return KeyCode::n8;
        case 0x26: return KeyCode::n9;
        case 0x27: return KeyCode::n0;

        // Control keys
        case 0x28: return KeyCode::enter;
        case 0x29: return KeyCode::escape;
        case 0x2A: return KeyCode::backspace;
        case 0x2B: return KeyCode::tab;
        case 0x2C: return KeyCode::space;

        // Modifiers
        case 0xE1: return KeyCode::shift_left;
        case 0xE5: return KeyCode::shift_right;
        case 0xE0: return KeyCode::ctrl_left;
        case 0xE4: return KeyCode::ctrl_right;
        case 0xE2: return KeyCode::alt_left;
        case 0xE6: return KeyCode::alt_right;
        case 0xE3: return KeyCode::meta_left;
        case 0xE7: return KeyCode::meta_right;
        case 0x39: return KeyCode::caps_lock;

        // Function keys
        case 0x3A: return KeyCode::f1;
        case 0x3B: return KeyCode::f2;
        case 0x3C: return KeyCode::f3;
        case 0x3D: return KeyCode::f4;
        case 0x3E: return KeyCode::f5;
        case 0x3F: return KeyCode::f6;
        case 0x40: return KeyCode::f7;
        case 0x41: return KeyCode::f8;
        case 0x42: return KeyCode::f9;
        case 0x43: return KeyCode::f10;
        case 0x44: return KeyCode::f11;
        case 0x45: return KeyCode::f12;

        // Navigation
        case 0x49: return KeyCode::insert;
        case 0x4A: return KeyCode::home;
        case 0x4B: return KeyCode::page_up;
        case 0x4C: return KeyCode::del;
        case 0x4D: return KeyCode::end;
        case 0x4E: return KeyCode::page_down;
        case 0x4F: return KeyCode::arrow_right;
        case 0x50: return KeyCode::arrow_left;
        case 0x51: return KeyCode::arrow_down;
        case 0x52: return KeyCode::arrow_up;

        // Numpad
        case 0x53: return KeyCode::num_lock;
        case 0x54: return KeyCode::numpad_divide;
        case 0x55: return KeyCode::numpad_multiply;
        case 0x56: return KeyCode::numpad_minus;
        case 0x57: return KeyCode::numpad_plus;
        case 0x58: return KeyCode::numpad_enter;
        case 0x59: return KeyCode::numpad_1;
        case 0x5A: return KeyCode::numpad_2;
        case 0x5B: return KeyCode::numpad_3;
        case 0x5C: return KeyCode::numpad_4;
        case 0x5D: return KeyCode::numpad_5;
        case 0x5E: return KeyCode::numpad_6;
        case 0x5F: return KeyCode::numpad_7;
        case 0x60: return KeyCode::numpad_8;
        case 0x61: return KeyCode::numpad_9;
        case 0x62: return KeyCode::numpad_0;
        case 0x63: return KeyCode::numpad_decimal;

        default:
            return KeyCode::unknown;
    }
}

GdkKeyboard::GdkKeyboard(uint32_t id, IGameInputDevice* device)
    : _id(id)
    , _device(device)
{
    if (_device) {
        _device->AddRef();
    }
}

GdkKeyboard::~GdkKeyboard() {
    if (_device) {
        _device->Release();
        _device = nullptr;
    }
}

bool GdkKeyboard::initialize() {
    if (!_device) return false;
    
    // Get device info for name
    const GameInputDeviceInfo* info = _device->GetDeviceInfo();
    if (info && info->displayName && info->displayName->data) {
        _name = std::string(info->displayName->data, info->displayName->sizeInBytes);
    } else {
        _name = "Keyboard " + std::to_string(_id);
    }
    
    return true;
}

void GdkKeyboard::updateState(IGameInputReading* reading) {
    if (!reading) return;
    
    // Clear current state before updating
    _state = KeyboardState{};
    
    // GameInput can return multiple key states in a single reading
    // GetKeyState returns the number of keys actually returned
    GameInputKeyState keyStates[16];
    uint32_t requestedCount = 16;
    
    uint32_t actualCount = reading->GetKeyState(requestedCount, keyStates);
    
    // Process all keys returned in the reading
    for (uint32_t i = 0; i < actualCount; ++i) {
        KeyCode code = gameInputScanCodeToKeyCode(keyStates[i].scanCode);
        if (code != KeyCode::unknown) {
            uint32_t idx = static_cast<uint32_t>(code);
            _state.keysDown[idx >> 6] |= 1ULL << (idx & 63);
            
            // Track modifiers based on key presses
            switch (code) {
                case KeyCode::shift_left:
                case KeyCode::shift_right:
                    _state.modifiers = _state.modifiers | KeyModifier::shift;
                    break;
                case KeyCode::ctrl_left:
                case KeyCode::ctrl_right:
                    _state.modifiers = _state.modifiers | KeyModifier::ctrl;
                    break;
                case KeyCode::alt_left:
                case KeyCode::alt_right:
                    _state.modifiers = _state.modifiers | KeyModifier::alt;
                    break;
                case KeyCode::meta_left:
                case KeyCode::meta_right:
                    _state.modifiers = _state.modifiers | KeyModifier::meta;
                    break;
                case KeyCode::caps_lock:
                    _state.modifiers = _state.modifiers | KeyModifier::caps_lock;
                    break;
                case KeyCode::num_lock:
                    _state.modifiers = _state.modifiers | KeyModifier::num_lock;
                    break;
                default:
                    break;
            }
        }
    }
}

bool GdkKeyboard::isKeyDown(KeyCode code) const noexcept {
    uint32_t idx = static_cast<uint32_t>(code);
    return (_state.keysDown[idx >> 6] >> (idx & 63)) & 1;
}

KeyModifier GdkKeyboard::modifiers() const noexcept {
    return _state.modifiers;
}

void GdkKeyboard::getState(KeyboardState& outState) const {
    outState = _state;
}
