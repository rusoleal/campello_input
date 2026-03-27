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

#include "inc/campello_input/input_linux_system.hpp"
#include "inc/campello_input/gamepad_linux.hpp"
#include "inc/campello_input/keyboard_linux.hpp"
#include "inc/campello_input/mouse_linux.hpp"
#include <libudev.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <poll.h>
#include <cstring>
#include <algorithm>
#include <sys/ioctl.h>
#include <linux/input.h>

using namespace systems::leal::campello_input;

struct LinuxInputSystem::Impl {
    struct udev* udev = nullptr;
    struct udev_monitor* udevMonitor = nullptr;
    int udevMonitorFd = -1;
    
    uint32_t nextDeviceId = 1;
    
    std::vector<std::unique_ptr<LinuxGamepad>> gamepads;
    std::unique_ptr<LinuxKeyboard> keyboard;
    std::unique_ptr<LinuxMouse> mouse;
    
    std::vector<DeviceObserver*> observers;
    
    ~Impl() {
        if (udevMonitor) {
            udev_monitor_unref(udevMonitor);
        }
        if (udev) {
            udev_unref(udev);
        }
    }
    
    bool initialize() {
        udev = udev_new();
        if (!udev) {
            return false;
        }
        
        // Set up udev monitor for hot-plugging
        udevMonitor = udev_monitor_new_from_netlink(udev, "udev");
        if (udevMonitor) {
            udev_monitor_filter_add_match_subsystem_devtype(udevMonitor, "input", nullptr);
            udev_monitor_enable_receiving(udevMonitor);
            udevMonitorFd = udev_monitor_get_fd(udevMonitor);
        }
        
        // Scan for existing devices
        scanForDevices();
        
        return true;
    }
    
    void scanForDevices() {
        struct udev_enumerate* enumerate = udev_enumerate_new(udev);
        udev_enumerate_add_match_subsystem(enumerate, "input");
        udev_enumerate_scan_devices(enumerate);
        
        struct udev_list_entry* devices = udev_enumerate_get_list_entry(enumerate);
        struct udev_list_entry* entry;
        
        udev_list_entry_foreach(entry, devices) {
            const char* path = udev_list_entry_get_name(entry);
            struct udev_device* device = udev_device_new_from_syspath(udev, path);
            
            if (device) {
                processDevice(device);
                udev_device_unref(device);
            }
        }
        
        udev_enumerate_unref(enumerate);
    }
    
    void processDevice(struct udev_device* device) {
        const char* devNode = udev_device_get_devnode(device);
        if (!devNode) return;
        
        // Skip event0 (usually system stuff)
        const char* sysname = udev_device_get_sysname(device);
        if (!sysname || strncmp(sysname, "event", 5) != 0) return;
        
        // Check if device is already known
        std::string devPath(devNode);
        for (const auto& gp : gamepads) {
            if (gp->devicePath() == devPath) return;
        }
        if (keyboard && keyboard->devicePath() == devPath) return;
        if (mouse && mouse->devicePath() == devPath) return;
        
        const char* sysPath = udev_device_get_syspath(device);
        
        // Try to open and identify the device
        int fd = open(devNode, O_RDONLY | O_NONBLOCK);
        if (fd < 0) return;
        
        // Use a simple heuristic: try to read device properties
        char name[256] = {0};
        if (ioctl(fd, EVIOCGNAME(sizeof(name)), name) < 0) {
            close(fd);
            return;
        }
        close(fd);
        
        // Check device capabilities via udev properties
        struct udev_device* parent = udev_device_get_parent_with_subsystem_devtype(
            device, "input", nullptr);
        
        bool isJoystick = udev_device_get_property_value(device, "ID_INPUT_JOYSTICK") != nullptr;
        bool isGamepad = udev_device_get_property_value(device, "ID_INPUT_GAMEPAD") != nullptr;
        bool isKeyboard = udev_device_get_property_value(device, "ID_INPUT_KEYBOARD") != nullptr;
        bool isMouse = udev_device_get_property_value(device, "ID_INPUT_MOUSE") != nullptr;
        bool isTouchpad = udev_device_get_property_value(device, "ID_INPUT_TOUCHPAD") != nullptr;
        
        // Heuristic: if no properties set, try to detect by name
        if (!isJoystick && !isGamepad && !isKeyboard && !isMouse && !isTouchpad) {
            const char* nameLower = name;
            if (strstr(nameLower, "joystick") || strstr(nameLower, "gamepad") ||
                strstr(nameLower, "controller") || strstr(nameLower, "xbox") ||
                strstr(nameLower, "playstation") || strstr(nameLower, "dualshock") ||
                strstr(nameLower, "dualsense")) {
                isGamepad = true;
            } else if (strstr(nameLower, "mouse") || strstr(nameLower, "trackball")) {
                isMouse = true;
            } else if (strstr(nameLower, "keyboard") || strstr(nameLower, "keypad")) {
                isKeyboard = true;
            }
        }
        
        // Don't treat touchpad as mouse (usually integrated)
        if (isTouchpad && isMouse) {
            isMouse = false;
        }
        
        if (isGamepad || isJoystick) {
            auto gamepad = std::make_unique<LinuxGamepad>(nextDeviceId++, name, devNode);
            if (gamepad->initialize()) {
                notifyDeviceConnected(DeviceType::gamepad, gamepad.get());
                gamepads.push_back(std::move(gamepad));
            }
        } else if (isKeyboard && !keyboard) {
            // Use the first keyboard found
            auto kb = std::make_unique<LinuxKeyboard>(nextDeviceId++, name, devNode);
            if (kb->initialize()) {
                notifyDeviceConnected(DeviceType::keyboard, kb.get());
                keyboard = std::move(kb);
            }
        } else if (isMouse && !mouse) {
            // Use the first mouse found
            auto ms = std::make_unique<LinuxMouse>(nextDeviceId++, name, devNode);
            if (ms->initialize()) {
                notifyDeviceConnected(DeviceType::mouse, ms.get());
                mouse = std::move(ms);
            }
        }
    }
    
