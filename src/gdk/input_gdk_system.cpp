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

#include "inc/campello_input/input_gdk_system.hpp"
#include "inc/campello_input/gamepad_gdk.hpp"
#include "inc/campello_input/keyboard_gdk.hpp"
#include "inc/campello_input/mouse_gdk.hpp"

#include <campello_input/device.hpp>

#include <GameInput.h>
#include <algorithm>
#include <cstring>
#include <vector>

using namespace systems::leal::campello_input;

// ---------------------------------------------------------------------------
// Virtual keyboard device that uses aggregate input
// ---------------------------------------------------------------------------
class AggregateKeyboard : public KeyboardDevice {
    uint32_t _id;
    std::string _name;
    KeyboardState _state{};
    IGameInput* _gameInput = nullptr;

public:
    AggregateKeyboard(uint32_t id, IGameInput* gameInput)
        : _id(id)
        , _name("System Keyboard")
        , _gameInput(gameInput)
    {
        if (_gameInput) {
            _gameInput->AddRef();
        }
    }

    ~AggregateKeyboard() override {
        if (_gameInput) {
            _gameInput->Release();
        }
    }

    // Device interface
    DeviceType type() const noexcept override { return DeviceType::keyboard; }
    uint32_t id() const noexcept override { return _id; }
    const char* name() const noexcept override { return _name.c_str(); }
    ConnectionStatus connectionStatus() const noexcept override { return ConnectionStatus::connected; }

    // KeyboardDevice interface
    bool isKeyDown(KeyCode code) const noexcept override {
        uint32_t idx = static_cast<uint32_t>(code);
        return (_state.keysDown[idx >> 6] >> (idx & 63)) & 1;
    }

    KeyModifier modifiers() const noexcept override {
        return _state.modifiers;
    }

    void getState(KeyboardState& outState) const override {
        outState = _state;
    }

