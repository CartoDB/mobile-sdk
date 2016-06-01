namespace Carto.Ui {
    using System;
    using Android.Opengl;
    using Carto.Renderers;

    // Android-specific implementation of RedrawRequestListener.
    internal class MapRedrawRequestListener : RedrawRequestListener {
        private WeakReference<MapView> _mapView;
    
        public MapRedrawRequestListener(MapView mapView) {
            _mapView = new WeakReference<MapView>(mapView);
        }

        public void Detach() {
            lock (this) {
                _mapView = null;
            }
        }

        public override void OnRedrawRequested() {
            try {
                MapView mapView = null;
                lock (this) {
                    _mapView.TryGetTarget(out mapView);
                }

                if (mapView != null) {
                    mapView.RequestRender();
                }
            }
            catch (Java.Lang.Exception e) {
                Carto.Utils.Log.Error("MapRedrawRequestListener.OnRedrawRequested: Java exception: " + e);
            }
            catch (System.Exception e) {
                Carto.Utils.Log.Error("MapRedrawRequestListener.OnRedrawRequested: " + e);
            }
        }
    }

}
