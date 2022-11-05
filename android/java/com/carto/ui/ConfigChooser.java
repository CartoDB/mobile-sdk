package com.carto.ui;

import javax.microedition.khronos.egl.EGL10;
import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.egl.EGLDisplay;

import android.opengl.GLSurfaceView;

/**
 * EGL configuration chooser for MapView and TextureMapView.
 * It is not intended for public usage.
 * @hidden
 */
public class ConfigChooser implements GLSurfaceView.EGLConfigChooser {
    
    private static int[][] ATTRIBUTE_TABLE = new int[][] {
        // 8-8-8-8-bit color, 8-bit stencil, 24-bit z buffer. Should work on most devices.
        new int[] { EGL10.EGL_RED_SIZE, 8, EGL10.EGL_GREEN_SIZE, 8, EGL10.EGL_BLUE_SIZE, 8, EGL10.EGL_ALPHA_SIZE, 8, EGL10.EGL_DEPTH_SIZE, 24, EGL10.EGL_STENCIL_SIZE, 8, EGL10.EGL_RENDERABLE_TYPE, 4, EGL10.EGL_NONE },
        // 8-8-8-8-bit color, 8-bit stencil, 16-bit z buffer. Better than 5-6-5/16 bit, should also fix problems on some obscure devices.
        new int[] { EGL10.EGL_RED_SIZE, 8, EGL10.EGL_GREEN_SIZE, 8, EGL10.EGL_BLUE_SIZE, 8, EGL10.EGL_ALPHA_SIZE, 8, EGL10.EGL_DEPTH_SIZE, 16, EGL10.EGL_STENCIL_SIZE, 8, EGL10.EGL_RENDERABLE_TYPE, 4, EGL10.EGL_NONE },
        // 5-6-5-bit color, 24-bit z buffer, unspecified stencil. Should work on most devices.
        new int[] { EGL10.EGL_RED_SIZE, 5, EGL10.EGL_GREEN_SIZE, 6, EGL10.EGL_BLUE_SIZE, 5, EGL10.EGL_DEPTH_SIZE, 24, EGL10.EGL_RENDERABLE_TYPE, 4, EGL10.EGL_NONE },
        // 5-6-5-bit color, 16-bit z buffer, unspecified stencil. Fallback for original Tegra devices. 
        new int[] { EGL10.EGL_RED_SIZE, 5, EGL10.EGL_GREEN_SIZE, 6, EGL10.EGL_BLUE_SIZE, 5, EGL10.EGL_DEPTH_SIZE, 16, EGL10.EGL_RENDERABLE_TYPE, 4, EGL10.EGL_NONE },
        // 5-6-5-bit color, unspecified z/stencil buffer.
        new int[] { EGL10.EGL_RED_SIZE, 5, EGL10.EGL_GREEN_SIZE, 6, EGL10.EGL_BLUE_SIZE, 5, EGL10.EGL_RENDERABLE_TYPE, 4, EGL10.EGL_NONE },
    };

    @Override
    public EGLConfig chooseConfig(EGL10 egl, EGLDisplay eglDisplay) {
        com.carto.utils.Log.debug("ConfigChooser.chooseConfig: Model: " + android.os.Build.MODEL + ", board: " + android.os.Build.BOARD + ", product: " + android.os.Build.PRODUCT);

        for (int i = 0; i < ATTRIBUTE_TABLE.length; i++) {
            int[] numConfigs = new int[] { 0 };
            EGLConfig[] configs = new EGLConfig[1];
            if (egl.eglChooseConfig(eglDisplay, ATTRIBUTE_TABLE[i], configs, 1, numConfigs)) {
                if (numConfigs[0] > 0) {
                    com.carto.utils.Log.debug("ConfigChooser.chooseConfig: Selected display configuration: " + i);
                    return configs[0];
                }
            }
        }
        throw new IllegalArgumentException("Failed to choose EGLConfig!");
    }
}
