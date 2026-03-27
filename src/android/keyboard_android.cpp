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

#include "inc/campello_input/keyboard_android.hpp"
#include <android/input.h>
#include <android/log.h>

using namespace systems::leal::campello_input;

// Map Android key codes to our KeyCode enum
// Android key codes are in android/keycodes.h
static KeyCode androidKeyCodeToKeyCode(int32_t keyCode) {
    switch (keyCode) {
        // Letters
        case AKEYCODE_A: return KeyCode::a;
        case AKEYCODE_B: return KeyCode::b;
        case AKEYCODE_C: return KeyCode::c;
        case AKEYCODE_D: return KeyCode::d;
        case AKEYCODE_E: return KeyCode::e;
        case AKEYCODE_F: return KeyCode::f;
        case AKEYCODE_G: return KeyCode::g;
        case AKEYCODE_H: return KeyCode::h;
        case AKEYCODE_I: return KeyCode::i;
        case AKEYCODE_J: return KeyCode::j;
        case AKEYCODE_K: return KeyCode::k;
        case AKEYCODE_L: return KeyCode::l;
        case AKEYCODE_M: return KeyCode::m;
        case AKEYCODE_N: return KeyCode::n;
        case AKEYCODE_O: return KeyCode::o;
        case AKEYCODE_P: return KeyCode::p;
        case AKEYCODE_Q: return KeyCode::q;
        case AKEYCODE_R: return KeyCode::r;
        case AKEYCODE_S: return KeyCode::s;
        case AKEYCODE_T: return KeyCode::t;
        case AKEYCODE_U: return KeyCode::u;
        case AKEYCODE_V: return KeyCode::v;
        case AKEYCODE_W: return KeyCode::w;
        case AKEYCODE_X: return KeyCode::x;
        case AKEYCODE_Y: return KeyCode::y;
        case AKEYCODE_Z: return KeyCode::z;

        // Numbers
        case AKEYCODE_0: return KeyCode::n0;
        case AKEYCODE_1: return KeyCode::n1;
        case AKEYCODE_2: return KeyCode::n2;
        case AKEYCODE_3: return KeyCode::n3;
        case AKEYCODE_4: return KeyCode::n4;
        case AKEYCODE_5: return KeyCode::n5;
        case AKEYCODE_6: return KeyCode::n6;
        case AKEYCODE_7: return KeyCode::n7;
        case AKEYCODE_8: return KeyCode::n8;
        case AKEYCODE_9: return KeyCode::n9;

        // Control keys
        case AKEYCODE_ENTER: return KeyCode::enter;
        case AKEYCODE_ESCAPE: return KeyCode::escape;
        case AKEYCODE_DEL: return KeyCode::backspace;  // DEL is backspace on Android
        case AKEYCODE_FORWARD_DEL: return KeyCode::del; // Forward delete
        case AKEYCODE_TAB: return KeyCode::tab;
        case AKEYCODE_SPACE: return KeyCode::space;

        // Modifiers
        case AKEYCODE_SHIFT_LEFT: return KeyCode::shift_left;
        case AKEYCODE_SHIFT_RIGHT: return KeyCode::shift_right;
        case AKEYCODE_CTRL_LEFT: return KeyCode::ctrl_left;
        case AKEYCODE_CTRL_RIGHT: return KeyCode::ctrl_right;
        case AKEYCODE_ALT_LEFT: return KeyCode::alt_left;
        case AKEYCODE_ALT_RIGHT: return KeyCode::alt_right;
        case AKEYCODE_META_LEFT: return KeyCode::meta_left;
        case AKEYCODE_META_RIGHT: return KeyCode::meta_right;
        case AKEYCODE_CAPS_LOCK: return KeyCode::caps_lock;

        // Function keys
        case AKEYCODE_F1: return KeyCode::f1;
        case AKEYCODE_F2: return KeyCode::f2;
        case AKEYCODE_F3: return KeyCode::f3;
        case AKEYCODE_F4: return KeyCode::f4;
        case AKEYCODE_F5: return KeyCode::f5;
        case AKEYCODE_F6: return KeyCode::f6;
        case AKEYCODE_F7: return KeyCode::f7;
        case AKEYCODE_F8: return KeyCode::f8;
        case AKEYCODE_F9: return KeyCode::f9;
        case AKEYCODE_F10: return KeyCode::f10;
        case AKEYCODE_F11: return KeyCode::f11;
        case AKEYCODE_F12: return KeyCode::f12;

        // Navigation
        case AKEYCODE_INSERT: return KeyCode::insert;
        case AKEYCODE_MOVE_HOME: return KeyCode::home;
        case AKEYCODE_PAGE_UP: return KeyCode::page_up;
        case AKEYCODE_MOVE_END: return KeyCode::end;
        case AKEYCODE_PAGE_DOWN: return KeyCode::page_down;
        case AKEYCODE_DPAD_RIGHT: return KeyCode::arrow_right;
        case AKEYCODE_DPAD_LEFT: return KeyCode::arrow_left;
        case AKEYCODE_DPAD_DOWN: return KeyCode::arrow_down;
        case AKEYCODE_DPAD_UP: return KeyCode::arrow_up;

        // Numpad
        case AKEYCODE_NUM_LOCK: return KeyCode::num_lock;
        case AKEYCODE_NUMPAD_DIVIDE: return KeyCode::numpad_divide;
        case AKEYCODE_NUMPAD_MULTIPLY: return KeyCode::numpad_multiply;
        case AKEYCODE_NUMPAD_SUBTRACT: return KeyCode::numpad_minus;
        case AKEYCODE_NUMPAD_ADD: return KeyCode::numpad_plus;
        case AKEYCODE_NUMPAD_ENTER: return KeyCode::numpad_enter;
        case AKEYCODE_NUMPAD_1: return KeyCode::numpad_1;
        case AKEYCODE_NUMPAD_2: return KeyCode::numpad_2;
        case AKEYCODE_NUMPAD_3: return KeyCode::numpad_3;
        case AKEYCODE_NUMPAD_4: return KeyCode::numpad_4;
        case AKEYCODE_NUMPAD_5: return KeyCode::numpad_5;
        case AKEYCODE_NUMPAD_6: return KeyCode::numpad_6;
        case AKEYCODE_NUMPAD_7: return KeyCode::numpad_7;
        case AKEYCODE_NUMPAD_8: return KeyCode::numpad_8;
        case AKEYCODE_NUMPAD_9: return KeyCode::numpad_9;
        case AKEYCODE_NUMPAD_0: return KeyCode::numpad_0;
        case AKEYCODE_NUMPAD_DOT: return KeyCode::numpad_decimal;

        // Symbols
        case AKEYCODE_MINUS: return KeyCode::minus;
        case AKEYCODE_EQUALS: return KeyCode::equals;
        case AKEYCODE_LEFT_BRACKET: return KeyCode::left_bracket;
        case AKEYCODE_RIGHT_BRACKET: return KeyCode::right_bracket;
        case AKEYCODE_BACKSLASH: return KeyCode::backslash;
        case AKEYCODE_SEMICOLON: return KeyCode::semicolon;
        case AKEYCODE_APOSTROPHE: return KeyCode::apostrophe;
        case AKEYCODE_GRAVE: return KeyCode::grave;
        case AKEYCODE_COMMA: return KeyCode::comma;
        case AKEYCODE_PERIOD: return KeyCode::period;
        case AKEYCODE_SLASH: return KeyCode::slash;

        default:
            return KeyCode::unknown;
    }
}

