// MIT License
// Copyright (c) 2025 Ruben Leal Mirete
//
// Windows Example - campello_input
//
// A Win32 GUI application demonstrating campello_input usage.
// Displays real-time input state for gamepads, keyboard, and mouse.

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <windows.h>
#include <windowsx.h>
#include <string>
#include <sstream>
#include <iomanip>
#include <vector>
#include <cmath>

#include <campello_input/input_system.hpp>
#include <campello_input/gamepad.hpp>
#include <campello_input/keyboard.hpp>
#include <campello_input/mouse.hpp>

using namespace systems::leal::campello_input;

// Global input system
std::unique_ptr<InputSystem> g_input;
bool g_running = true;

// Window dimensions
constexpr int WINDOW_WIDTH = 1000;
constexpr int WINDOW_HEIGHT = 700;

// Colors
constexpr COLORREF COLOR_BG = RGB(30, 30, 40);
constexpr COLORREF COLOR_TEXT = RGB(220, 220, 220);
constexpr COLORREF COLOR_HEADER = RGB(100, 200, 255);
constexpr COLORREF COLOR_GAMEPAD = RGB(100, 255, 150);
constexpr COLORREF COLOR_KEYBOARD = RGB(255, 200, 100);
constexpr COLORREF COLOR_MOUSE = RGB(255, 150, 200);
constexpr COLORREF COLOR_AXIS_POS = RGB(100, 255, 100);
constexpr COLORREF COLOR_AXIS_NEG = RGB(255, 100, 100);
constexpr COLORREF COLOR_BUTTON_ON = RGB(100, 255, 100);
constexpr COLORREF COLOR_BUTTON_OFF = RGB(80, 80, 80);

// Format float to string
std::string FormatFloat(float value, int precision = 2) {
    std::ostringstream ss;
    ss << std::fixed << std::setprecision(precision) << value;
    return ss.str();
}

// Draw text helper
void DrawText(HDC hdc, int x, int y, const std::string& text, COLORREF color, int fontSize = 14) {
    HFONT font = CreateFont(fontSize, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                            DEFAULT_CHARSET, OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS,
                            CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Segoe UI");
    HFONT oldFont = (HFONT)SelectObject(hdc, font);
    
    SetTextColor(hdc, color);
    SetBkMode(hdc, TRANSPARENT);
    
    std::wstring wtext(text.begin(), text.end());
    TextOutW(hdc, x, y, wtext.c_str(), (int)wtext.length());
    
    SelectObject(hdc, oldFont);
    DeleteObject(font);
}

// Draw centered text
void DrawTextCentered(HDC hdc, int x, int y, int width, const std::string& text, COLORREF color, int fontSize = 14) {
    HFONT font = CreateFont(fontSize, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                            DEFAULT_CHARSET, OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS,
                            CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Segoe UI");
    HFONT oldFont = (HFONT)SelectObject(hdc, font);
    
    SetTextColor(hdc, color);
    SetBkMode(hdc, TRANSPARENT);
    
    std::wstring wtext(text.begin(), text.end());
    
    SIZE size;
    GetTextExtentPoint32W(hdc, wtext.c_str(), (int)wtext.length(), &size);
    
    int drawX = x + (width - size.cx) / 2;
    TextOutW(hdc, drawX, y, wtext.c_str(), (int)wtext.length());
    
    SelectObject(hdc, oldFont);
    DeleteObject(font);
}

// Draw a filled rectangle
void FillRect(HDC hdc, int x, int y, int width, int height, COLORREF color) {
    HBRUSH brush = CreateSolidBrush(color);
    RECT rect = { x, y, x + width, y + height };
    FillRect(hdc, &rect, brush);
    DeleteObject(brush);
}

// Draw a rectangle outline
void DrawRect(HDC hdc, int x, int y, int width, int height, COLORREF color, int thickness = 1) {
    HPEN pen = CreatePen(PS_SOLID, thickness, color);
    HPEN oldPen = (HPEN)SelectObject(hdc, pen);
    HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, GetStockObject(NULL_BRUSH));
    
    Rectangle(hdc, x, y, x + width, y + height);
    
    SelectObject(hdc, oldBrush);
    SelectObject(hdc, oldPen);
    DeleteObject(pen);
}

// Draw a circle
void DrawCircle(HDC hdc, int x, int y, int radius, COLORREF color, bool filled = false) {
    if (filled) {
        HBRUSH brush = CreateSolidBrush(color);
        HPEN oldPen = (HPEN)SelectObject(hdc, GetStockObject(NULL_PEN));
        HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, brush);
        Ellipse(hdc, x - radius, y - radius, x + radius, y + radius);
        SelectObject(hdc, oldBrush);
        SelectObject(hdc, oldPen);
        DeleteObject(brush);
    } else {
        HPEN pen = CreatePen(PS_SOLID, 2, color);
        HPEN oldPen = (HPEN)SelectObject(hdc, pen);
        HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, GetStockObject(NULL_BRUSH));
        Ellipse(hdc, x - radius, y - radius, x + radius, y + radius);
        SelectObject(hdc, oldBrush);
        SelectObject(hdc, oldPen);
        DeleteObject(pen);
    }
}

