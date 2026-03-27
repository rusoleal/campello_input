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

#include <campello_input/mouse.hpp>
#include <mutex>

namespace systems::leal::campello_input {

    // AndroidMouse - implements MouseDevice for Android
    //
    // This is an internal class. Access via InputSystem::mouse() on Android.
    // Processes mouse/touchpad events from GameActivity.
    // Note: Most Android devices don't have physical mice, but Chromebooks and
    // devices with USB/bluetooth mice are supported.
    class AndroidMouse : public MouseDevice {
        mutable std::mutex stateMutex;
        MouseState currentState{};
        uint32_t _id;
        bool _locked = false;

        // Accumulate deltas between getState() calls
        float _accumDeltaX = 0.0f;
        float _accumDeltaY = 0.0f;
        float _accumScrollX = 0.0f;
        float _accumScrollY = 0.0f;

    public:
        AndroidMouse(uint32_t id);
        ~AndroidMouse() override = default;

        // Disable copy/move
        AndroidMouse(const AndroidMouse&) = delete;
        AndroidMouse& operator=(const AndroidMouse&) = delete;

        // Device interface
        DeviceType type() const noexcept override { return DeviceType::mouse; }
        uint32_t id() const noexcept override { return _id; }
        const char* name() const noexcept override { return "Android Mouse"; }
        ConnectionStatus connectionStatus() const noexcept override { return ConnectionStatus::connected; }

        // MouseDevice interface
        void getState(MouseState& outState) const override;
        bool isButtonDown(MouseButton button) const noexcept override;
        void setCursorLocked(bool locked) override;
        bool isCursorLocked() const noexcept override { return _locked; }

        // Called by AndroidInputSystem to process motion events
        void onMotionEvent(float deltaX, float deltaY, float posX, float posY);
        void onScrollEvent(float deltaX, float deltaY);
        void onButtonEvent(MouseButton button, bool pressed);
        
        // Called after processing to reset deltas
        void resetAccumulators();
    };

} // namespace systems::leal::campello_input
