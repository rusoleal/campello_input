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

// Forward declaration for libevdev
struct libevdev;

namespace systems::leal::campello_input {

    // LinuxGamepad - Linux evdev-based gamepad implementation
    //
    // This is an internal class. Access via InputSystem::gamepadAt() on Linux.
    class LinuxGamepad : public GamepadDevice {
    public:
        LinuxGamepad(uint32_t id, const char* name, const char* devicePath);
        ~LinuxGamepad() override;

        // Disable copy/move
        LinuxGamepad(const LinuxGamepad&) = delete;
        LinuxGamepad& operator=(const LinuxGamepad&) = delete;

        // Initialize the device (open and set up mappings)
        // Returns true on success, false on failure
        bool initialize();

        // Update state from pending events
        // Called by LinuxInputSystem::update()
        void updateState();

        // Device interface
        DeviceType type() const noexcept override { return DeviceType::gamepad; }
        uint32_t id() const noexcept override { return _id; }
        const char* name() const noexcept override { return _name.c_str(); }
        ConnectionStatus connectionStatus() const noexcept override { return _status; }

        // GamepadDevice interface
        bool getState(GamepadState& outState) const override;
        Haptics* haptics() noexcept override { return nullptr; }  // TODO: FF support

        // Linux-specific accessors
        const char* devicePath() const { return _devicePath.c_str(); }
        int fd() const { return _fd; }

        // Set connection status (called on disconnect)
        void setConnectionStatus(ConnectionStatus status) { _status = status; }

    private:
        uint32_t _id;
        std::string _name;
        std::string _devicePath;
        int _fd = -1;
        libevdev* _evdev = nullptr;
        GamepadState _state{};
        ConnectionStatus _status = ConnectionStatus::connected;

        // Mapping configuration
        bool _hasAbsX = false, _hasAbsY = false;
        bool _hasAbsRX = false, _hasAbsRY = false;
        bool _hasAbsZ = false, _hasAbsRZ = false;
        bool _hasAbsHat0X = false, _hasAbsHat0Y = false;
        bool _hasAbsGas = false, _hasAbsBrake = false;

        // Normalize axis value based on min/max
        float normalizeAxis(int code, int value) const;
        
        // Set up axis/button mappings based on device capabilities
        void setupMappings();
    };

} // namespace systems::leal::campello_input
