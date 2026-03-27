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
#include <campello_input/touch.hpp>

#include <cstdint>
#include <deque>
#include <string>

struct android_app;

namespace systems::leal::campello_input {

    // AndroidGamepad — implements GamepadDevice for a gamepad discovered via
    // GameActivity input events. State is updated by AndroidInput each frame.
    class AndroidGamepad : public GamepadDevice {
        uint32_t     _id;
        int32_t      _androidDeviceId;
        char         _name[64];
        GamepadState _state{};

    public:
        AndroidGamepad(uint32_t id, int32_t androidDeviceId, const char *name);

        int32_t androidDeviceId() const noexcept { return _androidDeviceId; }

        // Called by AndroidInput to update state each frame.
        void updateState(const GamepadState &s) noexcept;

        DeviceType       type()               const noexcept override;
        uint32_t         id()                 const noexcept override;
        const char*      name()               const noexcept override;
        ConnectionStatus connectionStatus()   const noexcept override;
        bool             getState(GamepadState &outState) const override;
        Haptics*         haptics()            noexcept override;
    };

    // AndroidInput — processes GameActivity input buffers, discovers gamepads,
    // and fires DeviceObserver callbacks on new deviceIds.
    //
    // Usage from android_main:
    //   AndroidInput input;
    //   input.addObserver(&myObserver);
    //   // each frame:
    //   input.processInputBuffers(pApp);
    class AndroidInput {
        struct Impl;
        Impl *pImpl;

    public:
        AndroidInput();
        ~AndroidInput();

        void addObserver   (DeviceObserver *observer);
        void removeObserver(DeviceObserver *observer);

        // Process one frame of GameActivity input. Call from the android_main loop.
        void processInputBuffers(android_app *pApp);

        // Gamepad access — valid after processInputBuffers.
        int             gamepadCount()                  const noexcept;
        AndroidGamepad* gamepadAt(int index)            const noexcept;
        AndroidGamepad* findGamepad(int32_t androidId)  const noexcept;

        // Touch state — updated by processInputBuffers.
        const TouchState& touchState() const noexcept;

        // Most-recently pressed keyboard key names — updated by processInputBuffers.
        const std::deque<std::string>& recentKeys() const noexcept;
    };

} // namespace systems::leal::campello_input
