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

#include <gtest/gtest.h>
#include "mock_input_system.hpp"
#include "mock_gamepad.hpp"
#include "mock_keyboard.hpp"
#include "mock_mouse.hpp"

using namespace systems::leal::campello_input;

// =============================================================================
// Test Observer Implementation
// =============================================================================

class TestObserver : public DeviceObserver {
public:
    int connectCount = 0;
    int disconnectCount = 0;
    Device* lastConnectedDevice = nullptr;
    Device* lastDisconnectedDevice = nullptr;
    DeviceType lastConnectedType = DeviceType::gamepad;
    DeviceType lastDisconnectedType = DeviceType::gamepad;

    void reset() {
        connectCount = 0;
        disconnectCount = 0;
        lastConnectedDevice = nullptr;
        lastDisconnectedDevice = nullptr;
    }

    void onDeviceConnected(Device& device) override {
        connectCount++;
        lastConnectedDevice = &device;
        lastConnectedType = device.type();
    }

    void onDeviceDisconnected(Device& device) override {
        disconnectCount++;
        lastDisconnectedDevice = &device;
        lastDisconnectedType = device.type();
    }
};

// =============================================================================
// Observer Registration Tests
// =============================================================================

TEST(ObserverTest, AddObserver) {
    MockInputSystem input;
    TestObserver observer;

    input.addObserver(&observer);

    // Initially no devices, so no callbacks yet
    EXPECT_EQ(observer.connectCount, 0);
}

TEST(ObserverTest, RemoveObserver) {
    MockInputSystem input;
    TestObserver observer;

    input.addObserver(&observer);
    input.removeObserver(&observer);

    // Add a device after removal - observer should not be called
    auto gamepad = std::make_unique<MockGamepad>(1, "Test");
    input.addGamepad(std::move(gamepad));

    EXPECT_EQ(observer.connectCount, 0);
}

// =============================================================================
// Device Connection Callback Tests
// =============================================================================

TEST(ObserverTest, CallbackOnGamepadConnect) {
    MockInputSystem input;
    TestObserver observer;

    input.addObserver(&observer);

    auto gamepad = std::make_unique<MockGamepad>(1, "Test Gamepad");
    auto* gamepadPtr = gamepad.get();
    input.addGamepad(std::move(gamepad));

    EXPECT_EQ(observer.connectCount, 1);
    EXPECT_EQ(observer.lastConnectedDevice, gamepadPtr);
    EXPECT_EQ(observer.lastConnectedType, DeviceType::gamepad);
}

TEST(ObserverTest, CallbackOnKeyboardConnect) {
    MockInputSystem input;
    TestObserver observer;

    input.addObserver(&observer);

    auto keyboard = std::make_unique<MockKeyboard>(1, "Test Keyboard");
    auto* keyboardPtr = keyboard.get();
    input.setKeyboard(std::move(keyboard));

    EXPECT_EQ(observer.connectCount, 1);
    EXPECT_EQ(observer.lastConnectedDevice, keyboardPtr);
    EXPECT_EQ(observer.lastConnectedType, DeviceType::keyboard);
}

TEST(ObserverTest, CallbackOnMouseConnect) {
    MockInputSystem input;
    TestObserver observer;

    input.addObserver(&observer);

    auto mouse = std::make_unique<MockMouse>(1, "Test Mouse");
    auto* mousePtr = mouse.get();
    input.setMouse(std::move(mouse));

    EXPECT_EQ(observer.connectCount, 1);
    EXPECT_EQ(observer.lastConnectedDevice, mousePtr);
    EXPECT_EQ(observer.lastConnectedType, DeviceType::mouse);
}

// =============================================================================
// Device Disconnection Callback Tests
// =============================================================================

