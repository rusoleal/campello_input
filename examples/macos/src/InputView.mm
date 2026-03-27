#import "InputView.h"

using namespace systems::leal::campello_input;

// ---------------------------------------------------------------------------
// Layout constants
// ---------------------------------------------------------------------------

static const CGFloat kPad           = 16.0;
static const CGFloat kTitleH        = 54.0;
static const CGFloat kSectionHeadH  = 30.0;
static const CGFloat kGamepadItemH  = 130.0;
static const CGFloat kGamepadAreaH  = 3 * kGamepadItemH + kSectionHeadH + 4;
static const CGFloat kBottomH       = 220.0;

// ---------------------------------------------------------------------------
// Color helpers
// ---------------------------------------------------------------------------

static NSColor *cBg()       { return [NSColor colorWithRed:0.11 green:0.11 blue:0.13 alpha:1]; }
static NSColor *cCard()     { return [NSColor colorWithRed:0.16 green:0.17 blue:0.19 alpha:1]; }
static NSColor *cTitle()    { return [NSColor colorWithRed:0.92 green:0.92 blue:0.95 alpha:1]; }
static NSColor *cLabel()    { return [NSColor colorWithRed:0.50 green:0.52 blue:0.58 alpha:1]; }
static NSColor *cValue()    { return [NSColor colorWithRed:0.65 green:0.85 blue:1.00 alpha:1]; }
static NSColor *cPressed()  { return [NSColor colorWithRed:0.20 green:0.85 blue:0.45 alpha:1]; }
static NSColor *cOrange()   { return [NSColor colorWithRed:1.00 green:0.60 blue:0.15 alpha:1]; }
static NSColor *cSection()  { return [NSColor colorWithRed:0.55 green:0.68 blue:1.00 alpha:1]; }
static NSColor *cSep()      { return [NSColor colorWithRed:0.25 green:0.27 blue:0.32 alpha:1]; }

// ---------------------------------------------------------------------------

@implementation InputView {
    InputDisplayState _state;
    NSFont *_fontUI;
    NSFont *_fontMono;
    NSFont *_fontBold;
    NSFont *_fontSmall;
}

- (instancetype)initWithFrame:(NSRect)frame {
    if (self = [super initWithFrame:frame]) {
        self.wantsLayer = YES;
        self.layerContentsRedrawPolicy = NSViewLayerContentsRedrawOnSetNeedsDisplay;
        _fontUI    = [NSFont systemFontOfSize:13];
        _fontMono  = [NSFont monospacedSystemFontOfSize:12 weight:NSFontWeightRegular];
        _fontBold  = [NSFont boldSystemFontOfSize:13];
        _fontSmall = [NSFont monospacedSystemFontOfSize:11 weight:NSFontWeightRegular];
    }
    return self;
}

- (BOOL)isFlipped { return YES; }   // y=0 at top

- (void)updateWithState:(const InputDisplayState &)state {
    _state = state;
    [self setNeedsDisplay:YES];
}

// ---------------------------------------------------------------------------
// Main draw entry
// ---------------------------------------------------------------------------

- (void)drawRect:(NSRect)dirtyRect {
    [cBg() setFill];
    NSRectFill(dirtyRect);

    CGFloat w = self.bounds.size.width;
    CGFloat y = kPad;

    // Title
    [self str:@"campello_input  —  Input Monitor"
           at:NSMakePoint(kPad, y) color:cTitle()
         font:[NSFont boldSystemFontOfSize:17]];
    y += 28;
    [self hline:y x:kPad width:w - kPad * 2 color:cSep()];
    y += 10;

    // Gamepads
    y = [self drawGamepadSection:y width:w];

    // Separator between gamepads and bottom row
    [self hline:y x:kPad width:w - kPad * 2 color:cSep()];
    y += 10;

    // Keyboard (left) + Mouse (right)
    CGFloat halfW = (w - kPad * 3) / 2.0;
    [self drawKeyboardSection:NSMakePoint(kPad, y)          size:NSMakeSize(halfW, kBottomH)];
    [self drawMouseSection:NSMakePoint(kPad * 2 + halfW, y) size:NSMakeSize(halfW, kBottomH)];
}

