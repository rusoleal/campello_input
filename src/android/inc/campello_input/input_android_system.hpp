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
#include <campello_input/gamepad.hpp>
#include <deque>
#include <string>

struct android_app;

namespace systems::leal::campello_input {

    // Forward declarations
    class AndroidGamepad;
    class AndroidTouch;

    // AndroidInputSystem - Android implementation of InputSystem
    //
    // This is an internal class. Applications should use InputSystem::create()
    // which returns this implementation on Android.
    //
    // Usage:
    //   PlatformContext ctx{.native = android_app_ptr};
    //   auto input = InputSystem::create(ctx);
    //   
    //   while (running) {
    //       input->update();  // Processes GameActivity input buffers
    //       // ... access devices
    //   }
    class AndroidInputSystem : public InputSystem {
        struct Impl;
        Impl *pImpl;

        android_app* app;
        std::unique_ptr<AndroidTouch> touchDevice;

    public:
        explicit AndroidInputSystem(const PlatformContext& ctx);
        ~AndroidInputSystem() override;

        // Disable copy/move
        AndroidInputSystem(const AndroidInputSystem&) = delete;
        AndroidInputSystem& operator=(const AndroidInputSystem&) = delete;

        // InputSystem interface
        void update() override;  // Processes GameActivity input buffers

        void addObserver(DeviceObserver *observer) override;
        void removeObserver(DeviceObserver *observer) override;

        int gamepadCount() const override;
        GamepadDevice* gamepadAt(int index) override;

        bool hasKeyboard() const override;
        KeyboardDevice* keyboard() override;

        bool hasMouse() const override;
        MouseDevice* mouse() override;

        bool hasTouch() const override;
        TouchDevice* touch() override;

        // Legacy API (deprecated) - kept for backward compatibility
        [[deprecated("Use InputSystem::update() instead")]]
        void processInputBuffers(android_app *pApp);

        [[deprecated("Use TouchDevice::getState() instead")]]
        const TouchState& touchState() const noexcept;

        [[deprecated("Keyboard implementation pending")]]
        const std::deque<std::string>& recentKeys() const noexcept;
    };

    // Backward compatibility alias (deprecated)
    using AndroidInput [[deprecated("Use InputSystem::create() instead")]] = AndroidInputSystem;

} // namespace systems::leal::campello_input
