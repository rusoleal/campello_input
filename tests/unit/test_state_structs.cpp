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
#include <campello_input/gamepad.hpp>
#include <campello_input/keyboard.hpp>
#include <campello_input/mouse.hpp>
#include <campello_input/touch.hpp>

using namespace systems::leal::campello_input;

// =============================================================================
// GamepadState Tests
// =============================================================================

TEST(GamepadStateTest, SizeAndAlignment) {
    // Check that GamepadState has reasonable size
    EXPECT_EQ(sizeof(GamepadState),
              sizeof(uint64_t) +                          // timestamp
              sizeof(uint64_t) +                          // buttons
              sizeof(GamepadAxisState) * 6);              // axes array

    // Alignment should be suitable for the largest member
    EXPECT_GE(alignof(GamepadState), alignof(uint64_t));
}

TEST(GamepadStateTest, DefaultConstruction) {
    GamepadState state{};

    EXPECT_EQ(state.timestamp, 0);
    EXPECT_EQ(state.buttons, 0);

    for (int i = 0; i < static_cast<int>(GamepadAxis::count); ++i) {
        EXPECT_EQ(state.axes[i].value, 0.0f);
        EXPECT_EQ(state.axes[i].deadzone, 0.0f);
    }
}

TEST(GamepadStateTest, AxisAccessByEnum) {
    GamepadState state{};

    // Set values using enum
    state.axes[static_cast<uint8_t>(GamepadAxis::left_x)].value = 0.5f;
    state.axes[static_cast<uint8_t>(GamepadAxis::left_y)].value = -0.5f;
    state.axes[static_cast<uint8_t>(GamepadAxis::trigger_left)].value = 0.75f;

    EXPECT_FLOAT_EQ(state.axes[0].value, 0.5f);
    EXPECT_FLOAT_EQ(state.axes[1].value, -0.5f);
    EXPECT_FLOAT_EQ(state.axes[4].value, 0.75f);
}

TEST(GamepadStateTest, ButtonBitmaskOperations) {
    GamepadState state{};

    state.buttons = static_cast<uint64_t>(GamepadButton::face_south) |
                    static_cast<uint64_t>(GamepadButton::shoulder_left);

    EXPECT_TRUE(state.buttons & GamepadButton::face_south);
    EXPECT_TRUE(state.buttons & GamepadButton::shoulder_left);
    EXPECT_FALSE(state.buttons & GamepadButton::face_east);
}

// =============================================================================
// KeyboardState Tests
// =============================================================================

TEST(KeyboardStateTest, SizeAndLayout) {
    // keysDown is 4 uint64_t = 256 bits
    EXPECT_EQ(sizeof(KeyboardState::keysDown), sizeof(uint64_t) * 4);

    // Total size should be at least the sum of members (may include padding)
    EXPECT_GE(sizeof(KeyboardState),
              sizeof(uint64_t) * 4 + sizeof(KeyModifier));
}

TEST(KeyboardStateTest, DefaultConstruction) {
    KeyboardState state{};

    for (int i = 0; i < 4; ++i) {
        EXPECT_EQ(state.keysDown[i], 0);
    }
    EXPECT_EQ(state.modifiers, KeyModifier::none);
}

TEST(KeyboardStateTest, KeyBitmapOperations) {
    KeyboardState state{};

    // Set some keys using the bitmap formula from the header
    auto setKey = [&state](KeyCode code) {
        uint32_t idx = static_cast<uint32_t>(code);
        state.keysDown[idx >> 6] |= 1ULL << (idx & 63);
    };

    auto isKeyDown = [&state](KeyCode code) -> bool {
        uint32_t idx = static_cast<uint32_t>(code);
        return (state.keysDown[idx >> 6] >> (idx & 63)) & 1;
    };

    setKey(KeyCode::a);
    setKey(KeyCode::z);
    setKey(KeyCode::space);
    setKey(KeyCode::enter);

    EXPECT_TRUE(isKeyDown(KeyCode::a));
    EXPECT_TRUE(isKeyDown(KeyCode::z));
    EXPECT_TRUE(isKeyDown(KeyCode::space));
    EXPECT_TRUE(isKeyDown(KeyCode::enter));
    EXPECT_FALSE(isKeyDown(KeyCode::b));
    EXPECT_FALSE(isKeyDown(KeyCode::escape));
}

TEST(KeyboardStateTest, ModifierStorage) {
    KeyboardState state{};

    state.modifiers = KeyModifier::shift | KeyModifier::ctrl;
    EXPECT_TRUE(state.modifiers & KeyModifier::shift);
    EXPECT_TRUE(state.modifiers & KeyModifier::ctrl);
    EXPECT_FALSE(state.modifiers & KeyModifier::alt);
}

