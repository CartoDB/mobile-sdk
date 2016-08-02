#include "TileDataSource.h"
#include "projections/Projection.h"
#include "projections/EPSG3857.h"
#include "utils/Const.h"

#include <algorithm>
#include <sstream>

#include <boost/lexical_cast.hpp>

namespace carto {

    TileDataSource::~TileDataSource() {
    }
        
    int TileDataSource::getMinZoom() const {
        return _minZoom;
    }
    
    int TileDataSource::getMaxZoom() const {
        return _maxZoom;
    }
    
    std::shared_ptr<Projection> TileDataSource::getProjection() const {
        return _projection;
    }
    
    void TileDataSource::notifyTilesChanged(bool removeTiles) {
        std::vector<std::shared_ptr<OnChangeListener> > onChangeListeners;
        {
            std::lock_guard<std::mutex> lock(_onChangeListenersMutex);
            onChangeListeners = _onChangeListeners;
        }
        for (const std::shared_ptr<OnChangeListener>& listener : onChangeListeners) {
            listener->onTilesChanged(removeTiles);
        }
    }
        
    void TileDataSource::registerOnChangeListener(const std::shared_ptr<OnChangeListener>& listener) {
        std::lock_guard<std::mutex> lock(_onChangeListenersMutex);
        _onChangeListeners.push_back(listener);
    }
    
    void TileDataSource::unregisterOnChangeListener(const std::shared_ptr<OnChangeListener>& listener) {
        std::lock_guard<std::mutex> lock(_onChangeListenersMutex);
        _onChangeListeners.erase(std::remove(_onChangeListeners.begin(), _onChangeListeners.end(), listener), _onChangeListeners.end());
    }
    
    TileDataSource::TileDataSource() :
        _minZoom(0),
        _maxZoom(Const::MAX_SUPPORTED_ZOOM_LEVEL),
        _projection(std::make_shared<EPSG3857>()),
        _onChangeListeners(),
        _onChangeListenersMutex()
    {
    }

    TileDataSource::TileDataSource(int minZoom, int maxZoom) :
        _minZoom(std::max(0, minZoom)),
        _maxZoom(std::min(static_cast<int>(Const::MAX_SUPPORTED_ZOOM_LEVEL), maxZoom)),
        _projection(std::make_shared<EPSG3857>()),
        _onChangeListeners(),
        _onChangeListenersMutex()
    {
    }
    
    std::map<std::string, std::string> TileDataSource::buildTagValues(const MapTile& tile) const {
        std::map<std::string, std::string> tagValues;

        std::stringstream ss;
        for (int i = tile.getZoom() - 1; i >= 0; i--) {
            ss << (((tile.getY() >> i) & 1) * 2 + ((tile.getX() >> i) & 1));
        }
        tagValues["quadkey"] = ss.str();

        tagValues["x"] = boost::lexical_cast<std::string>(tile.getX());
        tagValues["xflipped"] = boost::lexical_cast<std::string>((1 << tile.getZoom()) - 1 - tile.getX());

        tagValues["y"] = boost::lexical_cast<std::string>(tile.getY());
        tagValues["yflipped"] = boost::lexical_cast<std::string>((1 << tile.getZoom()) - 1 - tile.getY());

        tagValues["z"] = boost::lexical_cast<std::string>(tile.getZoom());
        tagValues["zoom"] = boost::lexical_cast<std::string>(tile.getZoom());

        return tagValues;
    }

}

