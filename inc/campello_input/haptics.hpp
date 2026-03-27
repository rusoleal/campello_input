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

namespace systems::leal::campello_input {

    // -------------------------------------------------------------------------
    // Rumble (dual-motor)
    // -------------------------------------------------------------------------

    struct RumbleEffect {
        float    lowFrequency;   // [0.0, 1.0] — large/low-frequency motor
        float    highFrequency;  // [0.0, 1.0] — small/high-frequency motor
        uint32_t durationMs;     // 0 = play indefinitely until stopRumble()
    };

    // -------------------------------------------------------------------------
    // Adaptive trigger resistance (e.g. PlayStation DualSense)
    // -------------------------------------------------------------------------

    enum class TriggerEffectType : uint8_t {
        off,         // no resistance
        feedback,    // constant resistance starting at a position
        weapon,      // click resistance at a position, then free
        vibration,   // oscillating resistance between two positions
    };

    struct TriggerEffect {
        TriggerEffectType type;
        float startPosition;  // [0.0, 1.0] — trigger travel where effect begins
        float endPosition;    // [0.0, 1.0] — trigger travel where effect ends (weapon/vibration)
        float strength;       // [0.0, 1.0]
        float frequency;      // Hz — used by vibration type
    };

    // -------------------------------------------------------------------------
    // Capability flags
    // -------------------------------------------------------------------------

    enum class HapticsCapability : uint32_t {
        none          = 0,
        rumble        = 1 << 0,
        trigger_left  = 1 << 1,
        trigger_right = 1 << 2,
    };

    inline HapticsCapability operator|(HapticsCapability a, HapticsCapability b) noexcept {
        return static_cast<HapticsCapability>(
            static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
    }

    inline bool operator&(HapticsCapability a, HapticsCapability b) noexcept {
        return (static_cast<uint32_t>(a) & static_cast<uint32_t>(b)) != 0;
    }

    // -------------------------------------------------------------------------
    // Haptics interface — obtained from GamepadDevice::haptics()
    // -------------------------------------------------------------------------

    class Haptics {
    public:
        virtual ~Haptics() = default;

        [[nodiscard]] virtual HapticsCapability capabilities() const noexcept = 0;

        // Rumble
        [[nodiscard]] virtual bool setRumble(const RumbleEffect& effect) = 0;
        [[nodiscard]] virtual bool stopRumble() = 0;

        // Adaptive triggers — leftTrigger=true for L2, false for R2
        [[nodiscard]] virtual bool setTriggerEffect(bool leftTrigger, const TriggerEffect& effect) = 0;
        [[nodiscard]] virtual bool stopTriggerEffect(bool leftTrigger) = 0;
    };

}
