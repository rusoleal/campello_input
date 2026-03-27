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
#include <campello_input/haptics.hpp>

using namespace systems::leal::campello_input;

// =============================================================================
// GamepadButton Tests
// =============================================================================

TEST(GamepadButtonTest, ValuesArePowerOfTwo) {
    // Each button should be a unique power of two for bitmask operations
    EXPECT_EQ(static_cast<uint64_t>(GamepadButton::face_south), 1ULL << 0);
    EXPECT_EQ(static_cast<uint64_t>(GamepadButton::face_east), 1ULL << 1);
    EXPECT_EQ(static_cast<uint64_t>(GamepadButton::face_west), 1ULL << 2);
    EXPECT_EQ(static_cast<uint64_t>(GamepadButton::face_north), 1ULL << 3);
    EXPECT_EQ(static_cast<uint64_t>(GamepadButton::shoulder_left), 1ULL << 4);
    EXPECT_EQ(static_cast<uint64_t>(GamepadButton::shoulder_right), 1ULL << 5);
}

TEST(GamepadButtonTest, BitmaskOrOperation) {
    uint64_t mask = 0;
    mask |= static_cast<uint64_t>(GamepadButton::face_south);
    mask |= static_cast<uint64_t>(GamepadButton::face_east);

    EXPECT_TRUE(mask & GamepadButton::face_south);
    EXPECT_TRUE(mask & GamepadButton::face_east);
    EXPECT_FALSE(mask & GamepadButton::face_west);
    EXPECT_FALSE(mask & GamepadButton::face_north);
}

TEST(GamepadButtonTest, BitmaskAndWithUint64) {
    uint64_t mask = static_cast<uint64_t>(GamepadButton::face_south) |
                    static_cast<uint64_t>(GamepadButton::face_north);

    // Test the operator& overload
    EXPECT_TRUE(mask & GamepadButton::face_south);
    EXPECT_TRUE(mask & GamepadButton::face_north);
    EXPECT_FALSE(mask & GamepadButton::face_east);
    EXPECT_FALSE(mask & GamepadButton::face_west);
}

TEST(GamepadButtonTest, MultipleButtonsInMask) {
    uint64_t mask = 0;
    mask |= static_cast<uint64_t>(GamepadButton::dpad_up);
    mask |= static_cast<uint64_t>(GamepadButton::dpad_down);
    mask |= static_cast<uint64_t>(GamepadButton::dpad_left);
    mask |= static_cast<uint64_t>(GamepadButton::dpad_right);

    EXPECT_TRUE(mask & GamepadButton::dpad_up);
    EXPECT_TRUE(mask & GamepadButton::dpad_down);
    EXPECT_TRUE(mask & GamepadButton::dpad_left);
    EXPECT_TRUE(mask & GamepadButton::dpad_right);
    EXPECT_FALSE(mask & GamepadButton::face_south);
}

// =============================================================================
// GamepadAxis Tests
// =============================================================================

TEST(GamepadAxisTest, SequentialValues) {
    EXPECT_EQ(static_cast<uint8_t>(GamepadAxis::left_x), 0);
    EXPECT_EQ(static_cast<uint8_t>(GamepadAxis::left_y), 1);
    EXPECT_EQ(static_cast<uint8_t>(GamepadAxis::right_x), 2);
    EXPECT_EQ(static_cast<uint8_t>(GamepadAxis::right_y), 3);
    EXPECT_EQ(static_cast<uint8_t>(GamepadAxis::trigger_left), 4);
    EXPECT_EQ(static_cast<uint8_t>(GamepadAxis::trigger_right), 5);
}

TEST(GamepadAxisTest, CountMatchesNumberOfAxes) {
    // Count should be the number of axes defined
    EXPECT_EQ(static_cast<uint8_t>(GamepadAxis::count), 6);
}

// =============================================================================
// KeyModifier Tests
// =============================================================================

TEST(KeyModifierTest, BitwiseOrOperation) {
    KeyModifier mods = KeyModifier::shift | KeyModifier::ctrl;

    EXPECT_TRUE(mods & KeyModifier::shift);
    EXPECT_TRUE(mods & KeyModifier::ctrl);
    EXPECT_FALSE(mods & KeyModifier::alt);
    EXPECT_FALSE(mods & KeyModifier::meta);
}

TEST(KeyModifierTest, BitwiseAndOperation) {
    KeyModifier mods = KeyModifier::shift | KeyModifier::alt | KeyModifier::meta;

    EXPECT_TRUE(mods & KeyModifier::shift);
    EXPECT_FALSE(mods & KeyModifier::ctrl);
    EXPECT_TRUE(mods & KeyModifier::alt);
    EXPECT_TRUE(mods & KeyModifier::meta);
}

TEST(KeyModifierTest, CombinedModifiers) {
    KeyModifier mods = KeyModifier::none;
    mods = mods | KeyModifier::shift;
    mods = mods | KeyModifier::ctrl;
    mods = mods | KeyModifier::alt;
    mods = mods | KeyModifier::meta;

    EXPECT_TRUE(mods & KeyModifier::shift);
    EXPECT_TRUE(mods & KeyModifier::ctrl);
    EXPECT_TRUE(mods & KeyModifier::alt);
    EXPECT_TRUE(mods & KeyModifier::meta);
}

TEST(KeyModifierTest, NoneIsZero) {
    EXPECT_EQ(static_cast<uint16_t>(KeyModifier::none), 0);
}

// =============================================================================
// HapticsCapability Tests
// =============================================================================

TEST(HapticsCapabilityTest, BitwiseOrOperation) {
    HapticsCapability caps = HapticsCapability::rumble |
                              HapticsCapability::trigger_left |
                              HapticsCapability::trigger_right;

    EXPECT_TRUE(caps & HapticsCapability::rumble);
    EXPECT_TRUE(caps & HapticsCapability::trigger_left);
    EXPECT_TRUE(caps & HapticsCapability::trigger_right);
}

TEST(HapticsCapabilityTest, NoneIsZero) {
    EXPECT_EQ(static_cast<uint32_t>(HapticsCapability::none), 0);
}

TEST(HapticsCapabilityTest, IndividualCapabilities) {
    HapticsCapability caps = HapticsCapability::rumble;
    EXPECT_TRUE(caps & HapticsCapability::rumble);
    EXPECT_FALSE(caps & HapticsCapability::trigger_left);
    EXPECT_FALSE(caps & HapticsCapability::trigger_right);
}

// =============================================================================
// DeviceType Tests
// =============================================================================

TEST(DeviceTypeTest, ValuesAreDistinct) {
    // Ensure each device type has a unique value
    EXPECT_NE(static_cast<uint8_t>(DeviceType::gamepad),
              static_cast<uint8_t>(DeviceType::keyboard));
    EXPECT_NE(static_cast<uint8_t>(DeviceType::keyboard),
              static_cast<uint8_t>(DeviceType::mouse));
    EXPECT_NE(static_cast<uint8_t>(DeviceType::mouse),
              static_cast<uint8_t>(DeviceType::touch));
}

// =============================================================================
// ConnectionStatus Tests
// =============================================================================

TEST(ConnectionStatusTest, ValuesAreDistinct) {
    EXPECT_NE(static_cast<uint8_t>(ConnectionStatus::connected),
              static_cast<uint8_t>(ConnectionStatus::disconnected));
}
