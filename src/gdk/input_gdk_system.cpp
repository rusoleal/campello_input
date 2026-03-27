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

#include <GameInput.h>
#include <algorithm>
#include <cstring>

using namespace systems::leal::campello_input;

// ---------------------------------------------------------------------------
// GdkInputSystem::Impl
// ---------------------------------------------------------------------------

struct GdkInputSystem::Impl {
    IGameInput* gameInput = nullptr;
    uint32_t nextDeviceId = 1;
    
    std::vector<std::unique_ptr<GdkGamepad>> gamepads;
    std::unique_ptr<GdkKeyboard> keyboard;
    std::unique_ptr<GdkMouse> mouse;
    
    std::vector<DeviceObserver*> observers;
    
    ~Impl() {
        // Release GameInput reference
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
        
        // Perform initial device scan
        scanForDevices();
        
        return true;
    }
    
    void scanForDevices() {
        if (!gameInput) return;
        
        // Enumerate gamepads (GameInputKindGamepad)
        IGameInputDevice* device = nullptr;
        uint32_t deviceIndex = 0;
        
        while (gameInput->GetDeviceByIndex(GameInputKindGamepad, deviceIndex, &device) == S_OK) {
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
                        notifyDeviceConnected(DeviceType::gamepad, gamepad.get());
                        gamepads.push_back(std::move(gamepad));
                    }
                }
                device->Release();
            }
            ++deviceIndex;
        }
        
        // Enumerate keyboards (GameInputKindKeyboard)
        if (!keyboard) {
            if (gameInput->GetDeviceByIndex(GameInputKindKeyboard, 0, &device) == S_OK && device) {
                keyboard = std::make_unique<GdkKeyboard>(nextDeviceId++, device);
                if (keyboard->initialize()) {
                    notifyDeviceConnected(DeviceType::keyboard, keyboard.get());
                } else {
                    keyboard.reset();
                }
                device->Release();
            }
        }
        
        // Enumerate mice (GameInputKindMouse)
        if (!mouse) {
            if (gameInput->GetDeviceByIndex(GameInputKindMouse, 0, &device) == S_OK && device) {
                mouse = std::make_unique<GdkMouse>(nextDeviceId++, device);
                if (mouse->initialize()) {
                    notifyDeviceConnected(DeviceType::mouse, mouse.get());
                } else {
                    mouse.reset();
                }
                device->Release();
            }
        }
    }
    
    void updateDevices() {
        if (!gameInput) return;
        
        // Get current reading for gamepads
        for (auto& gp : gamepads) {
            IGameInputReading* reading = nullptr;
            if (gameInput->GetCurrentReading(GameInputKindGamepad, gp->device(), &reading) == S_OK) {
                if (reading) {
                    gp->updateState(reading);
                    reading->Release();
                }
            }
        }
        
        // Update keyboard
        if (keyboard) {
            IGameInputReading* reading = nullptr;
            if (gameInput->GetCurrentReading(GameInputKindKeyboard, keyboard->device(), &reading) == S_OK) {
                if (reading) {
                    keyboard->updateState(reading);
                    reading->Release();
                }
            }
        }
        
        // Update mouse
        if (mouse) {
            IGameInputReading* reading = nullptr;
            if (gameInput->GetCurrentReading(GameInputKindMouse, mouse->device(), &reading) == S_OK) {
                if (reading) {
                    mouse->updateState(reading);
                    reading->Release();
                }
            }
            mouse->resetDeltas();
        }
        
        // Check for new devices (simple polling approach)
        scanForDevices();
    }
    
    void notifyDeviceConnected(DeviceType type, InputDevice* device) {
        DeviceInfo info;
        info.id = device->id();
        info.type = type;
        info.name = device->name();
        info.productName = device->productName();
        info.manufacturer = device->manufacturer();
        info.serialNumber = device->serialNumber();
        
        for (auto* observer : observers) {
            observer->onDeviceConnected(info);
        }
    }
};

// ---------------------------------------------------------------------------
// GdkInputSystem
// ---------------------------------------------------------------------------

GdkInputSystem::GdkInputSystem(const PlatformContext& ctx) 
    : pImpl(std::make_unique<Impl>()) {
    (void)ctx;  // No context needed for GDK, uses global GameInput
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
            DeviceInfo info;
            info.id = gp->id();
            info.type = DeviceType::gamepad;
            info.name = gp->name();
            observer->onDeviceConnected(info);
        }
        
        if (pImpl->keyboard) {
            DeviceInfo info;
            info.id = pImpl->keyboard->id();
            info.type = DeviceType::keyboard;
            info.name = pImpl->keyboard->name();
            observer->onDeviceConnected(info);
        }
        
        if (pImpl->mouse) {
            DeviceInfo info;
            info.id = pImpl->mouse->id();
            info.type = DeviceType::mouse;
            info.name = pImpl->mouse->name();
            observer->onDeviceConnected(info);
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