    // Update from GameInput aggregate
    void update() {
        if (!_gameInput) return;

        // Clear state for this frame
        _state = KeyboardState{};

        // Get current reading from aggregate (any keyboard)
        IGameInputReading* reading = nullptr;
        HRESULT hr = _gameInput->GetCurrentReading(GameInputKindKeyboard, nullptr, &reading);
        
        if (SUCCEEDED(hr) && reading) {
            GameInputKeyState keyStates[16];
            uint32_t requestedCount = 16;
            uint32_t actualCount = reading->GetKeyState(requestedCount, keyStates);
            
            for (uint32_t i = 0; i < actualCount; ++i) {
                KeyCode code = scanCodeToKeyCode(keyStates[i].scanCode);
                if (code != KeyCode::unknown) {
                    uint32_t idx = static_cast<uint32_t>(code);
                    _state.keysDown[idx >> 6] |= 1ULL << (idx & 63);
                    
                    // Track modifiers
                    updateModifiers(code);
                }
            }
            reading->Release();
        }
    }

private:
    static KeyCode scanCodeToKeyCode(uint32_t scanCode) {
        switch (scanCode) {
            case 0x04: return KeyCode::a;
            case 0x05: return KeyCode::b;
            case 0x06: return KeyCode::c;
            case 0x07: return KeyCode::d;
            case 0x08: return KeyCode::e;
            case 0x09: return KeyCode::f;
            case 0x0A: return KeyCode::g;
            case 0x0B: return KeyCode::h;
            case 0x0C: return KeyCode::i;
            case 0x0D: return KeyCode::j;
            case 0x0E: return KeyCode::k;
            case 0x0F: return KeyCode::l;
            case 0x10: return KeyCode::m;
            case 0x11: return KeyCode::n;
            case 0x12: return KeyCode::o;
            case 0x13: return KeyCode::p;
            case 0x14: return KeyCode::q;
            case 0x15: return KeyCode::r;
            case 0x16: return KeyCode::s;
            case 0x17: return KeyCode::t;
            case 0x18: return KeyCode::u;
            case 0x19: return KeyCode::v;
            case 0x1A: return KeyCode::w;
            case 0x1B: return KeyCode::x;
            case 0x1C: return KeyCode::y;
            case 0x1D: return KeyCode::z;
            case 0x1E: return KeyCode::n1;
            case 0x1F: return KeyCode::n2;
            case 0x20: return KeyCode::n3;
            case 0x21: return KeyCode::n4;
            case 0x22: return KeyCode::n5;
            case 0x23: return KeyCode::n6;
            case 0x24: return KeyCode::n7;
            case 0x25: return KeyCode::n8;
            case 0x26: return KeyCode::n9;
            case 0x27: return KeyCode::n0;
            case 0x28: return KeyCode::enter;
            case 0x29: return KeyCode::escape;
            case 0x2A: return KeyCode::backspace;
            case 0x2B: return KeyCode::tab;
            case 0x2C: return KeyCode::space;
            case 0xE1: return KeyCode::shift_left;
            case 0xE5: return KeyCode::shift_right;
            case 0xE0: return KeyCode::ctrl_left;
            case 0xE4: return KeyCode::ctrl_right;
            case 0xE2: return KeyCode::alt_left;
            case 0xE6: return KeyCode::alt_right;
            case 0xE3: return KeyCode::meta_left;
            case 0xE7: return KeyCode::meta_right;
            case 0x39: return KeyCode::caps_lock;
            case 0x3A: return KeyCode::f1;
            case 0x3B: return KeyCode::f2;
            case 0x3C: return KeyCode::f3;
            case 0x3D: return KeyCode::f4;
            case 0x3E: return KeyCode::f5;
            case 0x3F: return KeyCode::f6;
            case 0x40: return KeyCode::f7;
            case 0x41: return KeyCode::f8;
            case 0x42: return KeyCode::f9;
            case 0x43: return KeyCode::f10;
            case 0x44: return KeyCode::f11;
            case 0x45: return KeyCode::f12;
            case 0x49: return KeyCode::insert;
            case 0x4A: return KeyCode::home;
            case 0x4B: return KeyCode::page_up;
            case 0x4C: return KeyCode::del;
            case 0x4D: return KeyCode::end;
            case 0x4E: return KeyCode::page_down;
            case 0x4F: return KeyCode::arrow_right;
            case 0x50: return KeyCode::arrow_left;
            case 0x51: return KeyCode::arrow_down;
            case 0x52: return KeyCode::arrow_up;
            case 0x53: return KeyCode::num_lock;
            case 0x54: return KeyCode::numpad_divide;
            case 0x55: return KeyCode::numpad_multiply;
            case 0x56: return KeyCode::numpad_minus;
            case 0x57: return KeyCode::numpad_plus;
            case 0x58: return KeyCode::numpad_enter;
            case 0x59: return KeyCode::numpad_1;
            case 0x5A: return KeyCode::numpad_2;
            case 0x5B: return KeyCode::numpad_3;
            case 0x5C: return KeyCode::numpad_4;
            case 0x5D: return KeyCode::numpad_5;
            case 0x5E: return KeyCode::numpad_6;
            case 0x5F: return KeyCode::numpad_7;
            case 0x60: return KeyCode::numpad_8;
            case 0x61: return KeyCode::numpad_9;
            case 0x62: return KeyCode::numpad_0;
            case 0x63: return KeyCode::numpad_decimal;
            default: return KeyCode::unknown;
        }
    }

    void updateModifiers(KeyCode code) {
        switch (code) {
            case KeyCode::shift_left:
            case KeyCode::shift_right:
                _state.modifiers = _state.modifiers | KeyModifier::shift;
                break;
            case KeyCode::ctrl_left:
            case KeyCode::ctrl_right:
                _state.modifiers = _state.modifiers | KeyModifier::ctrl;
                break;
            case KeyCode::alt_left:
            case KeyCode::alt_right:
                _state.modifiers = _state.modifiers | KeyModifier::alt;
                break;
            case KeyCode::meta_left:
            case KeyCode::meta_right:
                _state.modifiers = _state.modifiers | KeyModifier::meta;
                break;
            case KeyCode::caps_lock:
                _state.modifiers = _state.modifiers | KeyModifier::caps_lock;
                break;
            case KeyCode::num_lock:
                _state.modifiers = _state.modifiers | KeyModifier::num_lock;
                break;
            default:
                break;
        }
    }
};