AndroidKeyboard::AndroidKeyboard(uint32_t id) : _id(id) {}

bool AndroidKeyboard::isKeyDown(KeyCode code) const noexcept {
    uint32_t idx = static_cast<uint32_t>(code);
    std::lock_guard<std::mutex> lock(stateMutex);
    return (currentState.keysDown[idx >> 6] >> (idx & 63)) & 1;
}

KeyModifier AndroidKeyboard::modifiers() const noexcept {
    std::lock_guard<std::mutex> lock(stateMutex);
    return currentState.modifiers;
}

void AndroidKeyboard::getState(KeyboardState& outState) const {
    std::lock_guard<std::mutex> lock(stateMutex);
    outState = currentState;
}

void AndroidKeyboard::onKeyEvent(int32_t keyCode, bool pressed) {
    KeyCode code = androidKeyCodeToKeyCode(keyCode);
    if (code == KeyCode::unknown) {
        // Log unmapped key for debugging
        __android_log_print(ANDROID_LOG_VERBOSE, "campello_input",
            "Unmapped key code: %d", keyCode);
        return;
    }

    uint32_t idx = static_cast<uint32_t>(code);
    
    std::lock_guard<std::mutex> lock(stateMutex);
    
    if (pressed) {
        currentState.keysDown[idx >> 6] |= 1ULL << (idx & 63);
    } else {
        currentState.keysDown[idx >> 6] &= ~(1ULL << (idx & 63));
    }

    // Update modifiers
    KeyModifier newMods = KeyModifier::none;
    if ((currentState.keysDown[static_cast<uint32_t>(KeyCode::shift_left) >> 6] >> 
         (static_cast<uint32_t>(KeyCode::shift_left) & 63)) & 1) {
        newMods = newMods | KeyModifier::shift;
    }
    if ((currentState.keysDown[static_cast<uint32_t>(KeyCode::shift_right) >> 6] >> 
         (static_cast<uint32_t>(KeyCode::shift_right) & 63)) & 1) {
        newMods = newMods | KeyModifier::shift;
    }
    if ((currentState.keysDown[static_cast<uint32_t>(KeyCode::ctrl_left) >> 6] >> 
         (static_cast<uint32_t>(KeyCode::ctrl_left) & 63)) & 1) {
        newMods = newMods | KeyModifier::ctrl;
    }
    if ((currentState.keysDown[static_cast<uint32_t>(KeyCode::ctrl_right) >> 6] >> 
         (static_cast<uint32_t>(KeyCode::ctrl_right) & 63)) & 1) {
        newMods = newMods | KeyModifier::ctrl;
    }
    if ((currentState.keysDown[static_cast<uint32_t>(KeyCode::alt_left) >> 6] >> 
         (static_cast<uint32_t>(KeyCode::alt_left) & 63)) & 1) {
        newMods = newMods | KeyModifier::alt;
    }
    if ((currentState.keysDown[static_cast<uint32_t>(KeyCode::alt_right) >> 6] >> 
         (static_cast<uint32_t>(KeyCode::alt_right) & 63)) & 1) {
        newMods = newMods | KeyModifier::alt;
    }
    if ((currentState.keysDown[static_cast<uint32_t>(KeyCode::meta_left) >> 6] >> 
         (static_cast<uint32_t>(KeyCode::meta_left) & 63)) & 1) {
        newMods = newMods | KeyModifier::meta;
    }
    if ((currentState.keysDown[static_cast<uint32_t>(KeyCode::meta_right) >> 6] >> 
         (static_cast<uint32_t>(KeyCode::meta_right) & 63)) & 1) {
        newMods = newMods | KeyModifier::meta;
    }
    if ((currentState.keysDown[static_cast<uint32_t>(KeyCode::caps_lock) >> 6] >> 
         (static_cast<uint32_t>(KeyCode::caps_lock) & 63)) & 1) {
        newMods = newMods | KeyModifier::caps_lock;
    }
    
    currentState.modifiers = newMods;
}

void AndroidKeyboard::clearState() {
    std::lock_guard<std::mutex> lock(stateMutex);
    currentState = KeyboardState{};
}
