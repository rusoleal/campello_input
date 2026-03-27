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

// Forward declaration for libevdev
struct libevdev;

namespace systems::leal::campello_input {

    // LinuxKeyboard - Linux evdev-based keyboard implementation
    //
    // This is an internal class. Access via InputSystem::keyboard() on Linux.
    class LinuxKeyboard : public KeyboardDevice {
    public:
        LinuxKeyboard(uint32_t id, const char* name, const char* devicePath);
        ~LinuxKeyboard() override;

        // Disable copy/move
        LinuxKeyboard(const LinuxKeyboard&) = delete;
        LinuxKeyboard& operator=(const LinuxKeyboard&) = delete;

        // Initialize the device
        bool initialize();

        // Update state from pending events
        void updateState();

        // Device interface
        DeviceType type() const noexcept override { return DeviceType::keyboard; }
        uint32_t id() const noexcept override { return _id; }
        const char* name() const noexcept override { return _name.c_str(); }
        ConnectionStatus connectionStatus() const noexcept override { return ConnectionStatus::connected; }

        // KeyboardDevice interface
        bool isKeyDown(KeyCode code) const noexcept override;
        KeyModifier modifiers() const noexcept override;
        void getState(KeyboardState& outState) const override;

        // Linux-specific accessors
        const char* devicePath() const { return _devicePath.c_str(); }
        int fd() const { return _fd; }

    private:
        uint32_t _id;
        std::string _name;
        std::string _devicePath;
        int _fd = -1;
        libevdev* _evdev = nullptr;
        KeyboardState _state{};

        // Convert Linux key code to our KeyCode
        static KeyCode linuxToKeyCode(int linuxCode);
    };

} // namespace systems::leal::campello_input
