#pragma once

#include <EGL/egl.h>
#include <GLES3/gl3.h>
#include "InputDisplayState.h"

struct android_app;

// ---------------------------------------------------------------------------
// InputRenderer — 2D OpenGL ES renderer for the input monitor UI.
// Matches the visual layout of the macOS example.
// ---------------------------------------------------------------------------

class InputRenderer {
public:
    explicit InputRenderer(android_app* app);
    ~InputRenderer();

    void render(const InputDisplayState& state);

private:
    void initGL();
    void updateViewport();

    // --- 2D drawing primitives (pixel coords, origin = top-left) ---
    void drawRect(float x, float y, float w, float h,
                  float r, float g, float b, float a = 1.f);
    void drawText(const char* text, float x, float y, float scale,
                  float r, float g, float b, float a = 1.f);
    void drawBar(float x, float y, float w, float h, float value,
                 float r, float g, float b);
    void drawStick(float cx, float cy, float sz, float vx, float vy);
    void drawHLine(float x, float y, float w,
                   float r, float g, float b, float a = 0.4f);

    // --- UI sections ---
    void drawSectionHeader(const char* title, float x, float& y, float w);
    void drawBtnLabel(const char* label, bool pressed, float x, float y);
    float drawGamepadCard(const InputDisplayState::GamepadEntry& gp,
                          float x, float y, float w);
    void drawGamepads(const InputDisplayState& state, float& y);
    void drawTouch(const InputDisplayState& state, float& y);
    void drawKeys(const InputDisplayState& state, float& y);

    // --- GL helpers ---
    static GLuint compileShader(GLenum type, const char* src);
    static GLuint linkProgram(GLuint vs, GLuint fs);
    void uploadQuad(float x, float y, float w, float h);
    void uploadQuadUV(float x, float y, float w, float h,
                      float u0, float v0, float u1, float v1);

    android_app* app_;
    EGLDisplay   display_ = EGL_NO_DISPLAY;
    EGLSurface   surface_ = EGL_NO_SURFACE;
    EGLContext   context_ = EGL_NO_CONTEXT;
    int          width_   = 0;
    int          height_  = 0;

    GLuint colorProg_  = 0;   // uniform-color quads
    GLint  uColorProj_ = -1;
    GLint  uColorCol_  = -1;
    GLint  uColorPos_  = -1;  // attrib location

    GLuint textProg_   = 0;   // font-atlas text
    GLint  uTextProj_  = -1;
    GLint  uTextCol_   = -1;
    GLint  uTextPos_   = -1;  // attrib location
    GLint  uTextUV_    = -1;  // attrib location
    GLint  uTextTex_   = -1;

    GLuint vbo_        = 0;
    GLuint fontTex_    = 0;

    float  scale_      = 2.f; // font / UI scale factor
    float  projMatrix_[16]{};
};
