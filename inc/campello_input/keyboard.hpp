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

#pragma once

#include <cstdint>
#include <campello_input/device.hpp>

namespace systems::leal::campello_input {

    // -------------------------------------------------------------------------
    // KeyCode — physical key positions (layout-independent).
    // Values match USB HID Usage Page 0x07 so platform layers can map cheaply.
    // -------------------------------------------------------------------------

    enum class KeyCode : uint32_t {
        // Letters
        a = 0x04, b, c, d, e, f, g, h, i, j, k, l, m,
        n, o, p, q, r, s, t, u, v, w, x, y, z,

        // Top-row digits
        n1 = 0x1e, n2, n3, n4, n5, n6, n7, n8, n9, n0,

        // Control / whitespace
        enter      = 0x28,
        escape     = 0x29,
        backspace  = 0x2a,
        tab        = 0x2b,
        space      = 0x2c,

        // Punctuation
        minus          = 0x2d,
        equals         = 0x2e,
        bracket_left   = 0x2f,
        bracket_right  = 0x30,
        backslash      = 0x31,
        semicolon      = 0x33,
        apostrophe     = 0x34,
        grave          = 0x35,
        comma          = 0x36,
        period         = 0x37,
        slash          = 0x38,
        caps_lock      = 0x39,

        // Function keys
        f1 = 0x3a, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12,

        // Navigation cluster
        print_screen = 0x46,
        scroll_lock  = 0x47,
        pause        = 0x48,
        insert       = 0x49,
        home         = 0x4a,
        page_up      = 0x4b,
        del          = 0x4c,
        end          = 0x4d,
        page_down    = 0x4e,
        arrow_right  = 0x4f,
        arrow_left   = 0x50,
        arrow_down   = 0x51,
        arrow_up     = 0x52,

        // Numpad
        num_lock        = 0x53,
        numpad_divide   = 0x54,
        numpad_multiply = 0x55,
        numpad_minus    = 0x56,
        numpad_plus     = 0x57,
        numpad_enter    = 0x58,
        numpad_1        = 0x59,
        numpad_2        = 0x5a,
        numpad_3        = 0x5b,
        numpad_4        = 0x5c,
        numpad_5        = 0x5d,
        numpad_6        = 0x5e,
        numpad_7        = 0x5f,
        numpad_8        = 0x60,
        numpad_9        = 0x61,
        numpad_0        = 0x62,
        numpad_decimal  = 0x63,

        // Modifiers (0xe0–0xe7)
        ctrl_left   = 0xe0,
        shift_left  = 0xe1,
        alt_left    = 0xe2,
        meta_left   = 0xe3,   // Win / Cmd / Super
        ctrl_right  = 0xe4,
        shift_right = 0xe5,
        alt_right   = 0xe6,
        meta_right  = 0xe7,

        unknown = 0xffff,
    };

    // -------------------------------------------------------------------------
    // Modifier flags — logical state of modifier keys
    // -------------------------------------------------------------------------

    enum class KeyModifier : uint16_t {
        none      = 0,
        shift     = 1 << 0,
        ctrl      = 1 << 1,
        alt       = 1 << 2,
        meta      = 1 << 3,   // Win / Cmd / Super
        caps_lock = 1 << 4,
        num_lock  = 1 << 5,
    };

    inline KeyModifier operator|(KeyModifier a, KeyModifier b) noexcept {
        return static_cast<KeyModifier>(
            static_cast<uint16_t>(a) | static_cast<uint16_t>(b));
    }

    inline bool operator&(KeyModifier a, KeyModifier b) noexcept {
        return (static_cast<uint16_t>(a) & static_cast<uint16_t>(b)) != 0;
    }

    // -------------------------------------------------------------------------
    // KeyEvent — single press or release
    // -------------------------------------------------------------------------

    struct KeyEvent {
        uint64_t    timestamp;   // microseconds
        KeyCode     code;        // physical key (layout-independent)
        KeyModifier modifiers;   // modifier state at the time of the event
        uint32_t    unicode;     // UTF-32 code point; 0 if non-printable
        bool        pressed;     // true = key down, false = key up
        bool        repeat;      // true if generated by key-repeat, not a new press
    };

    // -------------------------------------------------------------------------
    // KeyboardState — full snapshot (poll-friendly)
    //
    // keysDown is a 256-bit array covering KeyCode values 0x00–0xff.
    // To test whether key k is down:
    //   auto idx = static_cast<uint32_t>(k);
    //   bool down = (keysDown[idx >> 6] >> (idx & 63)) & 1;
    // -------------------------------------------------------------------------

    struct KeyboardState {
        uint64_t    keysDown[4];   // bits 0–255, one per KeyCode value
        KeyModifier modifiers;
    };

    // -------------------------------------------------------------------------
    // KeyboardDevice
    // -------------------------------------------------------------------------

    class KeyboardDevice : public Device {
    public:
        DeviceType type() const noexcept override { return DeviceType::keyboard; }

        [[nodiscard]] virtual bool isKeyDown(KeyCode code) const noexcept = 0;
        [[nodiscard]] virtual KeyModifier modifiers() const noexcept = 0;
        virtual void getState(KeyboardState& outState) const = 0;
    };

}