// ---------------------------------------------------------------------------
// Gamepads section
// ---------------------------------------------------------------------------

- (CGFloat)drawGamepadSection:(CGFloat)y width:(CGFloat)w {
    [self sectionHeader:@"GAMEPADS" x:kPad y:y width:w - kPad * 2];
    y += kSectionHeadH;

    if (_state.gamepads.empty()) {
        [self str:@"No gamepads connected.  Plug in a controller or connect via Bluetooth."
               at:NSMakePoint(kPad * 2, y) color:cLabel() font:_fontUI];
        y += 28;
        return y + 4;
    }

    for (const auto &gp : _state.gamepads) {
        y = [self drawGamepad:gp y:y width:w];
        y += 6;
    }
    return y + 4;
}

- (CGFloat)drawGamepad:(const InputDisplayState::GamepadInfo &)gp
                     y:(CGFloat)y
                 width:(CGFloat)w {
    // Card background
    NSRect card = NSMakeRect(kPad, y, w - kPad * 2, kGamepadItemH - 8);
    [[cCard() colorWithAlphaComponent:0.85] setFill];
    [[NSBezierPath bezierPathWithRoundedRect:card xRadius:8 yRadius:8] fill];

    CGFloat cx = kPad + 12;
    CGFloat cy = y + 10;

    // Connection dot + name
    NSColor *dot = gp.connected ? [NSColor systemGreenColor] : [NSColor systemRedColor];
    [self dot:NSMakePoint(cx + 4, cy + 7) radius:5 color:dot];
    [self str:[NSString stringWithUTF8String:gp.name.c_str()]
           at:NSMakePoint(cx + 14, cy) color:cTitle() font:_fontBold];
    cy += 24;

    using B = GamepadButton;
    using A = GamepadAxis;
    auto btn = [&](B b) -> BOOL {
        return (gp.state.buttons & static_cast<uint64_t>(b)) != 0;
    };

    // Row 1 — Face buttons + shoulders + thumb clicks
    CGFloat rx = cx;
    [self label:@"Face" at:NSMakePoint(rx, cy)]; rx += 36;
    [self btnLabel:@"Y"  pressed:btn(B::face_north)  at:NSMakePoint(rx, cy)]; rx += 30;
    [self btnLabel:@"X"  pressed:btn(B::face_west)   at:NSMakePoint(rx, cy)]; rx += 30;
    [self btnLabel:@"A"  pressed:btn(B::face_south)  at:NSMakePoint(rx, cy)]; rx += 30;
    [self btnLabel:@"B"  pressed:btn(B::face_east)   at:NSMakePoint(rx, cy)]; rx += 40;
    [self btnLabel:@"LB" pressed:btn(B::shoulder_left)  at:NSMakePoint(rx, cy)]; rx += 34;
    [self btnLabel:@"RB" pressed:btn(B::shoulder_right) at:NSMakePoint(rx, cy)]; rx += 42;
    [self btnLabel:@"L3" pressed:btn(B::thumb_left)  at:NSMakePoint(rx, cy)]; rx += 34;
    [self btnLabel:@"R3" pressed:btn(B::thumb_right) at:NSMakePoint(rx, cy)]; rx += 44;
    [self btnLabel:@"Start"  pressed:btn(B::start)  at:NSMakePoint(rx, cy)]; rx += 52;
    [self btnLabel:@"Select" pressed:btn(B::select) at:NSMakePoint(rx, cy)]; rx += 56;
    [self btnLabel:@"Home"   pressed:btn(B::home)   at:NSMakePoint(rx, cy)];
    cy += 26;

    // Row 2 — D-Pad + triggers
    rx = cx;
    [self label:@"D-Pad" at:NSMakePoint(rx, cy)]; rx += 48;
    [self btnLabel:@"↑" pressed:btn(B::dpad_up)    at:NSMakePoint(rx, cy)]; rx += 26;
    [self btnLabel:@"↓" pressed:btn(B::dpad_down)  at:NSMakePoint(rx, cy)]; rx += 26;
    [self btnLabel:@"←" pressed:btn(B::dpad_left)  at:NSMakePoint(rx, cy)]; rx += 26;
    [self btnLabel:@"→" pressed:btn(B::dpad_right) at:NSMakePoint(rx, cy)]; rx += 44;

    float lt = gp.state.axes[static_cast<uint8_t>(A::trigger_left)].value;
    float rt = gp.state.axes[static_cast<uint8_t>(A::trigger_right)].value;

    [self label:@"LT" at:NSMakePoint(rx, cy)]; rx += 22;
    [self bar:lt at:NSMakePoint(rx, cy) width:90 color:cOrange()]; rx += 96;
    [self str:[NSString stringWithFormat:@"%.2f", lt]
           at:NSMakePoint(rx, cy) color:cValue() font:_fontMono]; rx += 44;

    [self label:@"RT" at:NSMakePoint(rx, cy)]; rx += 22;
    [self bar:rt at:NSMakePoint(rx, cy) width:90 color:cOrange()]; rx += 96;
    [self str:[NSString stringWithFormat:@"%.2f", rt]
           at:NSMakePoint(rx, cy) color:cValue() font:_fontMono];
    cy += 28;

    // Row 3 — Sticks
    rx = cx;
    float lx = gp.state.axes[static_cast<uint8_t>(A::left_x)].value;
    float ly = gp.state.axes[static_cast<uint8_t>(A::left_y)].value;
    float rrx = gp.state.axes[static_cast<uint8_t>(A::right_x)].value;
    float ry = gp.state.axes[static_cast<uint8_t>(A::right_y)].value;

    [self label:@"L-Stick" at:NSMakePoint(rx, cy)]; rx += 58;
    [self stick:CGPointMake(lx, ly) at:NSMakePoint(rx, cy)]; rx += 42;
    [self str:[NSString stringWithFormat:@"(%+.2f, %+.2f)", lx, ly]
           at:NSMakePoint(rx, cy) color:cValue() font:_fontMono]; rx += 130;

    [self label:@"R-Stick" at:NSMakePoint(rx, cy)]; rx += 58;
    [self stick:CGPointMake(rrx, ry) at:NSMakePoint(rx, cy)]; rx += 42;
    [self str:[NSString stringWithFormat:@"(%+.2f, %+.2f)", rrx, ry]
           at:NSMakePoint(rx, cy) color:cValue() font:_fontMono];

    return y + kGamepadItemH - 8;
}

