#include "InputRenderer.h"
#include "Font.h"
#include "AndroidOut.h"

#include <game-activity/native_app_glue/android_native_app_glue.h>
#include <GLES3/gl3.h>
#include <EGL/egl.h>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <algorithm>

using namespace systems::leal::campello_input;

// ---------------------------------------------------------------------------
// Colors — matching the macOS example palette
// ---------------------------------------------------------------------------

static constexpr float cBg[]      = {0.11f, 0.11f, 0.13f};
static constexpr float cCard[]    = {0.16f, 0.17f, 0.19f};
static constexpr float cTitle[]   = {0.92f, 0.92f, 0.95f};
static constexpr float cLabel[]   = {0.50f, 0.52f, 0.58f};
static constexpr float cValue[]   = {0.65f, 0.85f, 1.00f};
static constexpr float cPressed[] = {0.20f, 0.85f, 0.45f};
static constexpr float cOrange[]  = {1.00f, 0.60f, 0.15f};
static constexpr float cSection[] = {0.55f, 0.68f, 1.00f};
static constexpr float cSep[]     = {0.25f, 0.27f, 0.32f};
static constexpr float cRed[]     = {0.90f, 0.25f, 0.25f};

// Touch point colors (one per finger ID mod 8)
static constexpr float kTouchColors[8][3] = {
    {0.20f, 0.85f, 0.45f}, // green
    {0.65f, 0.85f, 1.00f}, // blue
    {1.00f, 0.60f, 0.15f}, // orange
    {1.00f, 0.30f, 0.60f}, // pink
    {0.80f, 0.40f, 1.00f}, // purple
    {0.30f, 0.90f, 0.90f}, // cyan
    {1.00f, 1.00f, 0.30f}, // yellow
    {0.90f, 0.60f, 0.40f}, // tan
};

// ---------------------------------------------------------------------------
// GLSL shaders
// ---------------------------------------------------------------------------

static const char* kColorVS = R"glsl(#version 300 es
in vec2 aPos;
uniform mat4 uProj;
void main() {
    gl_Position = uProj * vec4(aPos, 0.0, 1.0);
})glsl";

static const char* kColorFS = R"glsl(#version 300 es
precision mediump float;
uniform vec4 uColor;
out vec4 fragColor;
void main() { fragColor = uColor; }
)glsl";

static const char* kTextVS = R"glsl(#version 300 es
in vec2 aPos;
in vec2 aUV;
uniform mat4 uProj;
out vec2 vUV;
void main() {
    vUV = aUV;
    gl_Position = uProj * vec4(aPos, 0.0, 1.0);
})glsl";

static const char* kTextFS = R"glsl(#version 300 es
precision mediump float;
in vec2 vUV;
uniform sampler2D uFont;
uniform vec4 uColor;
out vec4 fragColor;
void main() {
    float a = texture(uFont, vUV).r;
    if (a < 0.5) discard;
    fragColor = uColor;
})glsl";

// ---------------------------------------------------------------------------
// GL helpers
// ---------------------------------------------------------------------------

GLuint InputRenderer::compileShader(GLenum type, const char* src) {
    GLuint s = glCreateShader(type);
    glShaderSource(s, 1, &src, nullptr);
    glCompileShader(s);
    GLint ok = 0;
    glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        GLint len = 0; glGetShaderiv(s, GL_INFO_LOG_LENGTH, &len);
        if (len) { char* log = new char[len]; glGetShaderInfoLog(s, len, nullptr, log);
            aout << "Shader compile error:\n" << log; delete[] log; }
        glDeleteShader(s); return 0;
    }
    return s;
}

GLuint InputRenderer::linkProgram(GLuint vs, GLuint fs) {
    GLuint p = glCreateProgram();
    glAttachShader(p, vs); glAttachShader(p, fs);
    glLinkProgram(p);
    GLint ok = 0; glGetProgramiv(p, GL_LINK_STATUS, &ok);
    if (!ok) { glDeleteProgram(p); return 0; }
    glDeleteShader(vs); glDeleteShader(fs);
    return p;
}

