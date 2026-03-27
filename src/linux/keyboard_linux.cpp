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

#include "inc/campello_input/keyboard_linux.hpp"
#include <libevdev/libevdev.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/input.h>
#include <linux/input-event-codes.h>

using namespace systems::leal::campello_input;

// Linux key code to KeyCode mapping table
// Linux key codes are based on USB HID usage page 0x07, same as our KeyCode enum
// Most codes map directly (with offset), but some need special handling
static KeyCode linuxToKeyCodeMap[KEY_CNT];
static bool mappingInitialized = false;

static void initKeyCodeMapping() {
    if (mappingInitialized) return;
    
    // Initialize all to unknown
    for (int i = 0; i < KEY_CNT; ++i) {
        linuxToKeyCodeMap[i] = KeyCode::unknown;
    }

    // Letters (KEY_A = 30, but HID codes are sequential starting from 4)
    // Actually, Linux key codes are already HID codes, just with different base
    // Linux: KEY_A = 30, HID: 0x04 (a)
    // The mapping is: HID = Linux - 26 (approximately, but varies)
    
    // Direct mapping for most keys
    linuxToKeyCodeMap[KEY_A] = KeyCode::a;
    linuxToKeyCodeMap[KEY_B] = KeyCode::b;
    linuxToKeyCodeMap[KEY_C] = KeyCode::c;
    linuxToKeyCodeMap[KEY_D] = KeyCode::d;
    linuxToKeyCodeMap[KEY_E] = KeyCode::e;
    linuxToKeyCodeMap[KEY_F] = KeyCode::f;
    linuxToKeyCodeMap[KEY_G] = KeyCode::g;
    linuxToKeyCodeMap[KEY_H] = KeyCode::h;
    linuxToKeyCodeMap[KEY_I] = KeyCode::i;
    linuxToKeyCodeMap[KEY_J] = KeyCode::j;
    linuxToKeyCodeMap[KEY_K] = KeyCode::k;
    linuxToKeyCodeMap[KEY_L] = KeyCode::l;
    linuxToKeyCodeMap[KEY_M] = KeyCode::m;
    linuxToKeyCodeMap[KEY_N] = KeyCode::n;
    linuxToKeyCodeMap[KEY_O] = KeyCode::o;
    linuxToKeyCodeMap[KEY_P] = KeyCode::p;
    linuxToKeyCodeMap[KEY_Q] = KeyCode::q;
    linuxToKeyCodeMap[KEY_R] = KeyCode::r;
    linuxToKeyCodeMap[KEY_S] = KeyCode::s;
    linuxToKeyCodeMap[KEY_T] = KeyCode::t;
    linuxToKeyCodeMap[KEY_U] = KeyCode::u;
    linuxToKeyCodeMap[KEY_V] = KeyCode::v;
    linuxToKeyCodeMap[KEY_W] = KeyCode::w;
    linuxToKeyCodeMap[KEY_X] = KeyCode::x;
    linuxToKeyCodeMap[KEY_Y] = KeyCode::y;
    linuxToKeyCodeMap[KEY_Z] = KeyCode::z;

    // Numbers
    linuxToKeyCodeMap[KEY_1] = KeyCode::n1;
    linuxToKeyCodeMap[KEY_2] = KeyCode::n2;
    linuxToKeyCodeMap[KEY_3] = KeyCode::n3;
    linuxToKeyCodeMap[KEY_4] = KeyCode::n4;
    linuxToKeyCodeMap[KEY_5] = KeyCode::n5;
    linuxToKeyCodeMap[KEY_6] = KeyCode::n6;
    linuxToKeyCodeMap[KEY_7] = KeyCode::n7;
    linuxToKeyCodeMap[KEY_8] = KeyCode::n8;
    linuxToKeyCodeMap[KEY_9] = KeyCode::n9;
    linuxToKeyCodeMap[KEY_0] = KeyCode::n0;

    // Control keys
    linuxToKeyCodeMap[KEY_ENTER] = KeyCode::enter;
    linuxToKeyCodeMap[KEY_ESC] = KeyCode::escape;
    linuxToKeyCodeMap[KEY_BACKSPACE] = KeyCode::backspace;
    linuxToKeyCodeMap[KEY_TAB] = KeyCode::tab;
    linuxToKeyCodeMap[KEY_SPACE] = KeyCode::space;

    // Modifiers
    linuxToKeyCodeMap[KEY_LEFTSHIFT] = KeyCode::shift_left;
    linuxToKeyCodeMap[KEY_RIGHTSHIFT] = KeyCode::shift_right;
    linuxToKeyCodeMap[KEY_LEFTCTRL] = KeyCode::ctrl_left;
    linuxToKeyCodeMap[KEY_RIGHTCTRL] = KeyCode::ctrl_right;
    linuxToKeyCodeMap[KEY_LEFTALT] = KeyCode::alt_left;
    linuxToKeyCodeMap[KEY_RIGHTALT] = KeyCode::alt_right;
    linuxToKeyCodeMap[KEY_LEFTMETA] = KeyCode::meta_left;
    linuxToKeyCodeMap[KEY_RIGHTMETA] = KeyCode::meta_right;
    linuxToKeyCodeMap[KEY_CAPSLOCK] = KeyCode::caps_lock;

    // Function keys
    linuxToKeyCodeMap[KEY_F1] = KeyCode::f1;
    linuxToKeyCodeMap[KEY_F2] = KeyCode::f2;
    linuxToKeyCodeMap[KEY_F3] = KeyCode::f3;
    linuxToKeyCodeMap[KEY_F4] = KeyCode::f4;
    linuxToKeyCodeMap[KEY_F5] = KeyCode::f5;
    linuxToKeyCodeMap[KEY_F6] = KeyCode::f6;
    linuxToKeyCodeMap[KEY_F7] = KeyCode::f7;
    linuxToKeyCodeMap[KEY_F8] = KeyCode::f8;
    linuxToKeyCodeMap[KEY_F9] = KeyCode::f9;
    linuxToKeyCodeMap[KEY_F10] = KeyCode::f10;
    linuxToKeyCodeMap[KEY_F11] = KeyCode::f11;
    linuxToKeyCodeMap[KEY_F12] = KeyCode::f12;

    // Navigation
    linuxToKeyCodeMap[KEY_INSERT] = KeyCode::insert;
    linuxToKeyCodeMap[KEY_HOME] = KeyCode::home;
    linuxToKeyCodeMap[KEY_PAGEUP] = KeyCode::page_up;
    linuxToKeyCodeMap[KEY_DELETE] = KeyCode::del;
    linuxToKeyCodeMap[KEY_END] = KeyCode::end;
    linuxToKeyCodeMap[KEY_PAGEDOWN] = KeyCode::page_down;
    linuxToKeyCodeMap[KEY_RIGHT] = KeyCode::arrow_right;
    linuxToKeyCodeMap[KEY_LEFT] = KeyCode::arrow_left;
    linuxToKeyCodeMap[KEY_DOWN] = KeyCode::arrow_down;
    linuxToKeyCodeMap[KEY_UP] = KeyCode::arrow_up;

    // Numpad
    linuxToKeyCodeMap[KEY_NUMLOCK] = KeyCode::num_lock;
    linuxToKeyCodeMap[KEY_KPSLASH] = KeyCode::numpad_divide;
    linuxToKeyCodeMap[KEY_KPASTERISK] = KeyCode::numpad_multiply;
    linuxToKeyCodeMap[KEY_KPMINUS] = KeyCode::numpad_minus;
    linuxToKeyCodeMap[KEY_KPPLUS] = KeyCode::numpad_plus;
    linuxToKeyCodeMap[KEY_KPENTER] = KeyCode::numpad_enter;
    linuxToKeyCodeMap[KEY_KP1] = KeyCode::numpad_1;
    linuxToKeyCodeMap[KEY_KP2] = KeyCode::numpad_2;
    linuxToKeyCodeMap[KEY_KP3] = KeyCode::numpad_3;
    linuxToKeyCodeMap[KEY_KP4] = KeyCode::numpad_4;
    linuxToKeyCodeMap[KEY_KP5] = KeyCode::numpad_5;
    linuxToKeyCodeMap[KEY_KP6] = KeyCode::numpad_6;
    linuxToKeyCodeMap[KEY_KP7] = KeyCode::numpad_7;
    linuxToKeyCodeMap[KEY_KP8] = KeyCode::numpad_8;
    linuxToKeyCodeMap[KEY_KP9] = KeyCode::numpad_9;
    linuxToKeyCodeMap[KEY_KP0] = KeyCode::numpad_0;
    linuxToKeyCodeMap[KEY_KPDOT] = KeyCode::numpad_decimal;

    mappingInitialized = true;
}

