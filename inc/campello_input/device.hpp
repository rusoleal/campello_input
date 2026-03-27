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

#include <cstdint>

namespace systems::leal::campello_input {

    enum class DeviceType : uint8_t {
        gamepad,
        keyboard,
        mouse,
        touch,
    };

    enum class ConnectionStatus : uint8_t {
        connected,
        disconnected,
    };

    // Device - base class for all input devices
    //
    // THREAD SAFETY:
    //   - Device methods are thread-safe for concurrent reads after the device
    //     has been connected (onDeviceConnected callback fired).
    //   - Device pointers become invalid after onDeviceDisconnected callback.
    //   - Do not store device pointers long-term; query from InputSystem each frame.
    class Device {
    public:
        virtual ~Device() = default;

        [[nodiscard]] virtual DeviceType type() const noexcept = 0;
        [[nodiscard]] virtual uint32_t id() const noexcept = 0;
        [[nodiscard]] virtual const char* name() const noexcept = 0;
        [[nodiscard]] virtual ConnectionStatus connectionStatus() const noexcept = 0;
    };

    // DeviceObserver — implement and register with InputSystem to receive
    // connect/disconnect notifications. Callbacks fire on the main/UI thread.
    class DeviceObserver {
    public:
        virtual ~DeviceObserver() = default;
        virtual void onDeviceConnected   (Device& device) = 0;
        virtual void onDeviceDisconnected(Device& device) = 0;
    };

}