// ---------------------------------------------------------------------------
// Constructor / destructor
// ---------------------------------------------------------------------------

InputRenderer::InputRenderer(android_app* app) : app_(app) {
    initGL();
}

InputRenderer::~InputRenderer() {
    if (fontTex_)    glDeleteTextures(1, &fontTex_);
    if (vbo_)        glDeleteBuffers(1, &vbo_);
    if (colorProg_)  glDeleteProgram(colorProg_);
    if (textProg_)   glDeleteProgram(textProg_);

    if (display_ != EGL_NO_DISPLAY) {
        eglMakeCurrent(display_, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        if (context_ != EGL_NO_CONTEXT) eglDestroyContext(display_, context_);
        if (surface_ != EGL_NO_SURFACE) eglDestroySurface(display_, surface_);
        eglTerminate(display_);
    }
}

void InputRenderer::initGL() {
    constexpr EGLint attribs[] = {
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT,
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
        EGL_BLUE_SIZE, 8, EGL_GREEN_SIZE, 8, EGL_RED_SIZE, 8,
        EGL_NONE
    };

    auto display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    eglInitialize(display, nullptr, nullptr);

    EGLint numConfigs;
    eglChooseConfig(display, attribs, nullptr, 0, &numConfigs);
    EGLConfig* configs = new EGLConfig[numConfigs];
    eglChooseConfig(display, attribs, configs, numConfigs, &numConfigs);

    EGLConfig config = configs[0];
    for (int i = 0; i < numConfigs; ++i) {
        EGLint r, g, b;
        if (eglGetConfigAttrib(display, configs[i], EGL_RED_SIZE,   &r) && r == 8 &&
            eglGetConfigAttrib(display, configs[i], EGL_GREEN_SIZE, &g) && g == 8 &&
            eglGetConfigAttrib(display, configs[i], EGL_BLUE_SIZE,  &b) && b == 8) {
            config = configs[i]; break;
        }
    }
    delete[] configs;

    EGLSurface surface = eglCreateWindowSurface(display, config, app_->window, nullptr);
    EGLint ctxAttribs[] = {EGL_CONTEXT_CLIENT_VERSION, 3, EGL_NONE};
    EGLContext context = eglCreateContext(display, config, nullptr, ctxAttribs);
    eglMakeCurrent(display, surface, surface, context);

    display_ = display; surface_ = surface; context_ = context;
    width_ = -1; height_ = -1;

    // Shaders
    GLuint cvs = compileShader(GL_VERTEX_SHADER,   kColorVS);
    GLuint cfs = compileShader(GL_FRAGMENT_SHADER, kColorFS);
    colorProg_ = linkProgram(cvs, cfs);
    uColorProj_ = glGetUniformLocation(colorProg_, "uProj");
    uColorCol_  = glGetUniformLocation(colorProg_, "uColor");
    uColorPos_  = glGetAttribLocation(colorProg_,  "aPos");

    GLuint tvs = compileShader(GL_VERTEX_SHADER,   kTextVS);
    GLuint tfs = compileShader(GL_FRAGMENT_SHADER, kTextFS);
    textProg_  = linkProgram(tvs, tfs);
    uTextProj_ = glGetUniformLocation(textProg_, "uProj");
    uTextCol_  = glGetUniformLocation(textProg_, "uColor");
    uTextPos_  = glGetAttribLocation(textProg_,  "aPos");
    uTextUV_   = glGetAttribLocation(textProg_,  "aUV");
    uTextTex_  = glGetUniformLocation(textProg_,  "uFont");

    // VBO (reused every draw call)
    glGenBuffers(1, &vbo_);

    // Font texture
    fontTex_ = createFontTexture();

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void InputRenderer::updateViewport() {
    EGLint w, h;
    eglQuerySurface(display_, surface_, EGL_WIDTH,  &w);
    eglQuerySurface(display_, surface_, EGL_HEIGHT, &h);
    if (w == width_ && h == height_) return;
    width_ = w; height_ = h;
    glViewport(0, 0, w, h);

    // Scale: 1 unit = 8 px at 1x; auto-scale from screen width
    float rawScale = (float)w / 540.f;
    scale_ = std::max(1.f, std::min(4.f, rawScale));
    scale_ = floorf(scale_);  // integer scale for crisp pixels

    // Orthographic: (0,0)=top-left, (w,h)=bottom-right
    float* m = projMatrix_;
    memset(m, 0, 16 * sizeof(float));
    m[0]  =  2.f / (float)w;
    m[5]  = -2.f / (float)h;
    m[10] =  1.f;
    m[12] = -1.f;
    m[13] =  1.f;
    m[15] =  1.f;

    // Push projection to both programs
    glUseProgram(colorProg_);
    glUniformMatrix4fv(uColorProj_, 1, GL_FALSE, m);
    glUseProgram(textProg_);
    glUniformMatrix4fv(uTextProj_, 1, GL_FALSE, m);
    glUseProgram(0);
}

// ---------------------------------------------------------------------------
// Drawing primitives
// ---------------------------------------------------------------------------

void InputRenderer::uploadQuad(float x, float y, float w, float h) {
    float v[] = {
        x,     y,
        x + w, y,
        x,     y + h,
        x + w, y + h,
    };
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferData(GL_ARRAY_BUFFER, sizeof(v), v, GL_DYNAMIC_DRAW);
}

void InputRenderer::uploadQuadUV(float x, float y, float w, float h,
                                  float u0, float v0, float u1, float v1) {
    float v[] = {
        x,     y,     u0, v0,
        x + w, y,     u1, v0,
        x,     y + h, u0, v1,
        x + w, y + h, u1, v1,
    };
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferData(GL_ARRAY_BUFFER, sizeof(v), v, GL_DYNAMIC_DRAW);
}

void InputRenderer::drawRect(float x, float y, float w, float h,
                              float r, float g, float b, float a) {
    glUseProgram(colorProg_);
    glUniform4f(uColorCol_, r, g, b, a);
    uploadQuad(x, y, w, h);
    glEnableVertexAttribArray(uColorPos_);
    glVertexAttribPointer(uColorPos_, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), nullptr);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glDisableVertexAttribArray(uColorPos_);
}

void InputRenderer::drawHLine(float x, float y, float w,
                               float r, float g, float b, float a) {
    drawRect(x, y, w, 1.f, r, g, b, a);
}

void InputRenderer::drawText(const char* text, float x, float y, float scale,
                              float r, float g, float b, float a) {
    // Font atlas: 16 glyphs per row, each 8x8 in a 128x64 texture
    static constexpr float kAtlasW = 128.f, kAtlasH = 64.f;
    static constexpr float kGlyphW = 8.f,   kGlyphH = 8.f;

    float cw = kGlyphW * scale;
    float ch = kGlyphH * scale;

    glUseProgram(textProg_);
    glUniform4f(uTextCol_, r, g, b, a);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, fontTex_);
    glUniform1i(uTextTex_, 0);

    glEnableVertexAttribArray(uTextPos_);
    glEnableVertexAttribArray(uTextUV_);

    float cx = x;
    for (const char* p = text; *p; ++p) {
        unsigned char c = (unsigned char)*p;
        if (c < 0x20 || c > 0x7F) { cx += cw; continue; }
        int idx = c - 0x20;
        int col = idx % 16;
        int row = idx / 16;
        float u0 = col * kGlyphW / kAtlasW;
        float v0 = row * kGlyphH / kAtlasH;
        float u1 = u0 + kGlyphW / kAtlasW;
        float v1 = v0 + kGlyphH / kAtlasH;

        uploadQuadUV(cx, y, cw, ch, u0, v0, u1, v1);
        glVertexAttribPointer(uTextPos_, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), nullptr);
        glVertexAttribPointer(uTextUV_,  2, GL_FLOAT, GL_FALSE, 4 * sizeof(float),
                              (void*)(2 * sizeof(float)));
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        cx += cw;
    }

    glDisableVertexAttribArray(uTextPos_);
    glDisableVertexAttribArray(uTextUV_);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void InputRenderer::drawBar(float x, float y, float w, float h, float value,
                             float r, float g, float b) {
    // Background
    drawRect(x, y, w, h, cLabel[0], cLabel[1], cLabel[2], 0.15f);
    // Fill
    if (value > 0.001f) {
        float fw = w * std::min(1.f, std::max(0.f, value));
        drawRect(x, y, fw, h, r, g, b, 0.85f);
    }
}