LinuxKeyboard::LinuxKeyboard(uint32_t id, const char* name, const char* devicePath)
    : _id(id)
    , _name(name ? name : "Unknown Keyboard")
    , _devicePath(devicePath ? devicePath : "")
{
    initKeyCodeMapping();
}

LinuxKeyboard::~LinuxKeyboard() {
    if (_evdev) {
        libevdev_free(_evdev);
        _evdev = nullptr;
    }
    if (_fd >= 0) {
        close(_fd);
        _fd = -1;
    }
}

bool LinuxKeyboard::initialize() {
    _fd = open(_devicePath.c_str(), O_RDONLY | O_NONBLOCK);
    if (_fd < 0) {
        return false;
    }

    int rc = libevdev_new_from_fd(_fd, &_evdev);
    if (rc < 0) {
        close(_fd);
        _fd = -1;
        return false;
    }

    // Check if this is actually a keyboard
    if (!libevdev_has_event_type(_evdev, EV_KEY)) {
        libevdev_free(_evdev);
        _evdev = nullptr;
        close(_fd);
        _fd = -1;
        return false;
    }

    // Update name from device
    const char* devName = libevdev_get_name(_evdev);
    if (devName) {
        _name = devName;
    }

    return true;
}

void LinuxKeyboard::updateState() {
    if (!_evdev) return;

    struct input_event ev;
    int rc;

    while ((rc = libevdev_next_event(_evdev, LIBEVDEV_READ_FLAG_NORMAL, &ev)) == 0) {
        if (ev.type == EV_KEY) {
            KeyCode code = linuxToKeyCodeMap[ev.code];
            if (code != KeyCode::unknown) {
                uint32_t idx = static_cast<uint32_t>(code);
                if (ev.value) {  // Pressed
                    _state.keysDown[idx >> 6] |= 1ULL << (idx & 63);
                } else {  // Released
                    _state.keysDown[idx >> 6] &= ~(1ULL << (idx & 63));
                }
            }

            // Update modifier state
            KeyModifier newMods = KeyModifier::none;
            if (isKeyDown(KeyCode::shift_left) || isKeyDown(KeyCode::shift_right)) {
                newMods = newMods | KeyModifier::shift;
            }
            if (isKeyDown(KeyCode::ctrl_left) || isKeyDown(KeyCode::ctrl_right)) {
                newMods = newMods | KeyModifier::ctrl;
            }
            if (isKeyDown(KeyCode::alt_left) || isKeyDown(KeyCode::alt_right)) {
                newMods = newMods | KeyModifier::alt;
            }
            if (isKeyDown(KeyCode::meta_left) || isKeyDown(KeyCode::meta_right)) {
                newMods = newMods | KeyModifier::meta;
            }
            if (isKeyDown(KeyCode::caps_lock)) {
                newMods = newMods | KeyModifier::caps_lock;
            }
            if (isKeyDown(KeyCode::num_lock)) {
                newMods = newMods | KeyModifier::num_lock;
            }
            _state.modifiers = newMods;
        }
    }
}

bool LinuxKeyboard::isKeyDown(KeyCode code) const noexcept {
    uint32_t idx = static_cast<uint32_t>(code);
    return (_state.keysDown[idx >> 6] >> (idx & 63)) & 1;
}

KeyModifier LinuxKeyboard::modifiers() const noexcept {
    return _state.modifiers;
}

void LinuxKeyboard::getState(KeyboardState& outState) const {
    outState = _state;
}
