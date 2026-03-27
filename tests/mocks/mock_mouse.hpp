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
#include <string>

namespace systems::leal::campello_input {

    // MockMouse - A mock implementation of MouseDevice for testing
    class MockMouse : public MouseDevice {
        uint32_t _id;
        std::string _name;
        MouseState _state{};
        ConnectionStatus _status = ConnectionStatus::connected;
        bool _locked = false;

    public:
        MockMouse(uint32_t id, const char* name)
            : _id(id), _name(name ? name : "Mock Mouse") {}

        // Set position
        void setPosition(float x, float y) {
            _state.x = x;
            _state.y = y;
        }

        // Set delta
        void setDelta(float dx, float dy) {
            _state.deltaX = dx;
            _state.deltaY = dy;
        }

        // Set scroll
        void setScroll(float scrollX, float scrollY) {
            _state.scrollX = scrollX;
            _state.scrollY = scrollY;
        }

        // Set button state
        void setButtonDown(MouseButton button, bool pressed) {
            uint8_t bit = 1 << static_cast<uint8_t>(button);
            if (pressed) {
                _state.buttons |= bit;
            } else {
                _state.buttons &= ~bit;
            }
        }

        // Set timestamp
        void setTimestamp(uint64_t timestamp) { _state.timestamp = timestamp; }

        // Set connection status
        void setConnectionStatus(ConnectionStatus status) { _status = status; }

        // MouseDevice interface
        DeviceType type() const noexcept override { return DeviceType::mouse; }

        uint32_t id() const noexcept override { return _id; }

        const char* name() const noexcept override { return _name.c_str(); }

        ConnectionStatus connectionStatus() const noexcept override { return _status; }

        void getState(MouseState& outState) const override { outState = _state; }

        bool isButtonDown(MouseButton button) const noexcept override {
            return (_state.buttons & (1 << static_cast<uint8_t>(button))) != 0;
        }

        void setCursorLocked(bool locked) override { _locked = locked; }

        bool isCursorLocked() const noexcept override { return _locked; }
    };

} // namespace systems::leal::campello_input
