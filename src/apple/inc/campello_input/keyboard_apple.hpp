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

#include <campello_input/keyboard.hpp>

namespace systems::leal::campello_input {

    // Wraps GCKeyboard (macOS 11+, iOS 14+).
    // GCKeyCode values match USB HID page 0x07, so KeyCode casts directly.
    class AppleKeyboard : public KeyboardDevice {
        void *keyboard;   // GCKeyboard*, CFBridgingRetain'd

    public:
        explicit AppleKeyboard(void *gcKeyboard);
        ~AppleKeyboard();

        uint32_t         id()               const noexcept override;
        const char*      name()             const noexcept override;
        ConnectionStatus connectionStatus() const noexcept override;

        void*       nativeHandle()          const noexcept;  // returns GCKeyboard*
        bool        isKeyDown(KeyCode code) const noexcept override;
        KeyModifier modifiers()             const noexcept override;
        void        getState(KeyboardState& outState) const override;
    };

}
