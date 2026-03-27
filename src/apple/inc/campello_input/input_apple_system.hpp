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

// Touch is only available on iOS/iPadOS/tvOS
#if TARGET_OS_IOS || TARGET_OS_TV
#include "touch_apple.hpp"
#endif

namespace systems::leal::campello_input {

    // AppleInputSystem - macOS/iOS/tvOS implementation of InputSystem
    //
    // This is an internal class. Applications should use InputSystem::create()
    // which returns this implementation on Apple platforms.
    class AppleInputSystem : public InputSystem {
        void *impl;   // CIAppleInputHelper* (CFBridgingRetain'd)

#if TARGET_OS_IOS || TARGET_OS_TV
        std::unique_ptr<AppleTouch> _touch;
#endif

    public:
        explicit AppleInputSystem(const PlatformContext& ctx);
        ~AppleInputSystem() override;

        // Disable copy/move
        AppleInputSystem(const AppleInputSystem&) = delete;
        AppleInputSystem& operator=(const AppleInputSystem&) = delete;

        // InputSystem interface
        void update() override;  // No-op on Apple (events are async)

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
    };

    // Backward compatibility alias (deprecated)
    using AppleInput [[deprecated("Use InputSystem::create() instead")]] = AppleInputSystem;

} // namespace systems::leal::campello_input
