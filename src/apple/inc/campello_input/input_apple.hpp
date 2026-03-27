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

#include <campello_input/device.hpp>
#include <campello_input/gamepad.hpp>
#include <campello_input/keyboard.hpp>
#include <campello_input/mouse.hpp>

namespace systems::leal::campello_input {

    // AppleInput — discovers and manages GCController, GCKeyboard, GCMouse on
    // macOS / iOS / tvOS via NSNotificationCenter. Observer callbacks fire on
    // the main queue.
    //
    // Create and destroy on the main thread only.
    class AppleInput {
        void *impl;   // CIAppleInputHelper* (CFBridgingRetain'd)

    public:
        AppleInput();
        ~AppleInput();

        // Register / unregister an observer. Call on the main thread only.
        // The observer must outlive this AppleInput (or be unregistered first).
        // Registering fires onDeviceConnected immediately for all already-connected devices.
        void addObserver   (DeviceObserver *observer);
        void removeObserver(DeviceObserver *observer);

        // Current device access — valid on the main thread.
        int             gamepadCount()       const noexcept;
        GamepadDevice*  gamepadAt(int index) const noexcept;
        KeyboardDevice* keyboard()           const noexcept;
        MouseDevice*    mouse()              const noexcept;
    };

}
