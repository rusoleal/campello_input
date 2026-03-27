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
#include "mock_gamepad.hpp"
#include "mock_keyboard.hpp"
#include "mock_mouse.hpp"

using namespace systems::leal::campello_input;

// =============================================================================
// MockGamepad Tests
// =============================================================================

TEST(MockGamepadTest, Construction) {
    MockGamepad gamepad(42, "Test Gamepad");

    EXPECT_EQ(gamepad.id(), 42);
    EXPECT_STREQ(gamepad.name(), "Test Gamepad");
    EXPECT_EQ(gamepad.type(), DeviceType::gamepad);
    EXPECT_EQ(gamepad.connectionStatus(), ConnectionStatus::connected);
}

TEST(MockGamepadTest, DefaultState) {
    MockGamepad gamepad(1, "Default");

    GamepadState state;
    EXPECT_TRUE(gamepad.getState(state));

    EXPECT_EQ(state.timestamp, 0);
    EXPECT_EQ(state.buttons, 0);
    for (int i = 0; i < static_cast<int>(GamepadAxis::count); ++i) {
        EXPECT_FLOAT_EQ(state.axes[i].value, 0.0f);
    }
}

TEST(MockGamepadTest, SetState) {
    MockGamepad gamepad(1, "Test");

    GamepadState newState{};
    newState.timestamp = 123456;
    newState.buttons = static_cast<uint64_t>(GamepadButton::face_south);
    newState.axes[0].value = 0.5f;
    newState.axes[1].value = -0.5f;

    gamepad.setState(newState);

    GamepadState retrieved;
    EXPECT_TRUE(gamepad.getState(retrieved));

    EXPECT_EQ(retrieved.timestamp, 123456);
    EXPECT_TRUE(retrieved.buttons & GamepadButton::face_south);
    EXPECT_FLOAT_EQ(retrieved.axes[0].value, 0.5f);
    EXPECT_FLOAT_EQ(retrieved.axes[1].value, -0.5f);
}

TEST(MockGamepadTest, SetButton) {
    MockGamepad gamepad(1, "Test");

    gamepad.setButton(GamepadButton::face_south, true);
    gamepad.setButton(GamepadButton::shoulder_left, true);

    GamepadState state;
    gamepad.getState(state);

    EXPECT_TRUE(state.buttons & GamepadButton::face_south);
    EXPECT_TRUE(state.buttons & GamepadButton::shoulder_left);
    EXPECT_FALSE(state.buttons & GamepadButton::face_east);

    // Clear button
    gamepad.setButton(GamepadButton::face_south, false);
    gamepad.getState(state);
    EXPECT_FALSE(state.buttons & GamepadButton::face_south);
}

TEST(MockGamepadTest, SetAxis) {
    MockGamepad gamepad(1, "Test");

    gamepad.setAxis(GamepadAxis::left_x, 0.75f);
    gamepad.setAxis(GamepadAxis::left_y, -0.5f);
    gamepad.setAxis(GamepadAxis::trigger_left, 0.25f);

    GamepadState state;
    gamepad.getState(state);

    EXPECT_FLOAT_EQ(state.axes[static_cast<uint8_t>(GamepadAxis::left_x)].value, 0.75f);
    EXPECT_FLOAT_EQ(state.axes[static_cast<uint8_t>(GamepadAxis::left_y)].value, -0.5f);
    EXPECT_FLOAT_EQ(state.axes[static_cast<uint8_t>(GamepadAxis::trigger_left)].value, 0.25f);
}

TEST(MockGamepadTest, ConnectionStatus) {
    MockGamepad gamepad(1, "Test");

    EXPECT_EQ(gamepad.connectionStatus(), ConnectionStatus::connected);

    gamepad.setConnectionStatus(ConnectionStatus::disconnected);
    EXPECT_EQ(gamepad.connectionStatus(), ConnectionStatus::disconnected);
}

TEST(MockGamepadTest, NoHaptics) {
    MockGamepad gamepad(1, "Test");
    EXPECT_EQ(gamepad.haptics(), nullptr);
}

// =============================================================================
// MockKeyboard Tests
// =============================================================================

TEST(MockKeyboardTest, Construction) {
    MockKeyboard keyboard(42, "Test Keyboard");

    EXPECT_EQ(keyboard.id(), 42);
    EXPECT_STREQ(keyboard.name(), "Test Keyboard");
    EXPECT_EQ(keyboard.type(), DeviceType::keyboard);
    EXPECT_EQ(keyboard.connectionStatus(), ConnectionStatus::connected);
}

