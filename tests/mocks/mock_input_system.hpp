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
#include <vector>
#include <memory>
#include <algorithm>

namespace systems::leal::campello_input {

    // MockInputSystem - A mock implementation of InputSystem for testing
    //
    // This class allows tests to programmatically control the input system
    // state without requiring actual hardware or platform-specific setup.
    class MockInputSystem : public InputSystem {
        std::vector<std::unique_ptr<GamepadDevice>> _gamepads;
        std::unique_ptr<KeyboardDevice> _keyboard;
        std::unique_ptr<MouseDevice> _mouse;
        std::unique_ptr<TouchDevice> _touch;
        std::vector<DeviceObserver*> _observers;
        bool _hasKeyboard = false;
        bool _hasMouse = false;
        bool _hasTouch = false;

    public:
        MockInputSystem() = default;

        // Add a gamepad to the system (fires connect notification)
        void addGamepad(std::unique_ptr<GamepadDevice> gamepad) {
            auto* ptr = gamepad.get();
            _gamepads.push_back(std::move(gamepad));
            for (auto* obs : _observers) {
                obs->onDeviceConnected(*ptr);
            }
        }

        // Remove a gamepad by ID (fires disconnect notification)
        void removeGamepad(uint32_t id) {
            auto it = std::find_if(_gamepads.begin(), _gamepads.end(),
                [id](const auto& gp) { return gp->id() == id; });
            if (it != _gamepads.end()) {
                for (auto* obs : _observers) {
                    obs->onDeviceDisconnected(*it->get());
                }
                _gamepads.erase(it);
            }
        }

        // Set keyboard
        void setKeyboard(std::unique_ptr<KeyboardDevice> keyboard) {
            if (_keyboard) {
                for (auto* obs : _observers) {
                    obs->onDeviceDisconnected(*_keyboard);
                }
            }
            _keyboard = std::move(keyboard);
            _hasKeyboard = (_keyboard != nullptr);
            if (_keyboard) {
                for (auto* obs : _observers) {
                    obs->onDeviceConnected(*_keyboard);
                }
            }
        }

        // Set mouse
        void setMouse(std::unique_ptr<MouseDevice> mouse) {
            if (_mouse) {
                for (auto* obs : _observers) {
                    obs->onDeviceDisconnected(*_mouse);
                }
            }
            _mouse = std::move(mouse);
            _hasMouse = (_mouse != nullptr);
            if (_mouse) {
                for (auto* obs : _observers) {
                    obs->onDeviceConnected(*_mouse);
                }
            }
        }

        // Set touch
        void setTouch(std::unique_ptr<TouchDevice> touch) {
            if (_touch) {
                for (auto* obs : _observers) {
                    obs->onDeviceDisconnected(*_touch);
                }
            }
            _touch = std::move(touch);
            _hasTouch = (_touch != nullptr);
            if (_touch) {
                for (auto* obs : _observers) {
                    obs->onDeviceConnected(*_touch);
                }
            }
        }

        // Clear all devices
        void clearAllDevices() {
            for (auto& gp : _gamepads) {
                for (auto* obs : _observers) {
                    obs->onDeviceDisconnected(*gp);
                }
            }
            _gamepads.clear();

            if (_keyboard) {
                for (auto* obs : _observers) {
                    obs->onDeviceDisconnected(*_keyboard);
                }
                _keyboard.reset();
                _hasKeyboard = false;
            }

            if (_mouse) {
                for (auto* obs : _observers) {
                    obs->onDeviceDisconnected(*_mouse);
                }
                _mouse.reset();
                _hasMouse = false;
            }

            if (_touch) {
                for (auto* obs : _observers) {
                    obs->onDeviceDisconnected(*_touch);
                }
                _touch.reset();
                _hasTouch = false;
            }
        }

        // InputSystem interface
        void update() override {
            // Mock update does nothing
        }

        void addObserver(DeviceObserver* observer) override {
            _observers.push_back(observer);
            // Immediately notify about existing devices
            for (auto& gp : _gamepads) {
                observer->onDeviceConnected(*gp);
            }
            if (_keyboard) observer->onDeviceConnected(*_keyboard);
            if (_mouse) observer->onDeviceConnected(*_mouse);
            if (_touch) observer->onDeviceConnected(*_touch);
        }

        void removeObserver(DeviceObserver* observer) override {
            auto it = std::find(_observers.begin(), _observers.end(), observer);
            if (it != _observers.end()) {
                _observers.erase(it);
            }
        }

        int gamepadCount() const override {
            return static_cast<int>(_gamepads.size());
        }

        GamepadDevice* gamepadAt(int index) override {
            if (index < 0 || index >= static_cast<int>(_gamepads.size())) {
                return nullptr;
            }
            return _gamepads[index].get();
        }

        bool hasKeyboard() const override { return _hasKeyboard; }

        KeyboardDevice* keyboard() override { return _keyboard.get(); }

        bool hasMouse() const override { return _hasMouse; }

        MouseDevice* mouse() override { return _mouse.get(); }

        bool hasTouch() const override { return _hasTouch; }

        TouchDevice* touch() override { return _touch.get(); }
    };

} // namespace systems::leal::campello_input
