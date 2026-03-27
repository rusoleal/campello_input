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

// Forward declaration for GameInput
struct IGameInput;
struct IGameInputDevice;

namespace systems::leal::campello_input {

    // GdkInputSystem - Windows/Xbox GDK implementation using GameInput API
    //
    // This is an internal class. Applications should use InputSystem::create()
    // which returns this implementation on Windows/Xbox.
    //
    // Usage:
    //   auto input = InputSystem::create();  // No context needed for GDK
    //   
    //   while (running) {
    //       input->update();  // Polls GameInput for device state changes
    //       // ... access devices
    //   }
    class GdkInputSystem : public InputSystem {
        struct Impl;
        std::unique_ptr<Impl> pImpl;

    public:
        GdkInputSystem(const PlatformContext& ctx);
        ~GdkInputSystem() override;

        // Disable copy/move
        GdkInputSystem(const GdkInputSystem&) = delete;
        GdkInputSystem& operator=(const GdkInputSystem&) = delete;

        // InputSystem interface
        void update() override;
        void addObserver(DeviceObserver* observer) override;
        void removeObserver(DeviceObserver* observer) override;

        [[nodiscard]] int gamepadCount() const override;
        [[nodiscard]] GamepadDevice* gamepadAt(int index) override;

        [[nodiscard]] bool hasKeyboard() const override;
        [[nodiscard]] KeyboardDevice* keyboard() override;

        [[nodiscard]] bool hasMouse() const override;
        [[nodiscard]] MouseDevice* mouse() override;

        [[nodiscard]] bool hasTouch() const override { return false; }
        [[nodiscard]] TouchDevice* touch() override { return nullptr; }
    };

} // namespace systems::leal::campello_input
