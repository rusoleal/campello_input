#import "AppDelegate.h"
#import "InputViewController.h"

@implementation AppDelegate {
    NSWindow            *_window;
    InputViewController *_viewController;
}

- (void)applicationDidFinishLaunching:(NSNotification *)notification {
    NSRect frame = NSMakeRect(0, 0, 1000, 710);

    _window = [[NSWindow alloc]
        initWithContentRect:frame
                  styleMask:NSWindowStyleMaskTitled
                           | NSWindowStyleMaskClosable
                           | NSWindowStyleMaskMiniaturizable
                    backing:NSBackingStoreBuffered
                      defer:NO];

    [_window setTitle:@"campello_input — Input Monitor"];

    _viewController = [[InputViewController alloc] init];
    [_window setContentViewController:_viewController];

    [_window center];
    [_window makeKeyAndOrderFront:nil];
    [_window makeFirstResponder:_viewController.view];
}

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)sender {
    return YES;
}

@end
