//
//  InputView.mm
//  campello_input iOS Example
//

#import "InputView.h"
#include <campello_input/gamepad.hpp>

using namespace systems::leal::campello_input;

// Colors
static UIColor *cBg()       { return [UIColor colorWithRed:0.11 green:0.11 blue:0.13 alpha:1]; }
static UIColor *cCard()     { return [UIColor colorWithRed:0.16 green:0.17 blue:0.19 alpha:1]; }
static UIColor *cTitle()    { return [UIColor colorWithRed:0.92 green:0.92 blue:0.95 alpha:1]; }
static UIColor *cLabel()    { return [UIColor colorWithRed:0.50 green:0.52 blue:0.58 alpha:1]; }
static UIColor *cValue()    { return [UIColor colorWithRed:0.65 green:0.85 blue:1.00 alpha:1]; }
static UIColor *cPressed()  { return [UIColor colorWithRed:0.20 green:0.85 blue:0.45 alpha:1]; }
static UIColor *cOrange()   { return [UIColor colorWithRed:1.00 green:0.60 blue:0.15 alpha:1]; }
static UIColor *cSection()  { return [UIColor colorWithRed:0.55 green:0.68 blue:1.00 alpha:1]; }
static UIColor *cSep()      { return [UIColor colorWithRed:0.25 green:0.27 blue:0.32 alpha:1]; }

@implementation InputView {
    InputDisplayState _state;
    UIFont *_fontUI;
    UIFont *_fontMono;
    UIFont *_fontBold;
    UIFont *_fontSmall;
}

- (instancetype)initWithFrame:(CGRect)frame {
    if (self = [super initWithFrame:frame]) {
        self.backgroundColor = cBg();
        _fontUI    = [UIFont systemFontOfSize:14];
        _fontMono  = [UIFont monospacedSystemFontOfSize:12 weight:UIFontWeightRegular];
        _fontBold  = [UIFont boldSystemFontOfSize:14];
        _fontSmall = [UIFont monospacedSystemFontOfSize:10 weight:UIFontWeightRegular];
    }
    return self;
}

- (void)updateWithState:(const InputDisplayState &)state {
    _state = state;
    [self setNeedsDisplay];
}

- (void)drawRect:(CGRect)rect {
    CGContextRef ctx = UIGraphicsGetCurrentContext();
    
    CGFloat w = self.bounds.size.width;
    CGFloat h = self.bounds.size.height;
    CGFloat pad = 16;
    CGFloat y = pad;
    
    // Title
    [self drawString:@"campello_input — Input Monitor"
                at:CGPointMake(pad, y)
           color:cTitle()
            font:[UIFont boldSystemFontOfSize:18]];
    y += 30;
    [self drawHLineAt:y x:pad width:w - pad * 2 color:cSep()];
    y += 10;
    
    // Calculate layout
    CGFloat contentHeight = h - y - pad;
    CGFloat sectionHeight = contentHeight / 4;  // 4 sections for mobile
    
    // Gamepads
    [self drawGamepadSection:CGRectMake(pad, y, w - pad * 2, sectionHeight)];
    y += sectionHeight + 8;
    
    // Keyboard
    [self drawKeyboardSection:CGRectMake(pad, y, w - pad * 2, sectionHeight * 0.7)];
    y += sectionHeight * 0.7 + 8;
    
    // Mouse
    [self drawMouseSection:CGRectMake(pad, y, (w - pad * 3) / 2, sectionHeight * 0.6)];
    
    // Touch (mobile-specific)
    [self drawTouchSection:CGRectMake(pad * 2 + (w - pad * 3) / 2, y, (w - pad * 3) / 2, sectionHeight * 0.6)];
}

