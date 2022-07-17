#include "ui/MapRedrawRequestListener.h"
#include "ui/MapView.h"

namespace carto {
    MapRedrawRequestListener::MapRedrawRequestListener(MapView* mapView) {
        this->_mapView = mapView;
    }

    void MapRedrawRequestListener::onRedrawRequested() const {
        _mapView->requestRender();
    }
}