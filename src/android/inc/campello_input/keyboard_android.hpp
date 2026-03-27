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
#include <mutex>

namespace systems::leal::campello_input {

    // AndroidKeyboard - implements KeyboardDevice for Android
    //
    // This is an internal class. Access via InputSystem::keyboard() on Android.
    // Processes key events from GameActivity and maintains keyboard state.
    class AndroidKeyboard : public KeyboardDevice {
        mutable std::mutex stateMutex;
        KeyboardState currentState{};
        uint32_t _id;

    public:
        AndroidKeyboard(uint32_t id);
        ~AndroidKeyboard() override = default;

        // Disable copy/move
        AndroidKeyboard(const AndroidKeyboard&) = delete;
        AndroidKeyboard& operator=(const AndroidKeyboard&) = delete;

        // Device interface
        DeviceType type() const noexcept override { return DeviceType::keyboard; }
        uint32_t id() const noexcept override { return _id; }
        const char* name() const noexcept override { return "Android Keyboard"; }
        ConnectionStatus connectionStatus() const noexcept override { return ConnectionStatus::connected; }

        // KeyboardDevice interface
        bool isKeyDown(KeyCode code) const noexcept override;
        KeyModifier modifiers() const noexcept override;
        void getState(KeyboardState& outState) const override;

        // Called by AndroidInputSystem to process key events
        void onKeyEvent(int32_t keyCode, bool pressed);
        
        // Called when the keyboard loses focus (clear all keys)
        void clearState();
    };

} // namespace systems::leal::campello_input