- (void)drawGamepadSection:(CGRect)rect {
    [self drawSectionHeader:@"GAMEPADS" inRect:rect];
    
    if (_state.gamepads.empty()) {
        [self drawString:@"No gamepads connected"
                    at:CGPointMake(rect.origin.x + 10, rect.origin.y + 35)
               color:cLabel()
                font:_fontUI];
        return;
    }
    
    CGFloat y = rect.origin.y + 30;
    CGFloat itemHeight = 100;
    
    for (const auto &gp : _state.gamepads) {
        if (y + itemHeight > rect.origin.y + rect.size.height) break;
        
        // Card background
        UIBezierPath *card = [UIBezierPath bezierPathWithRoundedRect:
            CGRectMake(rect.origin.x, y, rect.size.width, itemHeight - 8) cornerRadius:8];
        [cCard() setFill];
        [card fill];
        
        CGFloat cx = rect.origin.x + 10;
        CGFloat cy = y + 8;
        
        // Name with connection indicator
        UIColor *dot = gp.connected ? [UIColor systemGreenColor] : [UIColor systemRedColor];
        [self drawDotAt:CGPointMake(cx + 5, cy + 8) radius:5 color:dot];
        NSString *name = [NSString stringWithUTF8String:gp.name.c_str()];
        [self drawString:name at:CGPointMake(cx + 16, cy) color:cTitle() font:_fontBold];
        cy += 22;
        
        // Buttons
        auto btn = [&](GamepadButton b) -> BOOL {
            return (gp.state.buttons & static_cast<uint64_t>(b)) != 0;
        };
        
        CGFloat rx = cx;
        [self drawLabel:@"Face" at:CGPointMake(rx, cy)]; rx += 36;
        [self drawButtonLabel:@"Y" pressed:btn(GamepadButton::face_north) at:CGPointMake(rx, cy)]; rx += 28;
        [self drawButtonLabel:@"A" pressed:btn(GamepadButton::face_south) at:CGPointMake(rx, cy)]; rx += 28;
        [self drawButtonLabel:@"X" pressed:btn(GamepadButton::face_west) at:CGPointMake(rx, cy)]; rx += 28;
        [self drawButtonLabel:@"B" pressed:btn(GamepadButton::face_east) at:CGPointMake(rx, cy)]; rx += 40;
        [self drawButtonLabel:@"LB" pressed:btn(GamepadButton::shoulder_left) at:CGPointMake(rx, cy)]; rx += 34;
        [self drawButtonLabel:@"RB" pressed:btn(GamepadButton::shoulder_right) at:CGPointMake(rx, cy)];
        cy += 24;
        
        // Sticks
        rx = cx;
        float lx = gp.state.axes[static_cast<uint8_t>(GamepadAxis::left_x)].value;
        float ly = gp.state.axes[static_cast<uint8_t>(GamepadAxis::left_y)].value;
        float rx_v = gp.state.axes[static_cast<uint8_t>(GamepadAxis::right_x)].value;
        float ry = gp.state.axes[static_cast<uint8_t>(GamepadAxis::right_y)].value;
        
        [self drawLabel:@"L-Stick" at:CGPointMake(rx, cy)]; rx += 55;
        [self drawStick:CGPointMake(lx, ly) at:CGPointMake(rx, cy)]; rx += 40;
        NSString *lStickStr = [NSString stringWithFormat:@"(%+.2f, %+.2f)", lx, ly];
        [self drawString:lStickStr at:CGPointMake(rx, cy + 10) color:cValue() font:_fontSmall];
        rx += 90;
        
        [self drawLabel:@"R-Stick" at:CGPointMake(rx, cy)]; rx += 55;
        [self drawStick:CGPointMake(rx_v, ry) at:CGPointMake(rx, cy)]; rx += 40;
        NSString *rStickStr = [NSString stringWithFormat:@"(%+.2f, %+.2f)", rx_v, ry];
        [self drawString:rStickStr at:CGPointMake(rx, cy + 10) color:cValue() font:_fontSmall];
        
        y += itemHeight;
    }
}

- (void)drawKeyboardSection:(CGRect)rect {
    [self drawSectionHeader:@"KEYBOARD" inRect:rect];
    
    if (!_state.keyboard.available) {
        [self drawString:@"No keyboard detected"
                    at:CGPointMake(rect.origin.x + 10, rect.origin.y + 35)
               color:cLabel()
                font:_fontUI];
        return;
    }
    
    CGFloat x = rect.origin.x + 10;
    CGFloat y = rect.origin.y + 30;
    
    // Modifiers
    auto &mod = _state.keyboard.modifiers;
    auto hasMod = [&](KeyModifier m) -> BOOL { return (mod & m) ? YES : NO; };
    
    CGFloat mx = x;
    [self drawButtonLabel:@"⇧" pressed:hasMod(KeyModifier::shift) at:CGPointMake(mx, y)]; mx += 32;
    [self drawButtonLabel:@"⌃" pressed:hasMod(KeyModifier::ctrl) at:CGPointMake(mx, y)]; mx += 32;
    [self drawButtonLabel:@"⌥" pressed:hasMod(KeyModifier::alt) at:CGPointMake(mx, y)]; mx += 32;
    [self drawButtonLabel:@"⌘" pressed:hasMod(KeyModifier::meta) at:CGPointMake(mx, y)];
}

- (void)drawMouseSection:(CGRect)rect {
    [self drawSectionHeader:@"MOUSE" inRect:rect];
    
    if (!_state.mouse.available) {
        [self drawString:@"No mouse detected"
                    at:CGPointMake(rect.origin.x + 10, rect.origin.y + 35)
               color:cLabel()
                font:_fontUI];
        return;
    }
    
    CGFloat x = rect.origin.x + 10;
    CGFloat y = rect.origin.y + 30;
    const auto &ms = _state.mouse.state;
    
    NSString *pos = [NSString stringWithFormat:@"(%d, %d)", ms.pos_x, ms.pos_y];
    [self drawString:pos at:CGPointMake(x, y) color:cValue() font:_fontMono];
    y += 20;
    
    CGFloat bx = x;
    [self drawButtonLabel:@"L" pressed:(ms.buttons & (1 << 0)) at:CGPointMake(bx, y)]; bx += 30;
    [self drawButtonLabel:@"R" pressed:(ms.buttons & (1 << 1)) at:CGPointMake(bx, y)]; bx += 30;
    [self drawButtonLabel:@"M" pressed:(ms.buttons & (1 << 2)) at:CGPointMake(bx, y)];
}

