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

#include <campello_input/touch.hpp>
#include <mutex>

namespace systems::leal::campello_input {

    // AndroidTouch - implements TouchDevice for Android touchscreen
    //
    // This is an internal class. Access via InputSystem::touch() on Android.
    class AndroidTouch : public TouchDevice {
        mutable std::mutex stateMutex;
        TouchState currentState{};

    public:
        AndroidTouch();
        ~AndroidTouch() override = default;

        // TouchDevice interface
        uint32_t id() const noexcept override;
        const char* name() const noexcept override;
        ConnectionStatus connectionStatus() const noexcept override;
        DeviceType type() const noexcept override { return DeviceType::touch; }

        uint32_t maxTouchPoints() const noexcept override;
        void getState(TouchState& outState) const override;
        bool triggerHapticPulse(float intensity) override;

        // Called by AndroidInputSystem to update touch state
        void updateState(const TouchState& state);
    };

} // namespace systems::leal::campello_input
