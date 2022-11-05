package com.carto.ui;

import java.lang.ref.WeakReference;

import com.carto.renderers.RedrawRequestListener;

/**
 * Android-specific implementation of RedrawRequestListener, used by TextureMapView.
 * It is not intended for public usage.
 * @hidden
 */
public class TextureMapRedrawRequestListener extends RedrawRequestListener {
    private final WeakReference<TextureMapView> mapViewRef;
    
    public TextureMapRedrawRequestListener(TextureMapView mapView) {
        mapViewRef = new WeakReference<TextureMapView>(mapView);
    }

    public void onRedrawRequested() {
        TextureMapView mapView = mapViewRef.get();
        if (mapView != null) {
            mapView.requestRender();
        }
    }
}