// Draw a line
void DrawLine(HDC hdc, int x1, int y1, int x2, int y2, COLORREF color, int thickness = 1) {
    HPEN pen = CreatePen(PS_SOLID, thickness, color);
    HPEN oldPen = (HPEN)SelectObject(hdc, pen);
    
    MoveToEx(hdc, x1, y1, nullptr);
    LineTo(hdc, x2, y2);
    
    SelectObject(hdc, oldPen);
    DeleteObject(pen);
}

// Draw an analog stick visualization
void DrawAnalogStick(HDC hdc, int centerX, int centerY, int radius, 
                     float xValue, float yValue, const std::string& label) {
    // Background circle
    DrawCircle(hdc, centerX, centerY, radius, COLOR_BUTTON_OFF, false);
    
    // Stick position
    int stickX = centerX + static_cast<int>(xValue * radius * 0.8f);
    int stickY = centerY + static_cast<int>(yValue * radius * 0.8f);
    
    // Draw stick
    DrawCircle(hdc, stickX, stickY, radius / 5, COLOR_GAMEPAD, true);
    
    // Draw crosshair lines
    DrawLine(hdc, centerX - radius, centerY, centerX + radius, centerY, COLOR_BUTTON_OFF);
    DrawLine(hdc, centerX, centerY - radius, centerX, centerY + radius, COLOR_BUTTON_OFF);
    
    // Label
    DrawTextCentered(hdc, centerX - 50, centerY + radius + 10, 100, 
                     label + " (" + FormatFloat(xValue) + ", " + FormatFloat(yValue) + ")",
                     COLOR_TEXT, 12);
}

// Draw trigger visualization
void DrawTrigger(HDC hdc, int x, int y, int width, int height, 
                 float value, const std::string& label) {
    // Background
    FillRect(hdc, x, y, width, height, COLOR_BUTTON_OFF);
    
    // Fill level
    int fillHeight = static_cast<int>(value * height);
    COLORREF color = (value > 0.5f) ? COLOR_AXIS_POS : COLOR_TEXT;
    FillRect(hdc, x, y + height - fillHeight, width, fillHeight, color);
    
    // Border
    DrawRect(hdc, x, y, width, height, COLOR_TEXT);
    
    // Label
    DrawTextCentered(hdc, x - 10, y + height / 2 - 7, width + 20,
                     label + " " + FormatFloat(value), COLOR_TEXT, 11);
}

// Draw a button
void DrawButton(HDC hdc, int x, int y, int width, int height, 
                const std::string& label, bool pressed) {
    COLORREF bgColor = pressed ? COLOR_BUTTON_ON : COLOR_BUTTON_OFF;
    COLORREF textColor = pressed ? RGB(0, 0, 0) : COLOR_TEXT;
    
    FillRect(hdc, x, y, width, height, bgColor);
    DrawRect(hdc, x, y, width, height, COLOR_TEXT);
    
    DrawTextCentered(hdc, x, y + height / 2 - 7, width, label, textColor, 12);
}

