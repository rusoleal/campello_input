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
#include <campello_input/controller_data.hpp>

namespace systems::leal::campello_input {

    // DEPRECATED: Use InputSystem::create() instead.
    // This legacy Manager class will be removed in a future version.
    // See input_system.hpp for the new unified cross-platform API.

    enum class ControllerStatus {
        connected,
        disconnected
    };

    typedef void (*ControllerStatusCallback)(const int32_t controllerIndex, const ControllerStatus status);

    class [[deprecated("Use InputSystem::create() instead. See input_system.hpp")]] Manager {
    private:
        void *native;

        ControllerStatusCallback controllersCallback;
        bool controllers[16];
        ControllerData controllerData[16];

        Manager(void *pd);
    public:

        ~Manager();
        
        void processNativeEvent(uint64_t cmd);

        void processData(void *pd);

        void setControllerStatusCallback(ControllerStatusCallback callback) {
            controllersCallback = callback;
        }

        bool getControllerData(const uint32_t index, ControllerData *data) {
            if (controllers[index]) {
                *data = controllerData[index];
                return true;
            }
            return false;
        }

        static std::shared_ptr<Manager> init(void *pd);

    };

}