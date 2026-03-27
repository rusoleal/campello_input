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

#include <campello_input/gamepad.hpp>
#include <string>

namespace systems::leal::campello_input {

    // MockGamepad - A mock implementation of GamepadDevice for testing
    //
    // This class allows tests to programmatically control gamepad state
    // without requiring actual hardware.
    class MockGamepad : public GamepadDevice {
        uint32_t _id;
        std::string _name;
        GamepadState _state{};
        ConnectionStatus _status = ConnectionStatus::connected;

    public:
        MockGamepad(uint32_t id, const char* name)
            : _id(id), _name(name ? name : "Mock Gamepad") {}

        // Set the complete state of the gamepad
        void setState(const GamepadState& state) { _state = state; }

        // Set individual axis value
        void setAxis(GamepadAxis axis, float value) {
            _state.axes[static_cast<uint8_t>(axis)].value = value;
        }

        // Set button pressed state
        void setButton(GamepadButton button, bool pressed) {
            if (pressed) {
                _state.buttons |= static_cast<uint64_t>(button);
            } else {
                _state.buttons &= ~static_cast<uint64_t>(button);
            }
        }

        // Set timestamp
        void setTimestamp(uint64_t timestamp) { _state.timestamp = timestamp; }

        // Set connection status
        void setConnectionStatus(ConnectionStatus status) { _status = status; }

        // Get current state (for verification)
        const GamepadState& currentState() const { return _state; }

        // GamepadDevice interface
        DeviceType type() const noexcept override { return DeviceType::gamepad; }

        uint32_t id() const noexcept override { return _id; }

        const char* name() const noexcept override { return _name.c_str(); }

        ConnectionStatus connectionStatus() const noexcept override { return _status; }

        bool getState(GamepadState& outState) const override {
            outState = _state;
            return true;
        }

        Haptics* haptics() noexcept override {
            // Mock gamepad has no haptics
            return nullptr;
        }
    };

} // namespace systems::leal::campello_input
