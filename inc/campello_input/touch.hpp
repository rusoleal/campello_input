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

    enum class TouchPhase : uint8_t {
        began,       // finger first touches the surface
        moved,       // finger moved since last event
        stationary,  // finger is touching but has not moved
        ended,       // finger lifted
        cancelled,   // touch was cancelled by the system (e.g. incoming call)
    };

    struct TouchPoint {
        uint32_t   id;        // stable identifier across began → ended lifecycle
        TouchPhase phase;
        float      x;         // position in pixels from top-left of window
        float      y;
        float      deltaX;    // movement since last event
        float      deltaY;
        float      pressure;  // [0.0, 1.0]; 1.0 when hardware does not report pressure
        float      radius;    // contact area radius in pixels; 0 when not supported
        uint64_t   timestamp; // microseconds
    };

    static constexpr uint32_t kMaxTouchPoints = 10;

    struct TouchState {
        TouchPoint points[kMaxTouchPoints];
        uint32_t   count;     // number of active touch points
    };

    // -------------------------------------------------------------------------
    // TouchDevice
    // -------------------------------------------------------------------------

    class TouchDevice : public Device {
    public:
        DeviceType type() const noexcept override { return DeviceType::touch; }

        // Maximum simultaneous touch points the hardware can track.
        [[nodiscard]] virtual uint32_t maxTouchPoints() const noexcept = 0;

        virtual void getState(TouchState& outState) const = 0;

        // Short haptic pulse — e.g. phone vibration on UI interaction.
        // intensity: [0.0, 1.0]. Returns false if not supported.
        [[nodiscard]] virtual bool triggerHapticPulse(float intensity) = 0;
    };

}