// ---------------------------------------------------------------------------
// Virtual mouse device that uses aggregate input
// ---------------------------------------------------------------------------
class AggregateMouse : public MouseDevice {
    uint32_t _id;
    std::string _name;
    MouseState _state{};
    IGameInput* _gameInput = nullptr;
    bool _locked = false;
    float _lastX = 0;
    float _lastY = 0;
    bool _hasLastPos = false;

public:
    AggregateMouse(uint32_t id, IGameInput* gameInput)
        : _id(id)
        , _name("System Mouse")
        , _gameInput(gameInput)
    {
        if (_gameInput) {
            _gameInput->AddRef();
        }
    }

    ~AggregateMouse() override {
        if (_gameInput) {
            _gameInput->Release();
        }
    }

    // Device interface
    DeviceType type() const noexcept override { return DeviceType::mouse; }
    uint32_t id() const noexcept override { return _id; }
    const char* name() const noexcept override { return _name.c_str(); }
    ConnectionStatus connectionStatus() const noexcept override { return ConnectionStatus::connected; }

    // MouseDevice interface
    void getState(MouseState& outState) const override {
        outState = _state;
    }

    bool isButtonDown(MouseButton button) const noexcept override {
        return (_state.buttons & (1u << static_cast<uint32_t>(button))) != 0;
    }

    void setCursorLocked(bool locked) override {
        _locked = locked;
    }

    bool isCursorLocked() const noexcept override {
        return _locked;
    }

    // Update from GameInput aggregate
    void update() {
        if (!_gameInput) return;

        // Get current reading from aggregate (any mouse)
        IGameInputReading* reading = nullptr;
        HRESULT hr = _gameInput->GetCurrentReading(GameInputKindMouse, nullptr, &reading);
        
        if (SUCCEEDED(hr) && reading) {
            GameInputMouseState mouseState;
            if (reading->GetMouseState(&mouseState)) {
                // Update position
                float newX = static_cast<float>(mouseState.positionX);
                float newY = static_cast<float>(mouseState.positionY);
                
                // Calculate delta
                if (_hasLastPos) {
                    _state.deltaX = newX - _lastX;
                    _state.deltaY = newY - _lastY;
                } else {
                    _state.deltaX = 0;
                    _state.deltaY = 0;
                    _hasLastPos = true;
                }
                
                _lastX = newX;
                _lastY = newY;
                _state.x = newX;
                _state.y = newY;
                
                // Update wheel
                _state.scrollX = static_cast<float>(mouseState.wheelX);
                _state.scrollY = static_cast<float>(mouseState.wheelY);
                
                // Update buttons
                _state.buttons = 0;
                uint32_t buttons = static_cast<uint32_t>(mouseState.buttons);
                
                if (buttons & GameInputMouseLeftButton) {
                    _state.buttons |= (1u << static_cast<uint32_t>(MouseButton::left));
                }
                if (buttons & GameInputMouseRightButton) {
                    _state.buttons |= (1u << static_cast<uint32_t>(MouseButton::right));
                }
                if (buttons & GameInputMouseMiddleButton) {
                    _state.buttons |= (1u << static_cast<uint32_t>(MouseButton::middle));
                }
                if (buttons & GameInputMouseButton4) {
                    _state.buttons |= (1u << static_cast<uint32_t>(MouseButton::extra1));
                }
                if (buttons & GameInputMouseButton5) {
                    _state.buttons |= (1u << static_cast<uint32_t>(MouseButton::extra2));
                }
                
                _state.timestamp = reading->GetTimestamp();
            }
            reading->Release();
        }
    }

    void resetDeltas() {
        _state.deltaX = 0;
        _state.deltaY = 0;
        _state.scrollX = 0;
        _state.scrollY = 0;
    }
};

// ---------------------------------------------------------------------------
// GdkInputSystem::Impl
// ---------------------------------------------------------------------------

struct GdkInputSystem::Impl {
    IGameInput* gameInput = nullptr;
    uint32_t nextDeviceId = 1;
    
    std::vector<std::unique_ptr<GdkGamepad>> gamepads;
    std::unique_ptr<AggregateKeyboard> keyboard;
    std::unique_ptr<AggregateMouse> mouse;
    
    std::vector<DeviceObserver*> observers;
    
    ~Impl() {
        if (gameInput) {
            gameInput->Release();
            gameInput = nullptr;
        }
    }
    