TEST(MockKeyboardTest, SetKeyDown) {
    MockKeyboard keyboard(1, "Test");

    keyboard.setKeyDown(KeyCode::a, true);
    keyboard.setKeyDown(KeyCode::space, true);

    EXPECT_TRUE(keyboard.isKeyDown(KeyCode::a));
    EXPECT_TRUE(keyboard.isKeyDown(KeyCode::space));
    EXPECT_FALSE(keyboard.isKeyDown(KeyCode::b));

    // Clear key
    keyboard.setKeyDown(KeyCode::a, false);
    EXPECT_FALSE(keyboard.isKeyDown(KeyCode::a));
}

TEST(MockKeyboardTest, SetModifiers) {
    MockKeyboard keyboard(1, "Test");

    keyboard.setModifiers(KeyModifier::shift | KeyModifier::ctrl);

    EXPECT_TRUE(keyboard.modifiers() & KeyModifier::shift);
    EXPECT_TRUE(keyboard.modifiers() & KeyModifier::ctrl);
    EXPECT_FALSE(keyboard.modifiers() & KeyModifier::alt);
}

TEST(MockKeyboardTest, GetState) {
    MockKeyboard keyboard(1, "Test");

    keyboard.setKeyDown(KeyCode::enter, true);
    keyboard.setModifiers(KeyModifier::ctrl);

    KeyboardState state;
    keyboard.getState(state);

    // Check that keysDown bitmap is set
    uint32_t idx = static_cast<uint32_t>(KeyCode::enter);
    EXPECT_TRUE((state.keysDown[idx >> 6] >> (idx & 63)) & 1);
    EXPECT_EQ(state.modifiers, KeyModifier::ctrl);
}

// =============================================================================
// MockMouse Tests
// =============================================================================

TEST(MockMouseTest, Construction) {
    MockMouse mouse(42, "Test Mouse");

    EXPECT_EQ(mouse.id(), 42);
    EXPECT_STREQ(mouse.name(), "Test Mouse");
    EXPECT_EQ(mouse.type(), DeviceType::mouse);
    EXPECT_EQ(mouse.connectionStatus(), ConnectionStatus::connected);
}

TEST(MockMouseTest, SetPosition) {
    MockMouse mouse(1, "Test");

    mouse.setPosition(100.0f, 200.0f);

    MouseState state;
    mouse.getState(state);

    EXPECT_FLOAT_EQ(state.x, 100.0f);
    EXPECT_FLOAT_EQ(state.y, 200.0f);
}

TEST(MockMouseTest, SetDelta) {
    MockMouse mouse(1, "Test");

    mouse.setDelta(5.5f, -3.0f);

    MouseState state;
    mouse.getState(state);

    EXPECT_FLOAT_EQ(state.deltaX, 5.5f);
    EXPECT_FLOAT_EQ(state.deltaY, -3.0f);
}

TEST(MockMouseTest, SetScroll) {
    MockMouse mouse(1, "Test");

    mouse.setScroll(0.0f, 120.0f);

    MouseState state;
    mouse.getState(state);

    EXPECT_FLOAT_EQ(state.scrollX, 0.0f);
    EXPECT_FLOAT_EQ(state.scrollY, 120.0f);
}

TEST(MockMouseTest, SetButtonDown) {
    MockMouse mouse(1, "Test");

    mouse.setButtonDown(MouseButton::left, true);
    mouse.setButtonDown(MouseButton::right, true);

    EXPECT_TRUE(mouse.isButtonDown(MouseButton::left));
    EXPECT_TRUE(mouse.isButtonDown(MouseButton::right));
    EXPECT_FALSE(mouse.isButtonDown(MouseButton::middle));

    MouseState state;
    mouse.getState(state);
    EXPECT_NE(state.buttons & (1 << static_cast<uint8_t>(MouseButton::left)), 0);
}

TEST(MockMouseTest, CursorLock) {
    MockMouse mouse(1, "Test");

    EXPECT_FALSE(mouse.isCursorLocked());

    mouse.setCursorLocked(true);
    EXPECT_TRUE(mouse.isCursorLocked());

    mouse.setCursorLocked(false);
    EXPECT_FALSE(mouse.isCursorLocked());
}
