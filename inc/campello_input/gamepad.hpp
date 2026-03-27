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
#include <campello_input/haptics.hpp>

namespace systems::leal::campello_input {

    // -------------------------------------------------------------------------
    // Buttons — generic labels that map to physical positions across vendors:
    //   face_south  = A  / Cross    face_east   = B  / Circle
    //   face_west   = X  / Square   face_north  = Y  / Triangle
    // -------------------------------------------------------------------------

    enum class GamepadButton : uint64_t {
        face_south      = 1ULL << 0,
        face_east       = 1ULL << 1,
        face_west       = 1ULL << 2,
        face_north      = 1ULL << 3,
        shoulder_left   = 1ULL << 4,   // LB / L1
        shoulder_right  = 1ULL << 5,   // RB / R1
        thumb_left      = 1ULL << 6,   // L3 — left stick click
        thumb_right     = 1ULL << 7,   // R3 — right stick click
        dpad_up         = 1ULL << 8,
        dpad_down       = 1ULL << 9,
        dpad_left       = 1ULL << 10,
        dpad_right      = 1ULL << 11,
        start           = 1ULL << 12,
        select          = 1ULL << 13,  // Back / Options
        home            = 1ULL << 14,  // Guide / PS / Xbox button
        misc1           = 1ULL << 15,  // Share / Touchpad click / Capture
    };

    inline bool operator&(uint64_t mask, GamepadButton btn) noexcept {
        return (mask & static_cast<uint64_t>(btn)) != 0;
    }

    // -------------------------------------------------------------------------
    // Axes
    //   Sticks : [-1.0, 1.0],  +X = right,  +Y = down (matches screen coords)
    //   Triggers: [0.0, 1.0],  0.0 = released, 1.0 = fully pressed
    // -------------------------------------------------------------------------

    enum class GamepadAxis : uint8_t {
        left_x        = 0,
        left_y        = 1,
        right_x       = 2,
        right_y       = 3,
        trigger_left  = 4,
        trigger_right = 5,
        count         = 6,
    };

    struct GamepadAxisState {
        float value;     // normalized value within the range stated above
        float deadzone;  // effective dead-zone radius applied by the platform
    };

    // -------------------------------------------------------------------------
    // Snapshot returned by GamepadDevice::getState()
    // -------------------------------------------------------------------------

    struct GamepadState {
        uint64_t        timestamp;                                              // microseconds
        uint64_t        buttons;                                                // bitmask of GamepadButton
        GamepadAxisState axes[static_cast<uint8_t>(GamepadAxis::count)];
    };

    // -------------------------------------------------------------------------
    // GamepadDevice
    // -------------------------------------------------------------------------

    class GamepadDevice : public Device {
    public:
        DeviceType type() const noexcept override { return DeviceType::gamepad; }

        [[nodiscard]] virtual bool getState(GamepadState& outState) const = 0;

        // Returns nullptr when the device has no haptics hardware.
        [[nodiscard]] virtual Haptics* haptics() noexcept = 0;
    };

}
