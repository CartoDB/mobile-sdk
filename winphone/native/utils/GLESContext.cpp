#include "GLESContext.h"
#include "utils/Log.h"

#include <stdexcept>

#include <wrl.h>
#include <wrl/implements.h>
#include <windows.ui.xaml.media.dxinterop.h>
#include <collection.h>
#include <concrt.h>
#include <ppltasks.h>

using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::UI::Xaml::Controls;

namespace carto {

    GLESContext::GLESContext() : mEglConfig(NULL), mEglDisplay(EGL_NO_DISPLAY), mEglContext(EGL_NO_CONTEXT) {
        Initialize();
    }

    GLESContext::~GLESContext() {
        Cleanup();
    }

    void GLESContext::Initialize() {
        const EGLint configAttributes[] = {
            EGL_RED_SIZE, 8,
            EGL_GREEN_SIZE, 8,
            EGL_BLUE_SIZE, 8,
            EGL_ALPHA_SIZE, 8,
            EGL_DEPTH_SIZE, 24,
            EGL_STENCIL_SIZE, 8,
            EGL_NONE
        };

        const EGLint contextAttributes[] = {
            EGL_CONTEXT_CLIENT_VERSION, 2,
            EGL_NONE
        };

        const EGLint defaultDisplayAttributes[] = {
            // These are the default display attributes, used to request ANGLE's D3D11 renderer.
            // eglInitialize will only succeed with these attributes if the hardware supports D3D11 Feature Level 10_0+.
            EGL_PLATFORM_ANGLE_TYPE_ANGLE, EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE,

            // EGL_ANGLE_DISPLAY_ALLOW_RENDER_TO_BACK_BUFFER is an optimization that can have large performance benefits on mobile devices.
            // Its syntax is subject to change, though. Please update your Visual Studio templates if you experience compilation issues with it.
            EGL_ANGLE_DISPLAY_ALLOW_RENDER_TO_BACK_BUFFER, EGL_TRUE,

            // EGL_PLATFORM_ANGLE_ENABLE_AUTOMATIC_TRIM_ANGLE is an option that enables ANGLE to automatically call 
            // the IDXGIDevice3::Trim method on behalf of the application when it gets suspended. 
            // Calling IDXGIDevice3::Trim when an application is suspended is a Windows Store application certification requirement.
            EGL_PLATFORM_ANGLE_ENABLE_AUTOMATIC_TRIM_ANGLE, EGL_TRUE,
            EGL_NONE,
        };

        const EGLint fl9_3DisplayAttributes[] = {
            // These can be used to request ANGLE's D3D11 renderer, with D3D11 Feature Level 9_3.
            // These attributes are used if the call to eglInitialize fails with the default display attributes.
            EGL_PLATFORM_ANGLE_TYPE_ANGLE, EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE,
            EGL_PLATFORM_ANGLE_MAX_VERSION_MAJOR_ANGLE, 9,
            EGL_PLATFORM_ANGLE_MAX_VERSION_MINOR_ANGLE, 3,
            EGL_ANGLE_DISPLAY_ALLOW_RENDER_TO_BACK_BUFFER, EGL_TRUE,
            EGL_PLATFORM_ANGLE_ENABLE_AUTOMATIC_TRIM_ANGLE, EGL_TRUE,
            EGL_NONE,
        };

        const EGLint warpDisplayAttributes[] = {
            // These attributes can be used to request D3D11 WARP.
            // They are used if eglInitialize fails with both the default display attributes and the 9_3 display attributes.
            EGL_PLATFORM_ANGLE_TYPE_ANGLE, EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE,
            EGL_PLATFORM_ANGLE_DEVICE_TYPE_ANGLE, EGL_PLATFORM_ANGLE_DEVICE_TYPE_WARP_ANGLE,
            EGL_ANGLE_DISPLAY_ALLOW_RENDER_TO_BACK_BUFFER, EGL_TRUE,
            EGL_PLATFORM_ANGLE_ENABLE_AUTOMATIC_TRIM_ANGLE, EGL_TRUE,
            EGL_NONE,
        };

        try {
            // eglGetPlatformDisplayEXT is an alternative to eglGetDisplay. It allows us to pass in display attributes, used to configure D3D11.
            PFNEGLGETPLATFORMDISPLAYEXTPROC eglGetPlatformDisplayEXT = reinterpret_cast<PFNEGLGETPLATFORMDISPLAYEXTPROC>(eglGetProcAddress("eglGetPlatformDisplayEXT"));
            if (!eglGetPlatformDisplayEXT) {
                throw std::runtime_error("Failed to get function eglGetPlatformDisplayEXT");
            }

            // This tries to initialize EGL to D3D11 Feature Level 9_3, if 10_0+ is unavailable (e.g. on Windows Phone, or certain Windows tablets).
            mEglDisplay = eglGetPlatformDisplayEXT(EGL_PLATFORM_ANGLE_ANGLE, EGL_DEFAULT_DISPLAY, fl9_3DisplayAttributes);
            if (mEglDisplay == EGL_NO_DISPLAY) {
                throw std::runtime_error("Failed to get EGL display");
            }

            if (eglInitialize(mEglDisplay, NULL, NULL) == EGL_FALSE) {
                // This initializes EGL to D3D11 Feature Level 11_0 on WARP, if 9_3+ is unavailable on the default GPU (e.g. on Surface RT).
                mEglDisplay = eglGetPlatformDisplayEXT(EGL_PLATFORM_ANGLE_ANGLE, EGL_DEFAULT_DISPLAY, warpDisplayAttributes);
                if (mEglDisplay == EGL_NO_DISPLAY) {
                    throw std::runtime_error("Failed to get EGL display");
                }

                if (eglInitialize(mEglDisplay, NULL, NULL) == EGL_FALSE) {
                    // If all of the calls to eglInitialize returned EGL_FALSE then an error has occurred.
                    throw std::runtime_error("Failed to initialize EGL");
                }
            }

            EGLint numConfigs = 0;
            if ((eglChooseConfig(mEglDisplay, configAttributes, &mEglConfig, 1, &numConfigs) == EGL_FALSE) || (numConfigs == 0)) {
                throw std::runtime_error("Failed to choose first EGLConfig");
            }

            mEglContext = eglCreateContext(mEglDisplay, mEglConfig, EGL_NO_CONTEXT, contextAttributes);
            if (mEglContext == EGL_NO_CONTEXT) {
                throw std::runtime_error("Failed to create EGL context");
            }
        }
        catch (const std::exception& ex) {
            Log::Errorf("GLESContext: Exception %s", ex.what());
        }
    }