- (void)drawTouchSection:(CGRect)rect {
    [self drawSectionHeader:@"TOUCH" inRect:rect];
    
    if (!_state.touch.available) {
        [self drawString:@"Touch not available"
                    at:CGPointMake(rect.origin.x + 10, rect.origin.y + 35)
               color:cLabel()
                font:_fontUI];
        return;
    }
    
    CGFloat x = rect.origin.x + 10;
    CGFloat y = rect.origin.y + 30;
    
    NSString *count = [NSString stringWithFormat:@"Touches: %u", _state.touch.state.count];
    [self drawString:count at:CGPointMake(x, y) color:cValue() font:_fontMono];
}

// Drawing helpers

- (void)drawSectionHeader:(NSString *)title inRect:(CGRect)rect {
    [self drawString:title
                at:CGPointMake(rect.origin.x, rect.origin.y + 5)
           color:cSection()
            font:[UIFont boldSystemFontOfSize:12]];
    [self drawHLineAt:rect.origin.y + 22 x:rect.origin.x width:rect.size.width color:cSep()];
}

- (void)drawLabel:(NSString *)text at:(CGPoint)pt {
    [self drawString:text at:pt color:cLabel() font:_fontUI];
}

- (void)drawString:(NSString *)s at:(CGPoint)pt color:(UIColor *)color font:(UIFont *)font {
    NSDictionary *attrs = @{
        NSForegroundColorAttributeName: color,
        NSFontAttributeName: font
    };
    [s drawAtPoint:pt withAttributes:attrs];
}

- (void)drawButtonLabel:(NSString *)label pressed:(BOOL)pressed at:(CGPoint)pt {
    CGSize sz = [label sizeWithAttributes:@{NSFontAttributeName: _fontSmall}];
    CGFloat pad = 4;
    CGRect rect = CGRectMake(pt.x, pt.y, sz.width + pad * 2, 20);
    
    UIBezierPath *path = [UIBezierPath bezierPathWithRoundedRect:rect cornerRadius:4];
    
    if (pressed) {
        [cPressed() setFill];
        [path fill];
        [self drawString:label
                    at:CGPointMake(pt.x + pad, pt.y + 2)
               color:[UIColor blackColor]
                font:_fontSmall];
    } else {
        [[cLabel() colorWithAlphaComponent:0.12] setFill];
        [path fill];
        [[cLabel() colorWithAlphaComponent:0.5] setStroke];
        [path stroke];
        [self drawString:label
                    at:CGPointMake(pt.x + pad, pt.y + 2)
               color:cLabel()
                font:_fontSmall];
    }
}

- (void)drawStick:(CGPoint)v at:(CGPoint)pt {
    const CGFloat sz = 30;
    CGRect box = CGRectMake(pt.x, pt.y, sz, sz);
    
    // Background
    [[cLabel() colorWithAlphaComponent:0.15] setFill];
    [[UIBezierPath bezierPathWithOvalInRect:box] fill];
    [[cSep() colorWithAlphaComponent:0.8] setStroke];
    [[UIBezierPath bezierPathWithOvalInRect:box] stroke];
    
    // Crosshair
    [cSep() setStroke];
    UIBezierPath *h = [UIBezierPath bezierPath];
    [h moveToPoint:CGPointMake(pt.x + 3, pt.y + sz/2)];
    [h addLineToPoint:CGPointMake(pt.x + sz - 3, pt.y + sz/2)];
    [h stroke];
    UIBezierPath *vert = [UIBezierPath bezierPath];
    [vert moveToPoint:CGPointMake(pt.x + sz/2, pt.y + 3)];
    [vert addLineToPoint:CGPointMake(pt.x + sz/2, pt.y + sz - 3)];
    [vert stroke];
    
    // Position dot
    CGFloat r = 4;
    CGFloat dx = pt.x + sz/2 + v.x * (sz/2 - r - 1);
    CGFloat dy = pt.y + sz/2 + v.y * (sz/2 - r - 1);
    CGRect dot = CGRectMake(dx - r, dy - r, r*2, r*2);
    [cPressed() setFill];
    [[UIBezierPath bezierPathWithOvalInRect:dot] fill];
}

- (void)drawDotAt:(CGPoint)center radius:(CGFloat)r color:(UIColor *)color {
    CGRect rect = CGRectMake(center.x - r, center.y - r, r*2, r*2);
    [color setFill];
    [[UIBezierPath bezierPathWithOvalInRect:rect] fill];
}

- (void)drawHLineAt:(CGFloat)y x:(CGFloat)x width:(CGFloat)w color:(UIColor *)color {
    [color setFill];
    UIRectFill(CGRectMake(x, y, w, 1));
}

@end
