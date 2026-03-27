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

    enum class MouseButton : uint8_t {
        left   = 0,
        right  = 1,
        middle = 2,
        extra1 = 3,
        extra2 = 4,
        count  = 5,
    };

    // -------------------------------------------------------------------------
    // MouseState — full snapshot (poll-friendly)
    //
    // - (x, y)           absolute position in pixels from top-left of window.
    // - (deltaX, deltaY) raw movement since the last poll, unaffected by
    //                    acceleration or clipping — useful for camera control.
    // - (scrollX, scrollY) scroll delta since last poll; units are platform-
    //                    dependent (lines or pixels depending on OS setting).
    // - buttons          bitmask: bit N corresponds to MouseButton(N).
    // -------------------------------------------------------------------------

    struct MouseState {
        uint64_t timestamp;   // microseconds
        float    x;
        float    y;
        float    deltaX;
        float    deltaY;
        float    scrollX;
        float    scrollY;
        uint8_t  buttons;     // bit N set = MouseButton(N) pressed
    };

    // -------------------------------------------------------------------------
    // MouseDevice
    // -------------------------------------------------------------------------

    class MouseDevice : public Device {
    public:
        DeviceType type() const noexcept override { return DeviceType::mouse; }

        virtual void getState(MouseState& outState) const = 0;
        [[nodiscard]] virtual bool isButtonDown(MouseButton button) const noexcept = 0;

        // When locked, the cursor is hidden and (x,y) stops updating;
        // deltaX/deltaY keep reporting raw movement for camera/look control.
        virtual void setCursorLocked(bool locked) = 0;
        [[nodiscard]] virtual bool isCursorLocked() const noexcept = 0;
    };

}
