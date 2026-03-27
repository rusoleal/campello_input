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

#import <GameController/GameController.h>
#include "inc/campello_input/keyboard_apple.hpp"

using namespace systems::leal::campello_input;

// GCKeyCode values are defined to match USB HID page 0x07, identical to our
// KeyCode enum values, so we can cast between them directly.

AppleKeyboard::AppleKeyboard(void *gcKeyboard) {
    GCKeyboard *kb = (__bridge GCKeyboard *)gcKeyboard;
    keyboard = (void *)CFBridgingRetain(kb);
}

AppleKeyboard::~AppleKeyboard() {
    if (keyboard) {
        CFBridgingRelease(keyboard);
        keyboard = nullptr;
    }
}

uint32_t AppleKeyboard::id() const noexcept {
    return 0;   // GCKeyboard is a singleton; fixed id
}

const char* AppleKeyboard::name() const noexcept {
    return "Keyboard";
}

ConnectionStatus AppleKeyboard::connectionStatus() const noexcept {
    if (@available(macOS 11.0, iOS 14.0, *)) {
        GCKeyboard *kb = (__bridge GCKeyboard *)keyboard;
        return kb ? ConnectionStatus::connected : ConnectionStatus::disconnected;
    }
    return ConnectionStatus::disconnected;
}

bool AppleKeyboard::isKeyDown(KeyCode code) const noexcept {
    if (@available(macOS 11.0, iOS 14.0, *)) {
        GCKeyboard *kb = (__bridge GCKeyboard *)keyboard;
        GCKeyboardInput *input = kb.keyboardInput;
        if (!input) return false;

        GCKeyCode gcCode = static_cast<GCKeyCode>(static_cast<uint32_t>(code));
        GCControllerButtonInput *btn = [input buttonForKeyCode:gcCode];
        return btn && btn.pressed;
    }
    return false;
}

KeyModifier AppleKeyboard::modifiers() const noexcept {
    KeyModifier mods = KeyModifier::none;

    auto isDown = [&](KeyCode code) -> bool { return isKeyDown(code); };

    if (isDown(KeyCode::shift_left)  || isDown(KeyCode::shift_right))  mods = mods | KeyModifier::shift;
    if (isDown(KeyCode::ctrl_left)   || isDown(KeyCode::ctrl_right))   mods = mods | KeyModifier::ctrl;
    if (isDown(KeyCode::alt_left)    || isDown(KeyCode::alt_right))    mods = mods | KeyModifier::alt;
    if (isDown(KeyCode::meta_left)   || isDown(KeyCode::meta_right))   mods = mods | KeyModifier::meta;
    if (isDown(KeyCode::caps_lock))                                     mods = mods | KeyModifier::caps_lock;

    return mods;
}

void* AppleKeyboard::nativeHandle() const noexcept {
    return keyboard;
}

void AppleKeyboard::getState(KeyboardState& outState) const {
    outState = {};

    if (@available(macOS 11.0, iOS 14.0, *)) {
        GCKeyboard *kb = (__bridge GCKeyboard *)keyboard;
        GCKeyboardInput *input = kb.keyboardInput;
        if (!input) return;

        // Scan all KeyCode values that fit in the 256-bit keysDown bitmap.
        // The enum covers 0x04–0xe7; we iterate the ranges rather than all 256.
        static const uint32_t ranges[][2] = {
            { 0x04, 0x27 },  // a–z, 1–0
            { 0x28, 0x64 },  // control keys, punctuation, F1–F12, numpad
            { 0xe0, 0xe7 },  // modifiers
        };

        for (auto &r : ranges) {
            for (uint32_t k = r[0]; k <= r[1]; ++k) {
                GCControllerButtonInput *btn = [input buttonForKeyCode:static_cast<GCKeyCode>(k)];
                if (btn && btn.pressed) {
                    outState.keysDown[k >> 6] |= 1ULL << (k & 63);
                }
            }
        }

        outState.modifiers = modifiers();
    }
}
