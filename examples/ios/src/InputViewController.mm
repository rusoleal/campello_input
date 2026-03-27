//
//  InputViewController.mm
//  campello_input iOS Example
//

#import "InputViewController.h"
#import "InputView.h"

#include <campello_input/input_system.hpp>
#include <campello_input/gamepad.hpp>
#include <campello_input/keyboard.hpp>
#include <campello_input/mouse.hpp>
#include <campello_input/touch.hpp>

#include <deque>
#include <memory>
#include <mutex>
#include <string>

using namespace systems::leal::campello_input;

// Device observer for connection notifications
struct InputObserver : DeviceObserver {
    std::deque<std::string> *recentEvents = nullptr;
    std::mutex *mutex = nullptr;
    
    void onDeviceConnected(const DeviceInfo& info) override {
        std::lock_guard<std::mutex> lock(*mutex);
        std::string msg = "Connected: " + std::string(info.name);
        recentEvents->push_front(msg);
        if (recentEvents->size() > 10) recentEvents->pop_back();
    }
    
    void onDeviceDisconnected(uint32_t deviceId) override {
        std::lock_guard<std::mutex> lock(*mutex);
        std::string msg = "Disconnected: ID " + std::to_string(deviceId);
        recentEvents->push_front(msg);
        if (recentEvents->size() > 10) recentEvents->pop_back();
    }
};

@interface InputViewController ()
@property (nonatomic, strong) InputView *inputView;
@property (nonatomic, strong) CADisplayLink *displayLink;
@end

@implementation InputViewController {
    std::unique_ptr<InputSystem> _input;
    InputObserver _observer;
    std::deque<std::string> _recentEvents;
    std::mutex _mutex;
}

- (void)viewDidLoad {
    [super viewDidLoad];
    
    self.title = @"Input Monitor";
    self.view.backgroundColor = [UIColor colorWithRed:0.11 green:0.11 blue:0.13 alpha:1.0];
    
    // Initialize input system
    _observer.recentEvents = &_recentEvents;
    _observer.mutex = &_mutex;
    _input = InputSystem::create();
    _input->addObserver(&_observer);
    
    // Create input view
    self.inputView = [[InputView alloc] initWithFrame:self.view.bounds];
    [self.view addSubview:self.inputView];
    
    // Setup display link for 60fps updates
    self.displayLink = [CADisplayLink displayLinkWithTarget:self selector:@selector(updateInput)];
    [self.displayLink addToRunLoop:[NSRunLoop mainRunLoop] forMode:NSDefaultRunLoopMode];
    
    // Setup auto layout
    self.inputView.translatesAutoresizingMaskIntoConstraints = NO;
    [NSLayoutConstraint activateConstraints:@[
        [self.inputView.topAnchor constraintEqualToAnchor:self.view.safeAreaLayoutGuide.topAnchor],
        [self.inputView.leadingAnchor constraintEqualToAnchor:self.view.leadingAnchor],
        [self.inputView.trailingAnchor constraintEqualToAnchor:self.view.trailingAnchor],
        [self.inputView.bottomAnchor constraintEqualToAnchor:self.view.bottomAnchor]
    ]];
}

- (void)dealloc {
    [self.displayLink invalidate];
    if (_input) {
        _input->removeObserver(&_observer);
    }
}

- (void)updateInput {
    if (!_input) return;
    
    _input->update();
    
    InputDisplayState snapshot;
    
    {
        std::lock_guard<std::mutex> lock(_mutex);
        
        // Gamepads
        int count = _input->gamepadCount();
        for (int i = 0; i < count && i < 2; ++i) {
            InputDisplayState::GamepadInfo info;
            auto *gp = _input->gamepadAt(i);
            if (gp) {
                info.connected = (gp->connectionStatus() == ConnectionStatus::connected);
                info.name = gp->name();
                gp->getState(info.state);
                snapshot.gamepads.push_back(std::move(info));
            }
        }
        
        // Keyboard
        auto *kb = _input->keyboard();
        snapshot.keyboard.available = (kb != nullptr);
        if (kb) {
            kb->getState(snapshot.keyboard.state);
            snapshot.keyboard.modifiers = kb->modifiers();
        }
        
        // Mouse
        auto *m = _input->mouse();
        snapshot.mouse.available = (m != nullptr);
        if (m) {
            m->getState(snapshot.mouse.state);
        }
        
        // Touch
        auto *t = _input->touch();
        snapshot.touch.available = (t != nullptr);
        if (t) {
            t->getState(snapshot.touch.state);
        }
        
        // Recent events
        snapshot.recentEvents.assign(_recentEvents.begin(), _recentEvents.end());
    }
    
    [self.inputView updateWithState:snapshot];
}

@end
