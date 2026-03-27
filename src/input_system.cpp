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

#include <campello_input/input_system.hpp>

#if defined(__APPLE__)
    #include "apple/inc/campello_input/input_apple_system.hpp"
#elif defined(__ANDROID__)
    #include "android/inc/campello_input/input_android_system.hpp"
#elif defined(_WIN32)
    #include "gdk/inc/campello_input/input_gdk_system.hpp"
#endif

namespace systems::leal::campello_input {

std::unique_ptr<InputSystem> InputSystem::create(const PlatformContext& ctx) {
#if defined(__APPLE__)
    return std::make_unique<AppleInputSystem>(ctx);
#elif defined(__ANDROID__)
    return std::make_unique<AndroidInputSystem>(ctx);
#elif defined(_WIN32)
    return std::make_unique<GdkInputSystem>(ctx);
#else
    // Platform not yet implemented
    (void)ctx;  // Suppress unused parameter warning
    return nullptr;
#endif
}

} // namespace systems::leal::campello_input