    void GLESContext::Cleanup() {
        if (mEglDisplay != EGL_NO_DISPLAY && mEglContext != EGL_NO_CONTEXT) {
            eglDestroyContext(mEglDisplay, mEglContext);
            mEglContext = EGL_NO_CONTEXT;
        }

        if (mEglDisplay != EGL_NO_DISPLAY) {
            eglTerminate(mEglDisplay);
            mEglDisplay = EGL_NO_DISPLAY;
        }
    }

    void GLESContext::Reset() {
        Cleanup();
        Initialize();
    }

    EGLSurface GLESContext::CreateSurface(void* panelPtr, int renderSurfaceWidth, int renderSurfaceHeight) {
        SwapChainPanel^ panel = reinterpret_cast<SwapChainPanel^>(panelPtr);
        if (!panel) {
            Log::Error("GLESContext: SwapChainPanel parameter is invalid");
            return NULL;
        }

        EGLSurface surface = EGL_NO_SURFACE;

        const EGLint surfaceAttributes[] = {
            // EGL_ANGLE_SURFACE_RENDER_TO_BACK_BUFFER is part of the same optimization as EGL_ANGLE_DISPLAY_ALLOW_RENDER_TO_BACK_BUFFER (see above).
            // If you have compilation issues with it then please update your Visual Studio templates.
            EGL_ANGLE_SURFACE_RENDER_TO_BACK_BUFFER, EGL_TRUE,
            EGL_NONE
        };

        // Create a PropertySet and initialize with the EGLNativeWindowType.
        PropertySet^ surfaceCreationProperties = ref new PropertySet();
        surfaceCreationProperties->Insert(ref new String(EGLNativeWindowTypeProperty), panel);

        // If a render surface size is specified, add it to the surface creation properties
        if (renderSurfaceWidth != -1 && renderSurfaceHeight != -1) {
            surfaceCreationProperties->Insert(ref new String(EGLRenderSurfaceSizeProperty), PropertyValue::CreateSize(Size(renderSurfaceWidth, renderSurfaceHeight)));
        }

        surface = eglCreateWindowSurface(mEglDisplay, mEglConfig, reinterpret_cast<IInspectable*>(surfaceCreationProperties), surfaceAttributes);
        if (surface == EGL_NO_SURFACE) {
            Log::Error("GLESContext: Failed to create EGL surface");
        }

        return surface;
    }

    void GLESContext::DestroySurface(const EGLSurface surface) {
        if (mEglDisplay != EGL_NO_DISPLAY && surface != EGL_NO_SURFACE) {
            eglDestroySurface(mEglDisplay, surface);
        }
    }

    void GLESContext::MakeCurrent(const EGLSurface surface) {
        if (eglMakeCurrent(mEglDisplay, surface, surface, mEglContext) == EGL_FALSE) {
            Log::Error("GLESContext: Failed to make EGLSurface current");
        }
    }

    bool GLESContext::SwapBuffers(const EGLSurface surface) {
        return (eglSwapBuffers(mEglDisplay, surface)) != EGL_FALSE;
    }

}