    bool initialize() {
        // Create GameInput instance
        HRESULT hr = GameInputCreate(&gameInput);
        if (FAILED(hr) || !gameInput) {
            return false;
        }
        
        // Always create aggregate keyboard and mouse devices
        // These will receive input from any system keyboard/mouse
        keyboard = std::make_unique<AggregateKeyboard>(nextDeviceId++, gameInput);
        mouse = std::make_unique<AggregateMouse>(nextDeviceId++, gameInput);
        
        // Notify observers about keyboard and mouse
        notifyDeviceConnected(*keyboard);
        notifyDeviceConnected(*mouse);
        
        // Scan for gamepads
        scanForGamepads();
        
        return true;
    }
    
    void scanForGamepads() {
        if (!gameInput) return;
        
        // Try to get a current reading for gamepad
        IGameInputReading* reading = nullptr;
        HRESULT hr = gameInput->GetCurrentReading(GameInputKindGamepad, nullptr, &reading);
        
        if (SUCCEEDED(hr) && reading) {
            IGameInputDevice* device = nullptr;
            reading->GetDevice(&device);
            if (device) {
                // Check if we already have this device
                bool exists = false;
                for (const auto& gp : gamepads) {
                    if (gp->device() == device) {
                        exists = true;
                        break;
                    }
                }
                
                if (!exists) {
                    auto gamepad = std::make_unique<GdkGamepad>(nextDeviceId++, device);
                    if (gamepad->initialize()) {
                        notifyDeviceConnected(*gamepad);
                        gamepads.push_back(std::move(gamepad));
                    }
                }
                device->Release();
            }
            reading->Release();
        }
    }
    
    void updateDevices() {
        if (!gameInput) return;
        
        // Update gamepads
        for (auto& gp : gamepads) {
            IGameInputReading* reading = nullptr;
            if (gameInput->GetCurrentReading(GameInputKindGamepad, gp->device(), &reading) == S_OK) {
                if (reading) {
                    gp->updateState(reading);
                    reading->Release();
                }
            }
        }
        
        // Update aggregate keyboard and mouse
        if (keyboard) {
            keyboard->update();
        }
        if (mouse) {
            mouse->update();
            mouse->resetDeltas();
        }
        
        // Check for new gamepads
        scanForGamepads();
    }
    
    void notifyDeviceConnected(Device& device) {
        for (auto* observer : observers) {
            observer->onDeviceConnected(device);
        }
    }
};

// ---------------------------------------------------------------------------
// GdkInputSystem
// ---------------------------------------------------------------------------

GdkInputSystem::GdkInputSystem(const PlatformContext& ctx) 
    : pImpl(std::make_unique<Impl>()) {
    (void)ctx;
    pImpl->initialize();
}

GdkInputSystem::~GdkInputSystem() = default;

void GdkInputSystem::update() {
    pImpl->updateDevices();
}

void GdkInputSystem::addObserver(DeviceObserver* observer) {
    if (observer) {
        pImpl->observers.push_back(observer);
        
        // Notify about existing devices
        for (auto& gp : pImpl->gamepads) {
            observer->onDeviceConnected(*gp);
        }
        
        if (pImpl->keyboard) {
            observer->onDeviceConnected(*pImpl->keyboard);
        }
        
        if (pImpl->mouse) {
            observer->onDeviceConnected(*pImpl->mouse);
        }
    }
}

void GdkInputSystem::removeObserver(DeviceObserver* observer) {
    auto& obs = pImpl->observers;
    obs.erase(std::remove(obs.begin(), obs.end(), observer), obs.end());
}

int GdkInputSystem::gamepadCount() const {
    return static_cast<int>(pImpl->gamepads.size());
}

GamepadDevice* GdkInputSystem::gamepadAt(int index) {
    if (index < 0 || index >= static_cast<int>(pImpl->gamepads.size())) {
        return nullptr;
    }
    return pImpl->gamepads[index].get();
}

bool GdkInputSystem::hasKeyboard() const {
    return pImpl->keyboard != nullptr;
}

KeyboardDevice* GdkInputSystem::keyboard() {
    return pImpl->keyboard.get();
}

bool GdkInputSystem::hasMouse() const {
    return pImpl->mouse != nullptr;
}

MouseDevice* GdkInputSystem::mouse() {
    return pImpl->mouse.get();
}
