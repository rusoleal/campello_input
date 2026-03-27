#import "InputViewController.h"
#import "InputView.h"

#import <GameController/GameController.h>

#include <campello_input/input_system.hpp>
#include <campello_input/keyboard_apple.hpp>  // For nativeHandle() - platform-specific

#include <deque>
#include <memory>
#include <mutex>
#include <string>

using namespace systems::leal::campello_input;

// ---------------------------------------------------------------------------
// KeyCode → display name
// ---------------------------------------------------------------------------

static std::string keyCodeToName(KeyCode code) {
    using K = KeyCode;
    switch (code) {
        case K::a: return "A";   case K::b: return "B";   case K::c: return "C";
        case K::d: return "D";   case K::e: return "E";   case K::f: return "F";
        case K::g: return "G";   case K::h: return "H";   case K::i: return "I";
        case K::j: return "J";   case K::k: return "K";   case K::l: return "L";
        case K::m: return "M";   case K::n: return "N";   case K::o: return "O";
        case K::p: return "P";   case K::q: return "Q";   case K::r: return "R";
        case K::s: return "S";   case K::t: return "T";   case K::u: return "U";
        case K::v: return "V";   case K::w: return "W";   case K::x: return "X";
        case K::y: return "Y";   case K::z: return "Z";
        case K::n1: return "1";  case K::n2: return "2";  case K::n3: return "3";
        case K::n4: return "4";  case K::n5: return "5";  case K::n6: return "6";
        case K::n7: return "7";  case K::n8: return "8";  case K::n9: return "9";
        case K::n0: return "0";
        case K::enter:     return "Return";
        case K::escape:    return "Escape";
        case K::backspace: return "Backspace";
        case K::tab:       return "Tab";
        case K::space:     return "Space";
        case K::f1:  return "F1";  case K::f2:  return "F2";  case K::f3:  return "F3";
        case K::f4:  return "F4";  case K::f5:  return "F5";  case K::f6:  return "F6";
        case K::f7:  return "F7";  case K::f8:  return "F8";  case K::f9:  return "F9";
        case K::f10: return "F10"; case K::f11: return "F11"; case K::f12: return "F12";
        case K::arrow_up:    return "↑";
        case K::arrow_down:  return "↓";
        case K::arrow_left:  return "←";
        case K::arrow_right: return "→";
        case K::shift_left:  case K::shift_right: return "Shift";
        case K::ctrl_left:   case K::ctrl_right:  return "Ctrl";
        case K::alt_left:    case K::alt_right:   return "Alt";
        case K::meta_left:   case K::meta_right:  return "Cmd";
        case K::caps_lock:   return "Caps";
        case K::del:         return "Delete";
        case K::home:        return "Home";
        case K::end:         return "End";
        case K::page_up:     return "PgUp";
        case K::page_down:   return "PgDn";
        case K::insert:      return "Insert";
        case K::minus:       return "-";
        case K::equals:      return "=";
        case K::backslash:   return "\\";
        case K::slash:       return "/";
        case K::period:      return ".";
        case K::comma:       return ",";
        case K::semicolon:   return ";";
        case K::apostrophe:  return "'";
        case K::grave:       return "`";
        case K::bracket_left:  return "[";
        case K::bracket_right: return "]";
        default: return "?";
    }
}

// ---------------------------------------------------------------------------
// DeviceObserver — wires the keyboard key-press handler for "recent keys"
// ---------------------------------------------------------------------------

struct InputVCObserver : DeviceObserver {
    std::deque<std::string> *recentKeys = nullptr;
    std::mutex              *mutex      = nullptr;

    void onDeviceConnected(Device &device) override {
        if (device.type() != DeviceType::keyboard) return;
        if (@available(macOS 11.0, *)) {
            auto &kb    = static_cast<AppleKeyboard &>(device);
            GCKeyboard *gcKb = (__bridge GCKeyboard *)kb.nativeHandle();
            gcKb.keyboardInput.keyChangedHandler =
                ^(GCKeyboardInput * /*input*/, GCControllerButtonInput * /*btn*/,
                  GCKeyCode keyCode, BOOL pressed)
                {
                    if (!pressed) return;
                    std::string name = keyCodeToName(static_cast<KeyCode>(keyCode));
                    std::lock_guard<std::mutex> lock(*mutex);
                    recentKeys->push_front(name);
                    if (recentKeys->size() > 10) recentKeys->pop_back();
                };
        }
    }

    void onDeviceDisconnected(Device &device) override { (void)device; }
};

// ---------------------------------------------------------------------------
// InputViewController
// ---------------------------------------------------------------------------

@interface InputViewController ()
@property (nonatomic) InputView *inputView;
@property (nonatomic) NSTimer   *pollTimer;
@end

@implementation InputViewController {
    std::unique_ptr<InputSystem> _input;
    InputVCObserver             _observer;
    std::deque<std::string>     _recentKeys;
    std::mutex                  _recentKeysMutex;
}

- (void)loadView {
    self.inputView = [[InputView alloc] initWithFrame:NSMakeRect(0, 0, 1000, 710)];
    self.view = self.inputView;
}

- (void)viewDidLoad {
    [super viewDidLoad];
    _observer.recentKeys = &_recentKeys;
    _observer.mutex      = &_recentKeysMutex;
    _input = InputSystem::create();  // Unified API - works on all platforms
    _input->addObserver(&_observer);
    [self startPolling];
}

// ---------------------------------------------------------------------------
// Poll loop (60 Hz)
// ---------------------------------------------------------------------------

- (void)startPolling {
    self.pollTimer = [NSTimer scheduledTimerWithTimeInterval:(1.0 / 60.0)
                                                      target:self
                                                    selector:@selector(poll)
                                                    userInfo:nil
                                                     repeats:YES];
}

- (void)poll {
    InputDisplayState snapshot;

    {
        std::lock_guard<std::mutex> lock(_recentKeysMutex);

        // Gamepads — show at most 3; fold the rest into a count row
        int count = _input->gamepadCount();
        for (int i = 0; i < count; ++i) {
            InputDisplayState::GamepadInfo info;
            auto *gp = _input->gamepadAt(i);

            if (i == 2 && count > 3) {
                info.name      = "... and " + std::to_string(count - 2) + " more";
                info.connected = true;
                snapshot.gamepads.push_back(std::move(info));
                break;
            }

            info.connected = (gp->connectionStatus() == ConnectionStatus::connected);
            info.name      = gp->name();
            gp->getState(info.state);
            snapshot.gamepads.push_back(std::move(info));

            if (i == 2) break;   // max 3 displayed
        }

        // Keyboard
        auto *kb = _input->keyboard();
        snapshot.keyboard.available = (kb != nullptr);
        if (kb) {
            kb->getState(snapshot.keyboard.state);
            snapshot.keyboard.modifiers = kb->modifiers();
        }
        snapshot.keyboard.recentKeys.assign(_recentKeys.begin(), _recentKeys.end());

        // Mouse
        auto *m = _input->mouse();
        snapshot.mouse.available = (m != nullptr);
        if (m) {
            m->getState(snapshot.mouse.state);
        }
    }

    [self.inputView updateWithState:snapshot];
}

// ---------------------------------------------------------------------------

- (void)dealloc {
    [self.pollTimer invalidate];
    if (_input) _input->removeObserver(&_observer);
}

@end