TEST(ObserverTest, CallbackOnGamepadDisconnect) {
    MockInputSystem input;
    TestObserver observer;

    input.addObserver(&observer);

    auto gamepad = std::make_unique<MockGamepad>(42, "Test");
    input.addGamepad(std::move(gamepad));

    observer.reset();
    input.removeGamepad(42);

    EXPECT_EQ(observer.disconnectCount, 1);
    EXPECT_EQ(observer.lastDisconnectedType, DeviceType::gamepad);
}

TEST(ObserverTest, CallbackOnKeyboardDisconnect) {
    MockInputSystem input;
    TestObserver observer;

    input.addObserver(&observer);
    input.setKeyboard(std::make_unique<MockKeyboard>(1, "Test"));

    observer.reset();
    input.setKeyboard(nullptr);

    EXPECT_EQ(observer.disconnectCount, 1);
    EXPECT_EQ(observer.lastDisconnectedType, DeviceType::keyboard);
}

// =============================================================================
// Multiple Observers Tests
// =============================================================================

TEST(ObserverTest, MultipleObservers) {
    MockInputSystem input;
    TestObserver observer1;
    TestObserver observer2;

    input.addObserver(&observer1);
    input.addObserver(&observer2);

    auto gamepad = std::make_unique<MockGamepad>(1, "Test");
    input.addGamepad(std::move(gamepad));

    EXPECT_EQ(observer1.connectCount, 1);
    EXPECT_EQ(observer2.connectCount, 1);
}

TEST(ObserverTest, RemoveOneObserver) {
    MockInputSystem input;
    TestObserver observer1;
    TestObserver observer2;

    input.addObserver(&observer1);
    input.addObserver(&observer2);

    input.removeObserver(&observer1);

    auto gamepad = std::make_unique<MockGamepad>(1, "Test");
    input.addGamepad(std::move(gamepad));

    EXPECT_EQ(observer1.connectCount, 0);  // Removed
    EXPECT_EQ(observer2.connectCount, 1);  // Still active
}

// =============================================================================
// Existing Devices on Registration Tests
// =============================================================================

TEST(ObserverTest, NotifyExistingDevicesOnRegister) {
    MockInputSystem input;

    // Add devices before observer
    auto gamepad = std::make_unique<MockGamepad>(1, "Gamepad");
    input.addGamepad(std::move(gamepad));
    input.setKeyboard(std::make_unique<MockKeyboard>(2, "Keyboard"));

    TestObserver observer;
    input.addObserver(&observer);

    // Observer should be notified about existing devices
    EXPECT_EQ(observer.connectCount, 2);
}

// =============================================================================
// Clear All Devices Tests
// =============================================================================

TEST(ObserverTest, ClearAllDevices) {
    MockInputSystem input;
    TestObserver observer;

    input.addObserver(&observer);

    input.addGamepad(std::make_unique<MockGamepad>(1, "Gamepad 1"));
    input.addGamepad(std::make_unique<MockGamepad>(2, "Gamepad 2"));
    input.setKeyboard(std::make_unique<MockKeyboard>(3, "Keyboard"));

    observer.reset();
    input.clearAllDevices();

    EXPECT_EQ(observer.disconnectCount, 3);  // 2 gamepads + keyboard
    EXPECT_EQ(input.gamepadCount(), 0);
    EXPECT_FALSE(input.hasKeyboard());
}

// =============================================================================
// Device Access After Notification Tests
// =============================================================================

TEST(ObserverTest, DeviceAccessibleInCallback) {
    MockInputSystem input;

    class AccessTestObserver : public DeviceObserver {
    public:
        bool deviceWasAccessible = false;

        void onDeviceConnected(Device& device) override {
            // Try to access device properties
            deviceWasAccessible = (device.id() != 0 && device.name() != nullptr);
        }

        void onDeviceDisconnected(Device& device) override {
            (void)device;
        }
    };

    AccessTestObserver observer;
    input.addObserver(&observer);

    auto gamepad = std::make_unique<MockGamepad>(42, "Test Gamepad");
    input.addGamepad(std::move(gamepad));

    EXPECT_TRUE(observer.deviceWasAccessible);
}
