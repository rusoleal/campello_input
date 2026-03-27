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

// Forward declarations for GameInput
struct IGameInputDevice;
struct IGameInputReading;

namespace systems::leal::campello_input {

    // GdkMouse - Windows/Xbox GDK GameInput mouse implementation
    //
    // This is an internal class. Access via InputSystem::mouse() on Windows/Xbox.
    class GdkMouse : public MouseDevice {
        uint32_t _id;
        std::string _name;
        IGameInputDevice* _device = nullptr;
        MouseState _state{};
        bool _locked = false;

        // Previous position for delta calculation
        int32_t _lastX = 0;
        int32_t _lastY = 0;
        bool _hasLastPos = false;

    public:
        GdkMouse(uint32_t id, IGameInputDevice* device);
        ~GdkMouse() override = default;

        // Disable copy/move
        GdkMouse(const GdkMouse&) = delete;
        GdkMouse& operator=(const GdkMouse&) = delete;

        // Initialize the device
        bool initialize();

        // Update state from GameInput reading
        void updateState(IGameInputReading* reading);

        // Device interface
        DeviceType type() const noexcept override { return DeviceType::mouse; }
        uint32_t id() const noexcept override { return _id; }
        const char* name() const noexcept override { return _name.c_str(); }
        ConnectionStatus connectionStatus() const noexcept override { return ConnectionStatus::connected; }

        // MouseDevice interface
        void getState(MouseState& outState) const override;
        bool isButtonDown(MouseButton button) const noexcept override;
        void setCursorLocked(bool locked) override;
        bool isCursorLocked() const noexcept override { return _locked; }

        // GDK-specific accessors
        IGameInputDevice* device() const { return _device; }
        
        // Reset deltas after reading
        void resetDeltas();
    };

} // namespace systems::leal::campello_input