void InputRenderer::drawStick(float cx, float cy, float sz, float vx, float vy) {
    // Background square
    drawRect(cx, cy, sz, sz, cLabel[0], cLabel[1], cLabel[2], 0.18f);
    // Border (4 lines)
    drawRect(cx, cy, sz, 1.f, cSep[0], cSep[1], cSep[2], 0.8f);
    drawRect(cx, cy + sz - 1, sz, 1.f, cSep[0], cSep[1], cSep[2], 0.8f);
    drawRect(cx, cy, 1.f, sz, cSep[0], cSep[1], cSep[2], 0.8f);
    drawRect(cx + sz - 1, cy, 1.f, sz, cSep[0], cSep[1], cSep[2], 0.8f);
    // Crosshair
    float mid = sz / 2.f;
    drawRect(cx + 2, cy + mid - 0.5f, sz - 4, 1.f, cSep[0], cSep[1], cSep[2], 0.5f);
    drawRect(cx + mid - 0.5f, cy + 2, 1.f, sz - 4, cSep[0], cSep[1], cSep[2], 0.5f);
    // Dot
    float dot = std::max(3.f, sz / 8.f);
    float dx = cx + mid + vx * (mid - dot - 1) - dot / 2.f;
    float dy = cy + mid + vy * (mid - dot - 1) - dot / 2.f;
    drawRect(dx, dy, dot, dot, cPressed[0], cPressed[1], cPressed[2], 1.f);
}

