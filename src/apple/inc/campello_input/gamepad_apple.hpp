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
#include "haptics_apple.hpp"
#include <memory>
#include <string>

namespace systems::leal::campello_input {

    class AppleGamepad : public GamepadDevice {
        void        *controller;   // GCController*, CFBridgingRetain'd
        uint32_t     _id;
        std::string  _name;
        std::unique_ptr<AppleHaptics> _haptics;

    public:
        // gcController: GCController* (will be retained)
        AppleGamepad(uint32_t id, void *gcController);
        ~AppleGamepad();

        uint32_t         id()               const noexcept override;
        const char*      name()             const noexcept override;
        ConnectionStatus connectionStatus() const noexcept override;

        bool     getState(GamepadState& outState) const override;
        Haptics* haptics() noexcept override;
    };

}