// Draw gamepad section
void DrawGamepadSection(HDC hdc, int x, int y, int width, int gamepadIndex) {
    if (gamepadIndex >= g_input->gamepadCount()) {
        DrawTextCentered(hdc, x, y + 50, width, "No Gamepad Connected", COLOR_BUTTON_OFF, 16);
        return;
    }
    
    auto* gamepad = g_input->gamepadAt(gamepadIndex);
    if (!gamepad) return;
    
    GamepadState state;
    gamepad->getState(state);
    
    // Title
    std::string title = std::string(gamepad->name()) + " [" + std::to_string(gamepadIndex) + "]";
    DrawTextCentered(hdc, x, y, width, title, COLOR_GAMEPAD, 16);
    
    int contentY = y + 30;
    
    // Left stick
    DrawAnalogStick(hdc, x + 80, contentY + 80, 60,
                    state.axes[(int)GamepadAxis::left_x].value,
                    state.axes[(int)GamepadAxis::left_y].value,
                    "Left Stick");
    
    // Right stick
    DrawAnalogStick(hdc, x + width - 80, contentY + 80, 60,
                    state.axes[(int)GamepadAxis::right_x].value,
                    state.axes[(int)GamepadAxis::right_y].value,
                    "Right Stick");
    
    // Triggers
    DrawTrigger(hdc, x + width / 2 - 60, contentY + 20, 25, 100,
                state.axes[(int)GamepadAxis::trigger_left].value, "LT");
    DrawTrigger(hdc, x + width / 2 + 35, contentY + 20, 25, 100,
                state.axes[(int)GamepadAxis::trigger_right].value, "RT");
    
    // Face buttons (Xbox layout)
    int btnY = contentY + 150;
    int btnCenterX = x + width / 2 + 100;
    int btnCenterY = btnY + 50;
    
    DrawButton(hdc, btnCenterX - 25, btnCenterY - 60, 50, 50, "Y",
               state.buttons & (uint64_t)GamepadButton::face_north);
    DrawButton(hdc, btnCenterX - 25, btnCenterY + 10, 50, 50, "A",
               state.buttons & (uint64_t)GamepadButton::face_south);
    DrawButton(hdc, btnCenterX - 95, btnCenterY - 25, 50, 50, "X",
               state.buttons & (uint64_t)GamepadButton::face_west);
    DrawButton(hdc, btnCenterX + 45, btnCenterY - 25, 50, 50, "B",
               state.buttons & (uint64_t)GamepadButton::face_east);
    
    // Shoulder buttons
    DrawButton(hdc, x + 50, contentY + 10, 80, 30, "LB",
               state.buttons & (uint64_t)GamepadButton::shoulder_left);
    DrawButton(hdc, x + width - 130, contentY + 10, 80, 30, "RB",
               state.buttons & (uint64_t)GamepadButton::shoulder_right);
    
    // D-pad
    int dpadX = x + 100;
    int dpadY = btnY + 50;
    DrawButton(hdc, dpadX - 25, dpadY - 60, 50, 50, "^",
               state.buttons & (uint64_t)GamepadButton::dpad_up);
    DrawButton(hdc, dpadX - 25, dpadY + 10, 50, 50, "v",
               state.buttons & (uint64_t)GamepadButton::dpad_down);
    DrawButton(hdc, dpadX - 95, dpadY - 25, 50, 50, "<",
               state.buttons & (uint64_t)GamepadButton::dpad_left);
    DrawButton(hdc, dpadX + 45, dpadY - 25, 50, 50, ">",
               state.buttons & (uint64_t)GamepadButton::dpad_right);
    
    // Menu buttons
    DrawButton(hdc, x + width / 2 - 80, contentY + 140, 60, 30, "View",
               state.buttons & (uint64_t)GamepadButton::select);
    DrawButton(hdc, x + width / 2 + 20, contentY + 140, 60, 30, "Menu",
               state.buttons & (uint64_t)GamepadButton::start);
}

