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
#include <campello_input/haptics.hpp>
#include <string>
#include <memory>

// Forward declarations for GameInput
struct IGameInputDevice;
struct IGameInputReading;
struct IGameInputRumbleDevice;

namespace systems::leal::campello_input {

    // Forward declaration
    class GdkHaptics;

    // GdkGamepad - Windows/Xbox GDK GameInput gamepad implementation
    //
    // This is an internal class. Access via InputSystem::gamepadAt() on Windows/Xbox.
    class GdkGamepad : public GamepadDevice {
        uint32_t _id;
        std::string _name;
        IGameInputDevice* _device = nullptr;
        GamepadState _state{};
        ConnectionStatus _status = ConnectionStatus::connected;
        std::unique_ptr<GdkHaptics> _haptics;

    public:
        GdkGamepad(uint32_t id, IGameInputDevice* device);
        ~GdkGamepad() override;

        // Disable copy/move
        GdkGamepad(const GdkGamepad&) = delete;
        GdkGamepad& operator=(const GdkGamepad&) = delete;

        // Initialize the device
        bool initialize();

        // Update state from GameInput reading
        void updateState(IGameInputReading* reading);

        // Device interface
        DeviceType type() const noexcept override { return DeviceType::gamepad; }
        uint32_t id() const noexcept override { return _id; }
        const char* name() const noexcept override { return _name.c_str(); }
        ConnectionStatus connectionStatus() const noexcept override { return _status; }

        // GamepadDevice interface
        bool getState(GamepadState& outState) const override;
        Haptics* haptics() noexcept override;

        // GDK-specific accessors
        IGameInputDevice* device() const { return _device; }
        void setConnectionStatus(ConnectionStatus status) { _status = status; }
    };

} // namespace systems::leal::campello_input
