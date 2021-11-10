/*
 * Copyright (C) 2017 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "PlatformEGLWayland.h"

#include "opengl/OpenGLDriver.h"
#include "opengl/OpenGLContext.h"
#include "opengl/OpenGLDriverFactory.h"

#include <utils/compiler.h>
#include <utils/Log.h>

using namespace utils;

namespace filament {
using namespace backend;

// The Android NDK doesn't exposes extensions, fake it with eglGetProcAddress
namespace glext {
UTILS_PRIVATE PFNEGLGETPLATFORMDISPLAYEXTPROC eglGetPlatformDisplayExt = {};
}
using namespace glext;

struct sharedContext {
    EGLContext eglSharedContext;
    struct wl_display *display;
    struct wl_surface *surface;
    struct wl_egl_window *egl_window;
};

PlatformEGLWayland::PlatformEGLWayland() noexcept
        : PlatformEGL() {

    mOSVersion = 0;
}

PlatformEGLWayland::~PlatformEGLWayland() noexcept = default;

void PlatformEGLWayland::terminate() noexcept {
    PlatformEGL::terminate();
}

Driver* PlatformEGLWayland::createDriver(void* sharedContext) noexcept {
    if (UTILS_UNLIKELY(sharedContext == nullptr)) {
        slog.e << "sharedContext not set" << io::endl;
        return nullptr;
    }

    auto extensions = GLUtils::split(eglQueryString(mEGLDisplay, EGL_EXTENSIONS));

    if (extensions.has("EGL_EXT_platform_wayland") && extensions.has("EGL_KHR_platform_wayland")) {
        eglGetPlatformDisplayExt = (PFNEGLGETPLATFORMDISPLAYEXTPROC) eglGetProcAddress("eglGetPlatformDisplayEXT");

        void *nativeDisplay = ((struct sharedContext *)sharedContext)->display;

        mEGLDisplay = eglGetPlatformDisplayExt(EGL_PLATFORM_WAYLAND_KHR, nativeDisplay, nullptr);
        if (UTILS_UNLIKELY(mEGLDisplay == EGL_NO_DISPLAY)) {
            slog.e << "eglGetPlatformDisplayExt failed" << io::endl;
            return nullptr;
        }
    } else {
        slog.e << "Platform != Wayland" << io::endl;
        return nullptr;
    }

    Driver* driver = PlatformEGL::createDriver(((struct sharedContext *)sharedContext)->eglSharedContext);

    return driver;
}

int PlatformEGLWayland::getOSVersion() const noexcept {
    return mOSVersion;
}

} // namespace filament

// ---------------------------------------------------------------------------------------------
