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

#include <campello_input/input_system.hpp>
#include <thread>
#include <atomic>
#include <vector>
#include <memory>
#include <mutex>
#include <queue>
#include <functional>

// Forward declarations for libevdev and libudev
struct libevdev;
struct udev;
struct udev_monitor;
struct udev_device;

namespace systems::leal::campello_input {

    // Forward declarations
    class LinuxGamepad;
    class LinuxKeyboard;
    class LinuxMouse;

    // LinuxInputSystem - Linux implementation using evdev/udev
    //
    // This is an internal class. Applications should use InputSystem::create()
    // which returns this implementation on Linux platforms.
    //
    // Uses libevdev for device access and libudev for hot-plug detection.
    // Runs a background thread for event monitoring via epoll.
    class LinuxInputSystem : public InputSystem {
    public:
        explicit LinuxInputSystem(const PlatformContext& ctx);
        ~LinuxInputSystem() override;

        // Disable copy/move
        LinuxInputSystem(const LinuxInputSystem&) = delete;
        LinuxInputSystem& operator=(const LinuxInputSystem&) = delete;

        // InputSystem interface
        void update() override;
        void addObserver(DeviceObserver* observer) override;
        void removeObserver(DeviceObserver* observer) override;

        int gamepadCount() const override;
        GamepadDevice* gamepadAt(int index) override;

        bool hasKeyboard() const override;
        KeyboardDevice* keyboard() override;

        bool hasMouse() const override;
        MouseDevice* mouse() override;

        bool hasTouch() const override { return false; }
        TouchDevice* touch() override { return nullptr; }

    private:
        // Internal implementation (PIMPL idiom)
        struct Impl;
        std::unique_ptr<Impl> pImpl;
    };

} // namespace systems::leal::campello_input
