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

#include <memory>
#include <campello_input/device.hpp>
#include <campello_input/gamepad.hpp>
#include <campello_input/keyboard.hpp>
#include <campello_input/mouse.hpp>
#include <campello_input/touch.hpp>

namespace systems::leal::campello_input {

    // -------------------------------------------------------------------------
    // PlatformContext
    // -------------------------------------------------------------------------
    // Pass platform-specific initialization data to InputSystem::create().
    // Most platforms can use the default (empty) context. Android requires
    // the android_app pointer.
    //
    // Platform-specific values for native pointer:
    //   - Android: android_app*
    //   - Other platforms: nullptr (auto-detect)
    // -------------------------------------------------------------------------
    struct PlatformContext {
        void* native = nullptr;
    };

    // -------------------------------------------------------------------------
    // InputSystem
    // -------------------------------------------------------------------------
    // Unified cross-platform input system interface. Applications use this
    // single API regardless of target platform.
    //
    // THREAD SAFETY:
    //   - InputSystem is NOT thread-safe. All methods must be called from the
    //     same thread (typically the main/UI thread).
    //   - Device pointers (gamepadAt(), keyboard(), etc.) remain valid only
    //     until the next update() call on some platforms.
    //   - DeviceObserver callbacks are fired on the main/UI thread.
    //
    // Usage:
    //   auto input = InputSystem::create({.native = android_app_ptr}); // Android
    //   auto input = InputSystem::create(); // macOS, iOS, Windows, Linux
    //
    //   while (running) {
    //       input->update();  // Poll/process input events
    //       
    //       for (int i = 0; i < input->gamepadCount(); i++) {
    //           GamepadState state;
    //           if (input->gamepadAt(i)->getState(state)) {
    //               // Process gamepad input
    //           }
    //       }
    //   }
    // -------------------------------------------------------------------------
    class InputSystem {
    public:
        virtual ~InputSystem() = default;

        // ---------------------------------------------------------------------
        // Factory
        // ---------------------------------------------------------------------
        // Creates the platform-specific InputSystem implementation.
        // Returns nullptr if platform is not supported.
        // ---------------------------------------------------------------------
        [[nodiscard]] static std::unique_ptr<InputSystem> create(const PlatformContext& ctx = {});

        // ---------------------------------------------------------------------
        // Update
        // ---------------------------------------------------------------------
        // Call once per frame to update input state. On some platforms this
        // polls for new events (Android, Windows, Linux). On Apple platforms
        // this may be a no-op as events are delivered asynchronously.
        // ---------------------------------------------------------------------
        virtual void update() = 0;

        // ---------------------------------------------------------------------
        // Device Observation
        // ---------------------------------------------------------------------
        // Register to receive connect/disconnect notifications for all device
        // types. Callbacks are fired on the platform's main/UI thread.
        // The observer must outlive the InputSystem (or be unregistered first).
        // Registering fires onDeviceConnected immediately for already-connected
        // devices.
        // ---------------------------------------------------------------------
        virtual void addObserver(DeviceObserver* observer) = 0;
        virtual void removeObserver(DeviceObserver* observer) = 0;

        // ---------------------------------------------------------------------
        // Gamepad Access
        // ---------------------------------------------------------------------
        // Returns the number of currently connected gamepads.
        [[nodiscard]] virtual int gamepadCount() const = 0;

        // Returns the gamepad at the given index (0 to gamepadCount()-1).
        // Returns nullptr if index is out of range.
        [[nodiscard]] virtual GamepadDevice* gamepadAt(int index) = 0;

        // ---------------------------------------------------------------------
        // Keyboard Access
        // ---------------------------------------------------------------------
        // Returns true if a keyboard is available (connected or integrated).
        // On some platforms (desktop) this may always return true.
        [[nodiscard]] virtual bool hasKeyboard() const = 0;

        // Returns the keyboard device, or nullptr if hasKeyboard() is false.
        [[nodiscard]] virtual KeyboardDevice* keyboard() = 0;

        // ---------------------------------------------------------------------
        // Mouse Access
        // ---------------------------------------------------------------------
        // Returns true if a mouse is available (connected or integrated).
        // On some platforms (mobile) this may always return false.
        [[nodiscard]] virtual bool hasMouse() const = 0;

        // Returns the mouse device, or nullptr if hasMouse() is false.
        [[nodiscard]] virtual MouseDevice* mouse() = 0;

        // ---------------------------------------------------------------------
        // Touch Access
        // ---------------------------------------------------------------------
        // Returns true if touch input is available. Typically true on mobile
        // platforms (iOS, Android) and false on desktop platforms (macOS,
        // Windows, Linux).
        [[nodiscard]] virtual bool hasTouch() const = 0;

        // Returns the touch device, or nullptr if hasTouch() is false.
        [[nodiscard]] virtual TouchDevice* touch() = 0;
    };

} // namespace systems::leal::campello_input
