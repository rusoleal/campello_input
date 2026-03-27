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

#include <campello_input/keyboard.hpp>
#include <string>

namespace systems::leal::campello_input {

    // MockKeyboard - A mock implementation of KeyboardDevice for testing
    class MockKeyboard : public KeyboardDevice {
        uint32_t _id;
        std::string _name;
        KeyboardState _state{};
        ConnectionStatus _status = ConnectionStatus::connected;

    public:
        MockKeyboard(uint32_t id, const char* name)
            : _id(id), _name(name ? name : "Mock Keyboard") {}

        // Set key pressed state
        void setKeyDown(KeyCode code, bool pressed) {
            uint32_t idx = static_cast<uint32_t>(code);
            if (pressed) {
                _state.keysDown[idx >> 6] |= 1ULL << (idx & 63);
            } else {
                _state.keysDown[idx >> 6] &= ~(1ULL << (idx & 63));
            }
        }

        // Set modifier state
        void setModifiers(KeyModifier mods) { _state.modifiers = mods; }

        // Set connection status
        void setConnectionStatus(ConnectionStatus status) { _status = status; }

        // KeyboardDevice interface
        DeviceType type() const noexcept override { return DeviceType::keyboard; }

        uint32_t id() const noexcept override { return _id; }

        const char* name() const noexcept override { return _name.c_str(); }

        ConnectionStatus connectionStatus() const noexcept override { return _status; }

        bool isKeyDown(KeyCode code) const noexcept override {
            uint32_t idx = static_cast<uint32_t>(code);
            return (_state.keysDown[idx >> 6] >> (idx & 63)) & 1;
        }

        KeyModifier modifiers() const noexcept override { return _state.modifiers; }

        void getState(KeyboardState& outState) const override { outState = _state; }
    };

} // namespace systems::leal::campello_input