// Draw keyboard section
void DrawKeyboardSection(HDC hdc, int x, int y, int width, int height) {
    DrawTextCentered(hdc, x, y, width, "Keyboard", COLOR_KEYBOARD, 18);
    
    if (!g_input->hasKeyboard()) {
        DrawTextCentered(hdc, x, y + 50, width, "No Keyboard Detected", COLOR_BUTTON_OFF, 14);
        return;
    }
    
    auto* keyboard = g_input->keyboard();
    KeyboardState state;
    keyboard->getState(state);
    
    // Modifiers
    int modY = y + 30;
    auto mods = keyboard->modifiers();
    
    DrawButton(hdc, x + 20, modY, 60, 30, "Shift", mods & KeyModifier::shift);
    DrawButton(hdc, x + 90, modY, 60, 30, "Ctrl", mods & KeyModifier::ctrl);
    DrawButton(hdc, x + 160, modY, 60, 30, "Alt", mods & KeyModifier::alt);
    DrawButton(hdc, x + 230, modY, 60, 30, "Win", mods & KeyModifier::meta);
    
    // Recent keys display
    int keysY = modY + 50;
    DrawText(hdc, x + 20, keysY, "Recent keys:", COLOR_TEXT, 12);
    
    // Show a few common keys status
    struct KeyCheck { KeyCode code; const char* name; int x; int y; };
    KeyCheck keys[] = {
        { KeyCode::w, "W", 20, 0 },
        { KeyCode::a, "A", 0, 30 },
        { KeyCode::s, "S", 30, 30 },
        { KeyCode::d, "D", 60, 30 },
        { KeyCode::space, "Space", 100, 30 },
        { KeyCode::enter, "Enter", 180, 30 },
        { KeyCode::escape, "Esc", 250, 30 },
    };
    
    int baseX = x + 20;
    int baseY = keysY + 30;
    
    for (const auto& key : keys) {
        bool pressed = keyboard->isKeyDown(key.code);
        DrawButton(hdc, baseX + key.x, baseY + key.y, 50, 30, key.name, pressed);
    }
}

// Draw mouse section
void DrawMouseSection(HDC hdc, int x, int y, int width, int height) {
    DrawTextCentered(hdc, x, y, width, "Mouse", COLOR_MOUSE, 18);
    
    if (!g_input->hasMouse()) {
        DrawTextCentered(hdc, x, y + 50, width, "No Mouse Detected", COLOR_BUTTON_OFF, 14);
        return;
    }
    
    auto* mouse = g_input->mouse();
    MouseState state;
    mouse->getState(state);
    
    // Position info
    int infoY = y + 35;
    DrawText(hdc, x + 20, infoY, 
             "Position: (" + std::to_string(state.pos_x) + ", " + std::to_string(state.pos_y) + ")",
             COLOR_TEXT, 12);
    DrawText(hdc, x + 20, infoY + 20,
             "Delta: (" + std::to_string(state.delta_x) + ", " + std::to_string(state.delta_y) + ")",
             COLOR_TEXT, 12);
    DrawText(hdc, x + 20, infoY + 40,
             "Scroll: (" + FormatFloat(state.scroll_delta_x) + ", " + FormatFloat(state.scroll_delta_y) + ")",
             COLOR_TEXT, 12);
    
    // Buttons
    int btnY = infoY + 70;
    DrawButton(hdc, x + 20, btnY, 60, 40, "Left", 
               state.buttons & (uint32_t)MouseButton::left);
    DrawButton(hdc, x + 90, btnY, 60, 40, "Right",
               state.buttons & (uint32_t)MouseButton::right);
    DrawButton(hdc, x + 160, btnY, 60, 40, "Middle",
               state.buttons & (uint32_t)MouseButton::middle);
}