// =============================================================================
// MouseState Tests
// =============================================================================

TEST(MouseStateTest, SizeAndAlignment) {
    // Check that MouseState has at least the expected size (may include padding)
    EXPECT_GE(sizeof(MouseState),
              sizeof(uint64_t) +      // timestamp
              sizeof(float) * 6 +     // x, y, deltaX, deltaY, scrollX, scrollY
              sizeof(uint8_t));       // buttons

    // Alignment should be suitable for float operations
    EXPECT_GE(alignof(MouseState), alignof(float));
}

TEST(MouseStateTest, DefaultConstruction) {
    MouseState state{};

    EXPECT_EQ(state.timestamp, 0);
    EXPECT_FLOAT_EQ(state.x, 0.0f);
    EXPECT_FLOAT_EQ(state.y, 0.0f);
    EXPECT_FLOAT_EQ(state.deltaX, 0.0f);
    EXPECT_FLOAT_EQ(state.deltaY, 0.0f);
    EXPECT_FLOAT_EQ(state.scrollX, 0.0f);
    EXPECT_FLOAT_EQ(state.scrollY, 0.0f);
    EXPECT_EQ(state.buttons, 0);
}

TEST(MouseStateTest, ButtonBitmask) {
    MouseState state{};

    // Set left and right buttons
    state.buttons = (1 << static_cast<uint8_t>(MouseButton::left)) |
                    (1 << static_cast<uint8_t>(MouseButton::right));

    EXPECT_NE(state.buttons & (1 << static_cast<uint8_t>(MouseButton::left)), 0);
    EXPECT_NE(state.buttons & (1 << static_cast<uint8_t>(MouseButton::right)), 0);
    EXPECT_EQ(state.buttons & (1 << static_cast<uint8_t>(MouseButton::middle)), 0);
}

// =============================================================================
// TouchPoint Tests
// =============================================================================

TEST(TouchPointTest, SizeAndAlignment) {
    // TouchPoint should be reasonably sized
    EXPECT_GE(sizeof(TouchPoint), sizeof(uint32_t) + sizeof(TouchPhase) +
                                  sizeof(float) * 5 + sizeof(uint64_t));
}

// =============================================================================
// TouchState Tests
// =============================================================================

TEST(TouchStateTest, MaxTouchPointsConstant) {
    EXPECT_EQ(kMaxTouchPoints, 10);
}

TEST(TouchStateTest, ArraySize) {
    EXPECT_EQ(sizeof(TouchState::points) / sizeof(TouchPoint), kMaxTouchPoints);
}

TEST(TouchStateTest, DefaultConstruction) {
    TouchState state{};

    EXPECT_EQ(state.count, 0);
    for (uint32_t i = 0; i < kMaxTouchPoints; ++i) {
        EXPECT_EQ(state.points[i].id, 0);
        EXPECT_EQ(state.points[i].phase, TouchPhase::began);
        EXPECT_FLOAT_EQ(state.points[i].x, 0.0f);
        EXPECT_FLOAT_EQ(state.points[i].y, 0.0f);
    }
}

TEST(TouchStateTest, CountBounds) {
    // count should never exceed kMaxTouchPoints
    TouchState state{};

    state.count = 5;
    EXPECT_LE(state.count, kMaxTouchPoints);

    state.count = kMaxTouchPoints;
    EXPECT_LE(state.count, kMaxTouchPoints);
}

// =============================================================================
// GamepadAxisState Tests
// =============================================================================

TEST(GamepadAxisStateTest, ValueRange) {
    GamepadAxisState axis{};

    // Test that value can hold normalized range [-1, 1] for sticks
    axis.value = 1.0f;
    EXPECT_FLOAT_EQ(axis.value, 1.0f);

    axis.value = -1.0f;
    EXPECT_FLOAT_EQ(axis.value, -1.0f);

    // Test that value can hold [0, 1] for triggers
    axis.value = 0.0f;
    EXPECT_FLOAT_EQ(axis.value, 0.0f);

    axis.value = 0.5f;
    EXPECT_FLOAT_EQ(axis.value, 0.5f);
}

TEST(GamepadAxisStateTest, DeadzoneStorage) {
    GamepadAxisState axis{};

    axis.deadzone = 0.1f;
    EXPECT_FLOAT_EQ(axis.deadzone, 0.1f);

    axis.deadzone = 0.0f;
    EXPECT_FLOAT_EQ(axis.deadzone, 0.0f);
}