// ---------------------------------------------------------------------------
// Section UI helpers
// ---------------------------------------------------------------------------

void InputRenderer::drawSectionHeader(const char* title, float x, float& y, float w) {
    drawText(title, x, y, scale_, cSection[0], cSection[1], cSection[2]);
    y += 8.f * scale_ + 4.f * scale_;
    drawHLine(x, y, w, cSep[0], cSep[1], cSep[2], 0.6f);
    y += 4.f * scale_;
}

void InputRenderer::drawBtnLabel(const char* label, bool pressed, float x, float y) {
    float s = scale_;
    float cw = 8.f * s;
    float ch = 8.f * s;
    float pad = 2.f * s;
    size_t len = strlen(label);
    float bw = cw * len + pad * 2;
    float bh = ch + pad * 2;

    if (pressed) {
        drawRect(x, y, bw, bh, cPressed[0], cPressed[1], cPressed[2], 1.f);
        drawText(label, x + pad, y + pad, s, 0.f, 0.f, 0.f);
    } else {
        drawRect(x, y, bw, bh, cLabel[0], cLabel[1], cLabel[2], 0.12f);
        // border
        drawRect(x, y, bw, 1.f, cLabel[0], cLabel[1], cLabel[2], 0.4f);
        drawRect(x, y + bh - 1, bw, 1.f, cLabel[0], cLabel[1], cLabel[2], 0.4f);
        drawRect(x, y, 1.f, bh, cLabel[0], cLabel[1], cLabel[2], 0.4f);
        drawRect(x + bw - 1, y, 1.f, bh, cLabel[0], cLabel[1], cLabel[2], 0.4f);
        drawText(label, x + pad, y + pad, s, cLabel[0], cLabel[1], cLabel[2]);
    }
}

