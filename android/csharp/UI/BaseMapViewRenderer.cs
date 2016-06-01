namespace Carto.Ui {
    using System;
    using Javax.Microedition.Khronos.Egl;
    using Javax.Microedition.Khronos.Opengles;
    using Android.Opengl;

    // GLSurfaceView.IRenderer implementation for MapView
    internal class BaseMapViewRenderer : Java.Lang.Object, GLSurfaceView.IRenderer {
        private BaseMapView _baseMapView;

        public BaseMapViewRenderer(BaseMapView baseMapView) {
            _baseMapView = baseMapView;
        }

        public void Detach() {
            lock (this) {
                _baseMapView = null;
            }
        }

        public void OnSurfaceCreated(IGL10 gl, Javax.Microedition.Khronos.Egl.EGLConfig config) {
            lock (this) {
                if (_baseMapView != null) {
                    _baseMapView.OnSurfaceCreated();
                }
            }
        }

        public void OnSurfaceChanged(IGL10 gl, int width, int height) {
            lock (this) {
                if (_baseMapView != null) {
                    _baseMapView.OnSurfaceChanged(width, height);
                }
            }
        }

        public void OnDrawFrame(IGL10 gl) {
            lock (this) {
                if (_baseMapView != null) {
                    _baseMapView.OnDrawFrame();
                }
            }
        }
    }
}
