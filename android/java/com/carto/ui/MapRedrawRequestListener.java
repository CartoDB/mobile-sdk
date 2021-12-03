package com.carto.ui;

import java.lang.ref.WeakReference;

import com.carto.renderers.RedrawRequestListener;

/**
 * Android-specific implementation of RedrawRequestListener, used by MapView.
 * It is not intended for public usage.
 * @hidden
 */
public class MapRedrawRequestListener extends RedrawRequestListener {
    private final WeakReference<MapView> mapViewRef;
    
    public MapRedrawRequestListener(MapView mapView) {
        mapViewRef = new WeakReference<MapView>(mapView);
    }

    public void onRedrawRequested() {
        MapView mapView = mapViewRef.get();
        if (mapView != null) {
            mapView.requestRender();
        }
    }
}