float InputRenderer::drawGamepadCard(const GamepadEntry& gp, float x, float y, float w) {
    float s = scale_;
    float cw = 8.f * s;
    float ch = 8.f * s;
    float pad = 8.f * s;
    float cardH = ch * 5.f + pad * 3.f;  // ~5 rows of content + padding

    // Card background
    drawRect(x + pad * 0.5f, y, w - pad, cardH, cCard[0], cCard[1], cCard[2], 0.85f);

    float lx = x + pad;
    float ly = y + pad * 0.5f;

    // Status dot + name
    bool conn = gp.active;
    if (conn) drawRect(lx, ly + ch * 0.3f, s * 6.f, s * 6.f,
                       0.2f, 0.9f, 0.4f, 1.f);
    else       drawRect(lx, ly + ch * 0.3f, s * 6.f, s * 6.f,
                       0.9f, 0.25f, 0.25f, 1.f);
    drawText(gp.name, lx + s * 10.f, ly, s, cTitle[0], cTitle[1], cTitle[2]);
    ly += ch + s * 4.f;

    using B = GamepadButton;
    using A = GamepadAxis;
    auto btn = [&](B b) -> bool {
        return (gp.state.buttons & static_cast<uint64_t>(b)) != 0;
    };

    // Row 1: face buttons + shoulders + thumb clicks + start/select/home
    float rx = lx;
    drawText("Face", rx, ly, s, cLabel[0], cLabel[1], cLabel[2]);  rx += cw * 4.5f;
    drawBtnLabel("Y",  btn(B::face_north),   rx, ly); rx += cw * 1.5f + s * 6.f;
    drawBtnLabel("X",  btn(B::face_west),    rx, ly); rx += cw * 1.5f + s * 6.f;
    drawBtnLabel("A",  btn(B::face_south),   rx, ly); rx += cw * 1.5f + s * 6.f;
    drawBtnLabel("B",  btn(B::face_east),    rx, ly); rx += cw * 1.5f + s * 10.f;
    drawBtnLabel("LB", btn(B::shoulder_left),  rx, ly); rx += cw * 2.5f + s * 6.f;
    drawBtnLabel("RB", btn(B::shoulder_right), rx, ly); rx += cw * 2.5f + s * 10.f;
    drawBtnLabel("L3", btn(B::thumb_left),   rx, ly); rx += cw * 2.5f + s * 6.f;
    drawBtnLabel("R3", btn(B::thumb_right),  rx, ly); rx += cw * 2.5f + s * 10.f;
    drawBtnLabel("Sta", btn(B::start),       rx, ly); rx += cw * 3.5f + s * 6.f;
    drawBtnLabel("Sel", btn(B::select),      rx, ly); rx += cw * 3.5f + s * 6.f;
    drawBtnLabel("Hom", btn(B::home),        rx, ly);
    ly += ch + s * 6.f + s * 4.f;

    // Row 2: D-pad + trigger bars
    rx = lx;
    drawText("D-Pad", rx, ly, s, cLabel[0], cLabel[1], cLabel[2]);  rx += cw * 5.5f;
    drawBtnLabel("^", btn(B::dpad_up),    rx, ly); rx += cw + s * 6.f;
    drawBtnLabel("v", btn(B::dpad_down),  rx, ly); rx += cw + s * 6.f;
    drawBtnLabel("<", btn(B::dpad_left),  rx, ly); rx += cw + s * 6.f;
    drawBtnLabel(">", btn(B::dpad_right), rx, ly); rx += cw + s * 14.f;

    float lt = gp.state.axes[(int)A::trigger_left ].value;
    float rt = gp.state.axes[(int)A::trigger_right].value;
    float barW = cw * 10.f;
    float barH = ch * 0.6f;

    drawText("LT", rx, ly, s, cLabel[0], cLabel[1], cLabel[2]);  rx += cw * 2.5f;
    drawBar(rx, ly + ch * 0.2f, barW, barH, lt, cOrange[0], cOrange[1], cOrange[2]); rx += barW + s * 4.f;
    char buf[16];
    snprintf(buf, sizeof(buf), "%.2f", lt);
    drawText(buf, rx, ly, s, cValue[0], cValue[1], cValue[2]);  rx += cw * 4.5f;

    drawText("RT", rx, ly, s, cLabel[0], cLabel[1], cLabel[2]);  rx += cw * 2.5f;
    drawBar(rx, ly + ch * 0.2f, barW, barH, rt, cOrange[0], cOrange[1], cOrange[2]); rx += barW + s * 4.f;
    snprintf(buf, sizeof(buf), "%.2f", rt);
    drawText(buf, rx, ly, s, cValue[0], cValue[1], cValue[2]);
    ly += ch + s * 6.f + s * 4.f;

    // Row 3: sticks
    rx = lx;
    float stickSz = ch * 4.f;
    float lsx = gp.state.axes[(int)A::left_x ].value;
    float lsy = gp.state.axes[(int)A::left_y ].value;
    float rsx = gp.state.axes[(int)A::right_x].value;
    float rsy = gp.state.axes[(int)A::right_y].value;

    drawText("L", rx, ly + stickSz * 0.4f, s, cLabel[0], cLabel[1], cLabel[2]); rx += cw * 1.5f;
    drawStick(rx, ly, stickSz, lsx, lsy);  rx += stickSz + s * 4.f;
    snprintf(buf, sizeof(buf), "(%+.2f,%+.2f)", lsx, lsy);
    drawText(buf, rx, ly + stickSz * 0.3f, s, cValue[0], cValue[1], cValue[2]);  rx += cw * 14.f;

    drawText("R", rx, ly + stickSz * 0.4f, s, cLabel[0], cLabel[1], cLabel[2]); rx += cw * 1.5f;
    drawStick(rx, ly, stickSz, rsx, rsy);  rx += stickSz + s * 4.f;
    snprintf(buf, sizeof(buf), "(%+.2f,%+.2f)", rsx, rsy);
    drawText(buf, rx, ly + stickSz * 0.3f, s, cValue[0], cValue[1], cValue[2]);

    return y + cardH + s * 6.f;
}