// Window procedure
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_DESTROY:
            g_running = false;
            PostQuitMessage(0);
            return 0;
            
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            
            // Clear background
            FillRect(hdc, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, COLOR_BG);
            
            // Title
            DrawTextCentered(hdc, 0, 10, WINDOW_WIDTH, 
                             "campello_input - Windows Example", COLOR_HEADER, 24);
            DrawTextCentered(hdc, 0, 40, WINDOW_WIDTH,
                             "Press ESC to exit", COLOR_TEXT, 12);
            
            if (g_input) {
                // Update input
                g_input->update();
                
                // Check for ESC key
                if (g_input->hasKeyboard() && g_input->keyboard()->isKeyDown(KeyCode::escape)) {
                    g_running = false;
                    PostQuitMessage(0);
                }
                
                // Layout sections
                int sectionY = 70;
                int sectionHeight = 250;
                int padding = 20;
                int availableWidth = WINDOW_WIDTH - 2 * padding;
                int gamepadWidth = availableWidth / 2;
                
                // Gamepad 0 (left)
                DrawGamepadSection(hdc, padding, sectionY, gamepadWidth - padding, 0);
                
                // Gamepad 1 (right, if exists)
                if (g_input->gamepadCount() > 1) {
                    DrawGamepadSection(hdc, padding + gamepadWidth, sectionY, 
                                       gamepadWidth - padding, 1);
                } else {
                    // Show keyboard/mouse on right
                    DrawKeyboardSection(hdc, padding + gamepadWidth, sectionY, 
                                        gamepadWidth - padding, 150);
                    DrawMouseSection(hdc, padding + gamepadWidth, sectionY + 160,
                                     gamepadWidth - padding, 90);
                }
                
                // If no gamepads, show keyboard/mouse larger
                if (g_input->gamepadCount() == 0) {
                    DrawKeyboardSection(hdc, padding, sectionY, 
                                        availableWidth / 2 - padding, 150);
                    DrawMouseSection(hdc, availableWidth / 2 + padding, sectionY,
                                     availableWidth / 2 - padding, 150);
                }
            }
            
            EndPaint(hwnd, &ps);
            return 0;
        }
        
        case WM_TIMER: {
            // Trigger repaint at 60fps
            InvalidateRect(hwnd, nullptr, FALSE);
            return 0;
        }
    }
    
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

// Entry point
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, 
                   LPSTR lpCmdLine, int nCmdShow) {
    (void)hPrevInstance;
    (void)lpCmdLine;
    
    // Initialize input system
    g_input = InputSystem::create();
    if (!g_input) {
        MessageBoxA(nullptr, "Failed to initialize input system", "Error", MB_OK | MB_ICONERROR);
        return 1;
    }
    
    // Register window class
    const wchar_t CLASS_NAME[] = L"campello_input_windows_example";
    
    WNDCLASSW wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    
    if (!RegisterClassW(&wc)) {
        MessageBoxA(nullptr, "Failed to register window class", "Error", MB_OK | MB_ICONERROR);
        return 1;
    }
    
    // Create window
    HWND hwnd = CreateWindowExW(
        0,
        CLASS_NAME,
        L"campello_input - Windows Example",
        WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT,
        WINDOW_WIDTH, WINDOW_HEIGHT,
        nullptr,
        nullptr,
        hInstance,
        nullptr
    );
    
    if (!hwnd) {
        MessageBoxA(nullptr, "Failed to create window", "Error", MB_OK | MB_ICONERROR);
        return 1;
    }
    
    // Center window
    RECT rc;
    GetWindowRect(hwnd, &rc);
    int x = (GetSystemMetrics(SM_CXSCREEN) - (rc.right - rc.left)) / 2;
    int y = (GetSystemMetrics(SM_CYSCREEN) - (rc.bottom - rc.top)) / 2;
    SetWindowPos(hwnd, nullptr, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
    
    ShowWindow(hwnd, nCmdShow);
    
    // Set up 60fps timer
    SetTimer(hwnd, 1, 16, nullptr);  // ~60fps
    
    // Message loop
    MSG msg = {};
    while (g_running) {
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        
        // Small sleep to avoid burning CPU
        Sleep(1);
    }
    
    // Cleanup
    KillTimer(hwnd, 1);
    g_input.reset();
    
    return 0;
}