// ---------------------------------------------------------------------------
// Keyboard section
// ---------------------------------------------------------------------------

- (void)drawKeyboardSection:(NSPoint)origin size:(NSSize)sz {
    [self sectionHeader:@"KEYBOARD" x:origin.x y:origin.y width:sz.width];

    NSRect card = NSMakeRect(origin.x, origin.y + kSectionHeadH,
                             sz.width, sz.height - kSectionHeadH - kPad);
    [[cCard() colorWithAlphaComponent:0.85] setFill];
    [[NSBezierPath bezierPathWithRoundedRect:card xRadius:8 yRadius:8] fill];

    CGFloat x = origin.x + 12;
    CGFloat y = origin.y + kSectionHeadH + 12;

    if (!_state.keyboard.available) {
        [self str:@"No keyboard detected."
               at:NSMakePoint(x, y) color:cLabel() font:_fontUI];
        [self str:@"Requires macOS 11 + Game Controller framework."
               at:NSMakePoint(x, y + 18) color:cLabel() font:_fontSmall];
        return;
    }

    // Modifier keys
    using M = KeyModifier;
    auto &mod = _state.keyboard.modifiers;
    auto hasMod = [&](M m) -> BOOL { return (mod & m) ? YES : NO; };

    [self label:@"Mods" at:NSMakePoint(x, y)];
    CGFloat mx = x + 40;
    [self btnLabel:@"⇧"   pressed:hasMod(M::shift)     at:NSMakePoint(mx, y)]; mx += 26;
    [self btnLabel:@"⌃"   pressed:hasMod(M::ctrl)      at:NSMakePoint(mx, y)]; mx += 26;
    [self btnLabel:@"⌥"   pressed:hasMod(M::alt)       at:NSMakePoint(mx, y)]; mx += 26;
    [self btnLabel:@"⌘"   pressed:hasMod(M::meta)      at:NSMakePoint(mx, y)]; mx += 26;
    [self btnLabel:@"⇪"   pressed:hasMod(M::caps_lock) at:NSMakePoint(mx, y)];
    y += 30;

    // Recent keys
    [self label:@"Recent" at:NSMakePoint(x, y)];
    y += 20;

    NSMutableString *line = [NSMutableString string];
    int count = 0;
    for (const auto &k : _state.keyboard.recentKeys) {
        if (count > 0) [line appendString:@"  "];
        [line appendString:[NSString stringWithUTF8String:k.c_str()]];
        if (++count >= 10) break;
    }
    if (!line.length) [line appendString:@"—"];

    // Wrap text to card width
    CGFloat maxW = sz.width - 24;
    NSRect textRect = NSMakeRect(x, y, maxW, 60);
    NSDictionary *attrs = @{ NSFontAttributeName: _fontMono,
                             NSForegroundColorAttributeName: cValue() };
    [line drawInRect:textRect withAttributes:attrs];
}