// ---------------------------------------------------------------------------
// Section renderers
// ---------------------------------------------------------------------------

void InputRenderer::drawGamepads(const InputDisplayState& state, float& y) {
    float s = scale_;
    float ch = 8.f * s;
    float pad = 8.f * s;
    float w = (float)width_;

    drawSectionHeader("GAMEPADS", pad, y, w - pad * 2.f);

    if (state.gamepadCount == 0) {
        drawText("No gamepads connected. Connect a controller via USB or Bluetooth.",
                 pad * 2.f, y, s, cLabel[0], cLabel[1], cLabel[2]);
        y += ch + pad;
        return;
    }

    for (int i = 0; i < state.gamepadCount && i < InputDisplayState::kMaxGamepads; ++i) {
        if (state.gamepads[i].active) {
            y = drawGamepadCard(state.gamepads[i], 0.f, y, w);
        }
    }
    y += pad * 0.5f;
}

void InputRenderer::drawTouch(const InputDisplayState& state, float& y) {
    float s   = scale_;
    float cw  = 8.f * s;
    float ch  = 8.f * s;
    float pad = 8.f * s;
    float w   = (float)width_;
    float fh  = (float)height_;

    drawSectionHeader("TOUCH", pad, y, w - pad * 2.f);

    uint32_t count = state.touch.count;

    if (count == 0) {
        drawText("No active touch points.", pad * 2.f, y, s,
                 cLabel[0], cLabel[1], cLabel[2]);
        y += ch + pad;
        return;
    }

    // Touch canvas — a scaled-down representation of the screen
    float cvW = w * 0.25f;
    float cvH = fh * 0.25f;
    float cvX = w - cvW - pad;
    float cvY = y;

    // Canvas background
    drawRect(cvX, cvY, cvW, cvH, cCard[0], cCard[1], cCard[2], 0.9f);
    drawRect(cvX, cvY, cvW, 1.f, cSep[0], cSep[1], cSep[2], 0.6f);
    drawRect(cvX, cvY + cvH, cvW, 1.f, cSep[0], cSep[1], cSep[2], 0.6f);
    drawRect(cvX, cvY, 1.f, cvH, cSep[0], cSep[1], cSep[2], 0.6f);
    drawRect(cvX + cvW - 1, cvY, 1.f, cvH, cSep[0], cSep[1], cSep[2], 0.6f);

    for (uint32_t i = 0; i < count; ++i) {
        const auto& pt = state.touch.points[i];
        const float* col = kTouchColors[pt.id % 8];

        // Dot on canvas
        float dotX = cvX + (pt.x / (float)width_)  * cvW;
        float dotY = cvY + (pt.y / (float)height_) * cvH;
        float dotSz = s * 6.f;
        drawRect(dotX - dotSz / 2.f, dotY - dotSz / 2.f, dotSz, dotSz,
                 col[0], col[1], col[2], 1.f);

        // Coordinate label (left column)
        char line[64];
        snprintf(line, sizeof(line), "id=%d  (%.0f, %.0f)  p=%.2f",
                 pt.id, pt.x, pt.y, pt.pressure);
        drawText(line, pad * 2.f, y + i * (ch + s * 4.f), s,
                 col[0], col[1], col[2]);
    }

    float textH = count * (ch + s * 4.f);
    y += std::max(textH, cvH) + pad;
}