    void checkForNewDevices() {
        if (!udevMonitor || udevMonitorFd < 0) return;
        
        struct pollfd pfd;
        pfd.fd = udevMonitorFd;
        pfd.events = POLLIN;
        
        // Non-blocking check
        if (poll(&pfd, 1, 0) > 0 && (pfd.revents & POLLIN)) {
            struct udev_device* device = udev_monitor_receive_device(udevMonitor);
            if (device) {
                const char* action = udev_device_get_action(device);
                if (action && strcmp(action, "add") == 0) {
                    processDevice(device);
                } else if (action && strcmp(action, "remove") == 0) {
                    // Handle removal
                    handleDeviceRemoval(device);
                }
                udev_device_unref(device);
            }
        }
    }
    
    void handleDeviceRemoval(struct udev_device* device) {
        const char* devNode = udev_device_get_devnode(device);
        if (!devNode) return;
        
        std::string devPath(devNode);
        
        // Check gamepads
        for (auto it = gamepads.begin(); it != gamepads.end();) {
            if ((*it)->devicePath() == devPath) {
                notifyDeviceDisconnected(DeviceType::gamepad, it->get());
                it = gamepads.erase(it);
            } else {
                ++it;
            }
        }
        
        // Check keyboard
        if (keyboard && keyboard->devicePath() == devPath) {
            notifyDeviceDisconnected(DeviceType::keyboard, keyboard.get());
            keyboard.reset();
        }
        
        // Check mouse
        if (mouse && mouse->devicePath() == devPath) {
            notifyDeviceDisconnected(DeviceType::mouse, mouse.get());
            mouse.reset();
        }
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
    
    void notifyDeviceDisconnected(DeviceType type, InputDevice* device) {
        for (auto* observer : observers) {
            observer->onDeviceDisconnected(device->id());
        }
    }
    
    void update() {
        // Check for hot-plug events
        checkForNewDevices();
        
        // Update all devices
        for (auto& gp : gamepads) {
            gp->updateState();
        }
        if (keyboard) {
            keyboard->updateState();
        }
        if (mouse) {
            mouse->updateState();
            mouse->resetDeltas();
        }
    }
};

LinuxInputSystem::LinuxInputSystem(const PlatformContext& ctx) 
    : pImpl(std::make_unique<Impl>()) {
    (void)ctx;
    pImpl->initialize();
}

LinuxInputSystem::~LinuxInputSystem() = default;

void LinuxInputSystem::update() {
    pImpl->update();
}

void LinuxInputSystem::addObserver(DeviceObserver* observer) {
    if (observer) {
        pImpl->observers.push_back(observer);
    }
}

void LinuxInputSystem::removeObserver(DeviceObserver* observer) {
    auto& obs = pImpl->observers;
    obs.erase(std::remove(obs.begin(), obs.end(), observer), obs.end());
}

int LinuxInputSystem::gamepadCount() const {
    return static_cast<int>(pImpl->gamepads.size());
}

GamepadDevice* LinuxInputSystem::gamepadAt(int index) {
    if (index < 0 || index >= static_cast<int>(pImpl->gamepads.size())) {
        return nullptr;
    }
    return pImpl->gamepads[index].get();
}

bool LinuxInputSystem::hasKeyboard() const {
    return pImpl->keyboard != nullptr;
}

KeyboardDevice* LinuxInputSystem::keyboard() {
    return pImpl->keyboard.get();
}

bool LinuxInputSystem::hasMouse() const {
    return pImpl->mouse != nullptr;
}

MouseDevice* LinuxInputSystem::mouse() {
    return pImpl->mouse.get();
}