// ---------------------------------------------------------------------------
// Mouse section
// ---------------------------------------------------------------------------

- (void)drawMouseSection:(NSPoint)origin size:(NSSize)sz {
    [self sectionHeader:@"MOUSE" x:origin.x y:origin.y width:sz.width];

    NSRect card = NSMakeRect(origin.x, origin.y + kSectionHeadH,
                             sz.width, sz.height - kSectionHeadH - kPad);
    [[cCard() colorWithAlphaComponent:0.85] setFill];
    [[NSBezierPath bezierPathWithRoundedRect:card xRadius:8 yRadius:8] fill];

    CGFloat x = origin.x + 12;
    CGFloat y = origin.y + kSectionHeadH + 12;

    if (!_state.mouse.available) {
        [self str:@"No mouse detected."
               at:NSMakePoint(x, y) color:cLabel() font:_fontUI];
        [self str:@"Requires macOS 11 + Game Controller framework."
               at:NSMakePoint(x, y + 18) color:cLabel() font:_fontSmall];
        return;
    }

    const auto &ms = _state.mouse.state;

    auto row = [&](NSString *lbl, NSString *val) {
        [self label:lbl at:NSMakePoint(x, y)];
        [self str:val at:NSMakePoint(x + 72, y) color:cValue() font:_fontMono];
        y += 22;
    };

    row(@"Position",
        [NSString stringWithFormat:@"( %.1f,  %.1f )", ms.x, ms.y]);
    row(@"Delta",
        [NSString stringWithFormat:@"( %+.1f, %+.1f )", ms.deltaX, ms.deltaY]);
    row(@"Scroll",
        [NSString stringWithFormat:@"( %+.1f, %+.1f )", ms.scrollX, ms.scrollY]);

    y += 4;
    [self label:@"Buttons" at:NSMakePoint(x, y)];
    CGFloat bx = x + 72;
    [self btnLabel:@"L"  pressed:(ms.buttons & (1 << 0)) at:NSMakePoint(bx, y)]; bx += 28;
    [self btnLabel:@"R"  pressed:(ms.buttons & (1 << 1)) at:NSMakePoint(bx, y)]; bx += 28;
    [self btnLabel:@"M"  pressed:(ms.buttons & (1 << 2)) at:NSMakePoint(bx, y)]; bx += 28;
    [self btnLabel:@"X1" pressed:(ms.buttons & (1 << 3)) at:NSMakePoint(bx, y)]; bx += 34;
    [self btnLabel:@"X2" pressed:(ms.buttons & (1 << 4)) at:NSMakePoint(bx, y)];
}

// ---------------------------------------------------------------------------
// Drawing primitives
// ---------------------------------------------------------------------------

- (void)sectionHeader:(NSString *)title x:(CGFloat)x y:(CGFloat)y width:(CGFloat)w {
    [self str:title at:NSMakePoint(x, y)
         color:cSection() font:[NSFont boldSystemFontOfSize:11]];
    [self hline:y + 18 x:x width:w color:cSep()];
}

