namespace Carto.Ui {
    using Javax.Microedition.Khronos.Egl;
    using Android.Util;

    // EGL configuration chooser for MapView.
    internal class ConfigChooser : Java.Lang.Object, Android.Opengl.GLSurfaceView.IEGLConfigChooser {

        private static int[][] ATTRIBUTE_TABLE = new int[][] {
            // 8-8-8-8-bit color, 8-bit stencil, 24-bit z buffer. Should work on most devices.
            new int[] { EGL10.EglRedSize, 8, EGL10.EglGreenSize, 8, EGL10.EglBlueSize, 8, EGL10.EglAlphaSize, 8, EGL10.EglDepthSize, 24, EGL10.EglStencilSize, 8, EGL10.EglRenderableType, 4, EGL10.EglNone },
            // 8-8-8-8-bit color, 8-bit stencil, 16-bit z buffer. Better than 5-6-5/16 bit, should also fix problems on some obscure devices.
            new int[] { EGL10.EglRedSize, 8, EGL10.EglGreenSize, 8, EGL10.EglBlueSize, 8, EGL10.EglAlphaSize, 8, EGL10.EglDepthSize, 16, EGL10.EglStencilSize, 8, EGL10.EglRenderableType, 4, EGL10.EglNone },
            // 5-6-5-bit color, 24-bit z buffer. Should work on most devices.
            new int[] { EGL10.EglRedSize, 5, EGL10.EglGreenSize, 6, EGL10.EglBlueSize, 5, EGL10.EglDepthSize, 24, EGL10.EglRenderableType, 4, EGL10.EglNone },
            // 5-6-5-bit color, 16-bit z buffer. Fallback for original Tegra devices. 
            new int[] { EGL10.EglRedSize, 5, EGL10.EglGreenSize, 6, EGL10.EglBlueSize, 5, EGL10.EglDepthSize, 16, EGL10.EglRenderableType, 4, EGL10.EglNone },
            // 5-6-5-bit color, unspecified z/stencil buffer.
            new int[] { EGL10.EglRedSize, 5, EGL10.EglGreenSize, 6, EGL10.EglBlueSize, 5, EGL10.EglRenderableType, 4, EGL10.EglNone },
        };

        public Javax.Microedition.Khronos.Egl.EGLConfig ChooseConfig(Javax.Microedition.Khronos.Egl.IEGL10 egl, Javax.Microedition.Khronos.Egl.EGLDisplay eglDisplay) {
            Carto.Utils.Log.Debug("ConfigChooser.ChooseConfig: Model: " + Android.OS.Build.Model + ", board: " + Android.OS.Build.Board + ", product: " + Android.OS.Build.Product);

            for (int i = 0; i < ATTRIBUTE_TABLE.Length; i++) {
                int[] numConfigs = new int[] { 0 };
                EGLConfig[] configs = new EGLConfig[1];
                if (egl.EglChooseConfig(eglDisplay, ATTRIBUTE_TABLE[i], configs, 1, numConfigs)) {
                    if (numConfigs[0] > 0) {
                        Carto.Utils.Log.Debug("ConfigChooser.ChooseConfig: Selected display configuration: " + i);
                        return configs[0];
                    }
                }
            }
            return null;
        }
    }
}
