namespace Carto.Ui {
    using System;
    using Carto.Renderers;

    // WinPhone-specific implementation of RedrawRequestListener.
    internal class MapViewRedrawRequestListener : RedrawRequestListener {
        private readonly WeakReference<MapView> _mapViewRef;

        public MapViewRedrawRequestListener(MapView mapView) {
            _mapViewRef = new WeakReference<MapView>(mapView);
        }

        public override void OnRedrawRequested() {
            MapView mapView = null;
            if (_mapViewRef.TryGetTarget(out mapView)) {
                if (mapView != null) { // check probably not needed
                    mapView.Redraw();
                }
            }
        }
    }
}