- (void)btnLabel:(NSString *)label pressed:(BOOL)pressed at:(NSPoint)pt {
    NSSize sz = [label sizeWithAttributes:@{ NSFontAttributeName: _fontSmall }];
    CGFloat pad = 4;
    NSRect rect = NSMakeRect(pt.x, pt.y, sz.width + pad * 2, 17);
    NSBezierPath *path = [NSBezierPath bezierPathWithRoundedRect:rect xRadius:3 yRadius:3];

    if (pressed) {
        [cPressed() setFill];
        [path fill];
        [self str:label at:NSMakePoint(pt.x + pad, pt.y + 1)
             color:[NSColor blackColor] font:_fontSmall];
    } else {
        [[cLabel() colorWithAlphaComponent:0.12] setFill];
        [path fill];
        [[cLabel() colorWithAlphaComponent:0.50] setStroke];
        [path stroke];
        [self str:label at:NSMakePoint(pt.x + pad, pt.y + 1)
             color:cLabel() font:_fontSmall];
    }
}

// Horizontal trigger / axis bar  (value in [0,1])
- (void)bar:(float)value at:(NSPoint)pt width:(CGFloat)w color:(NSColor *)col {
    NSRect bg = NSMakeRect(pt.x, pt.y + 3, w, 12);
    [[cLabel() colorWithAlphaComponent:0.15] setFill];
    [[NSBezierPath bezierPathWithRoundedRect:bg xRadius:3 yRadius:3] fill];

    if (value > 0.001f) {
        NSRect fill = NSMakeRect(pt.x, pt.y + 3, w * value, 12);
        [[col colorWithAlphaComponent:0.85] setFill];
        [[NSBezierPath bezierPathWithRoundedRect:fill xRadius:3 yRadius:3] fill];
    }
}

// Thumbstick visualiser — circle with crosshair + dot
- (void)stick:(CGPoint)v at:(NSPoint)pt {
    const CGFloat sz = 34.0;
    NSRect box = NSMakeRect(pt.x, pt.y, sz, sz);

    // Background circle
    [[cLabel() colorWithAlphaComponent:0.15] setFill];
    [[NSBezierPath bezierPathWithOvalInRect:box] fill];
    [[cSep() colorWithAlphaComponent:0.8] setStroke];
    [[NSBezierPath bezierPathWithOvalInRect:box] stroke];

    // Crosshair
    [cSep() setStroke];
    NSBezierPath *h = [NSBezierPath bezierPath];
    [h moveToPoint:NSMakePoint(pt.x + 3, pt.y + sz / 2)];
    [h lineToPoint:NSMakePoint(pt.x + sz - 3, pt.y + sz / 2)];
    [h stroke];
    NSBezierPath *vert = [NSBezierPath bezierPath];
    [vert moveToPoint:NSMakePoint(pt.x + sz / 2, pt.y + 3)];
    [vert lineToPoint:NSMakePoint(pt.x + sz / 2, pt.y + sz - 3)];
    [vert stroke];

    // Position dot
    CGFloat r = 5.0;
    CGFloat dx = pt.x + sz / 2 + v.x * (sz / 2 - r - 1);
    CGFloat dy = pt.y + sz / 2 + v.y * (sz / 2 - r - 1);
    NSRect dot = NSMakeRect(dx - r, dy - r, r * 2, r * 2);
    [cPressed() setFill];
    [[NSBezierPath bezierPathWithOvalInRect:dot] fill];
}

- (void)dot:(NSPoint)center radius:(CGFloat)r color:(NSColor *)col {
    NSRect rect = NSMakeRect(center.x - r, center.y - r, r * 2, r * 2);
    [col setFill];
    [[NSBezierPath bezierPathWithOvalInRect:rect] fill];
}

- (void)hline:(CGFloat)y x:(CGFloat)x width:(CGFloat)w color:(NSColor *)col {
    [col setFill];
    NSRectFill(NSMakeRect(x, y, w, 1));
}

- (void)label:(NSString *)text at:(NSPoint)pt {
    [self str:text at:pt color:cLabel() font:_fontUI];
}

- (void)str:(NSString *)s at:(NSPoint)pt color:(NSColor *)col font:(NSFont *)f {
    [s drawAtPoint:pt withAttributes:@{ NSForegroundColorAttributeName: col,
                                        NSFontAttributeName: f }];
}

@end
