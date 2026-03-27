//
//  InputView.h
//  campello_input iOS Example
//

#import <UIKit/UIKit.h>
#include <campello_input/gamepad.hpp>
#include <campello_input/keyboard.hpp>
#include <campello_input/mouse.hpp>
#include <campello_input/touch.hpp>
#include <string>
#include <vector>

// Display state snapshot
struct InputDisplayState {
    struct GamepadInfo {
        bool connected = false;
        std::string name;
        systems::leal::campello_input::GamepadState state{};
    };
    std::vector<GamepadInfo> gamepads;
    
    struct KeyboardInfo {
        bool available = false;
        systems::leal::campello_input::KeyboardState state{};
        systems::leal::campello_input::KeyModifier modifiers
            = systems::leal::campello_input::KeyModifier::none;
    } keyboard;
    
    struct MouseInfo {
        bool available = false;
        systems::leal::campello_input::MouseState state{};
    } mouse;
    
    struct TouchInfo {
        bool available = false;
        systems::leal::campello_input::TouchState state{};
    } touch;
    
    std::vector<std::string> recentEvents;
};

@interface InputView : UIView
- (void)updateWithState:(const InputDisplayState &)state;
@end