void InputRenderer::drawKeys(const InputDisplayState& state, float& y) {
    float s   = scale_;
    float cw  = 8.f * s;
    float ch  = 8.f * s;
    float pad = 8.f * s;
    float w   = (float)width_;

    drawSectionHeader("KEYS", pad, y, w - pad * 2.f);

    if (state.recentKeys.empty()) {
        drawText("No key events yet. Press a key or connect a keyboard.",
                 pad * 2.f, y, s, cLabel[0], cLabel[1], cLabel[2]);
        y += ch + pad;
        return;
    }

    float kx = pad * 2.f;
    for (size_t i = 0; i < state.recentKeys.size() && i < 10; ++i) {
        drawBtnLabel(state.recentKeys[i].c_str(), false, kx, y);
        kx += strlen(state.recentKeys[i].c_str()) * cw + cw + s * 8.f;
        if (kx > w - pad * 4.f) break;
    }
    y += ch + s * 8.f + pad;
}

// ---------------------------------------------------------------------------
// Main render entry
// ---------------------------------------------------------------------------

void InputRenderer::render(const InputDisplayState& state) {
    updateViewport();

    float s   = scale_;
    float pad = 8.f * s;
    float ch  = 8.f * s;
    float w   = (float)width_;

    // Clear
    glClearColor(cBg[0], cBg[1], cBg[2], 1.f);
    glClear(GL_COLOR_BUFFER_BIT);

    float y = pad;

    // Title
    drawText("campello_input  --  Input Monitor", pad, y, s,
             cTitle[0], cTitle[1], cTitle[2]);
    y += ch + s * 4.f;
    drawHLine(pad, y, w - pad * 2.f, cSep[0], cSep[1], cSep[2], 0.6f);
    y += s * 8.f;

    // Sections
    drawGamepads(state, y);
    drawHLine(pad, y, w - pad * 2.f, cSep[0], cSep[1], cSep[2], 0.4f);
    y += s * 8.f;
    drawTouch(state, y);
    drawHLine(pad, y, w - pad * 2.f, cSep[0], cSep[1], cSep[2], 0.4f);
    y += s * 8.f;
    drawKeys(state, y);

    eglSwapBuffers(display_, surface_);
}
