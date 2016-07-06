#include "Options.h"
#include "assets/DefaultBackgroundPNG.h"
#include "assets/DefaultSkyPNG.h"
#include "assets/CartoWatermarkPNG.h"
#include "components/CancelableThreadPool.h"
#include "graphics/Bitmap.h"
#include "projections/EPSG3857.h"
#include "utils/Const.h"
#include "utils/Log.h"
#include "utils/GeneralUtils.h"

#include <algorithm>

namespace carto {

    Options::Options(const std::shared_ptr<CancelableThreadPool>& envelopeThreadPool, const std::shared_ptr<CancelableThreadPool>& tileThreadPool) :
        _ambientLightColor(0xFF707070),
        _mainLightColor(0xFF8F8F8F),
        _mainLightDir(0.35, 0.35, -0.87),
        _projectionMode(ProjectionMode::PROJECTION_MODE_PERSPECTIVE),
        _clickTypeDetection(true),
        _tileDrawSize(256),
        _dpi(160.0f),
        _drawDistance(16),
        _fovY(70),
        _panningMode(PanningMode::PANNING_MODE_FREE),
        _pivotMode(PivotMode::PIVOT_MODE_TOUCHPOINT),
        _seamlessPanning(true),
        _tiltGestureReversed(false),
        _backgroundBitmap(GetDefaultBackgroundBitmap()),
        _skyBitmap(GetDefaultSkyBitmap()),
        _watermarkAlignmentX(-1),
        _watermarkAlignmentY(-1),
        _watermarkBitmap(GetDefaultWatermarkBitmap()),
        _watermarkPadding(4, 4),
        _watermarkScale(1.0f),
        _userInput(true),
        _kineticPan(true),
        _kineticRotation(true),
        _kineticZoom(true),
        _rotatable(true),
        _tiltRange(Const::MIN_SUPPORTED_TILT_ANGLE, 90.0f),
        _zoomRange(0.0, Const::MAX_SUPPORTED_ZOOM_LEVEL),
        _panBounds(MapPos(-Const::HALF_WORLD_SIZE, -Const::HALF_WORLD_SIZE, 0), MapPos(Const::HALF_WORLD_SIZE, Const::HALF_WORLD_SIZE, 0)),
        _focusPointOffset(0, 0),
        _baseProjection(std::make_shared<EPSG3857>()),
        _envelopeThreadPool(envelopeThreadPool),
        _tileThreadPool(tileThreadPool),
        _mutex()
    {
        setEnvelopeThreadPoolSize(1);
        setTileThreadPoolSize(1);
    }
    
    Options::~Options() {
    }
        
    Color Options::getAmbientLightColor() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _ambientLightColor;
    }
    
    void Options::setAmbientLightColor(const Color& color) {
        {
            std::lock_guard<std::mutex> lock(_mutex);
            if (_ambientLightColor == color) {
                return;
            }
            _ambientLightColor = color;
        }
        notifyOptionChanged("AmbientLightColor");
    }
    
    Color Options::getMainLightColor() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _mainLightColor;
    }
    
    void Options::setMainLightColor(const Color& color) {
        {
            std::lock_guard<std::mutex> lock(_mutex);
            if (_mainLightColor == color) {
                return;
            }
            _mainLightColor = color;
        }
        notifyOptionChanged("MainLightColor");
    }
    
    MapVec Options::getMainLightDirection() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _mainLightDir;
    }
    
    void Options::setMainLightDirection(const MapVec& direction) {
        {
            std::lock_guard<std::mutex> lock(_mutex);
            MapVec directionNormalized = direction;
            directionNormalized.normalize();
            if (_mainLightDir == directionNormalized) {
                return;
            }
            _mainLightDir = directionNormalized;
        }
        notifyOptionChanged("MainLightDirection");
    }
    
    ProjectionMode::ProjectionMode Options::getProjectionMode() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _projectionMode;
    }
    
    void Options::setProjectionMode(ProjectionMode::ProjectionMode projectionMode) {
        {
            std::lock_guard<std::mutex> lock(_mutex);
            if (_projectionMode == projectionMode) {
                return;
            }
            _projectionMode = projectionMode;
        }
        notifyOptionChanged("ProjectionMode");
    }
    
    bool Options::isClickTypeDetection() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _clickTypeDetection;
    }
    
    void Options::setClickTypeDetection(bool enabled) {
        {
            std::lock_guard<std::mutex> lock(_mutex);
            if (_clickTypeDetection == enabled) {
                return;
            }
            _clickTypeDetection = enabled;
        }
        notifyOptionChanged("ClickTypeDetection");
    }
    
    int Options::getTileDrawSize() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _tileDrawSize;
    }
    
    void Options::setTileDrawSize(int tileDrawSize) {
        {
            std::lock_guard<std::mutex> lock(_mutex);
            if (_tileDrawSize == tileDrawSize) {
                return;
            }
            _tileDrawSize = tileDrawSize;
        }
        notifyOptionChanged("TileDrawSize");
    }
    
    float Options::getDPI() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _dpi;
    }
    
    void Options::setDPI(float dpi) {
        {
            std::lock_guard<std::mutex> lock(_mutex);
            if (_dpi == dpi) {
                return;
            }
            _dpi = dpi;
        }
        notifyOptionChanged("DPI");
    }
    
    float Options::getDrawDistance() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _drawDistance;
    }
    
    void Options::setDrawDistance(float drawDistance) {
        {
            std::lock_guard<std::mutex> lock(_mutex);
            if (_drawDistance == drawDistance) {
                return;
            }
            _drawDistance = drawDistance;
        }
        notifyOptionChanged("DrawDistance");
    }
    
    int Options::getFieldOfViewY() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _fovY;
    }
    
    void Options::setFieldOfViewY(int fovY) {
        {
            std::lock_guard<std::mutex> lock(_mutex);
            if (_fovY == fovY) {
                return;
            }
            _fovY = fovY;
        }
        notifyOptionChanged("FieldOfViewY");
    }
    
    PanningMode::PanningMode Options::getPanningMode() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _panningMode;
    }
    
    void Options::setPanningMode(PanningMode::PanningMode panningMode) {
        {
            std::lock_guard<std::mutex> lock(_mutex);
            if (_panningMode == panningMode) {
                return;
            }
            _panningMode = panningMode;
        }
        notifyOptionChanged("PanningMode");
    }
    
    PivotMode::PivotMode Options::getPivotMode() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _pivotMode;
    }

    void Options::setPivotMode(PivotMode::PivotMode pivotMode) {
        {
            std::lock_guard<std::mutex> lock(_mutex);
            if (_pivotMode == pivotMode) {
                return;
            }
            _pivotMode = pivotMode;
        }
        notifyOptionChanged("PivotMode");
    }

    bool Options::isSeamlessPanning() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _seamlessPanning;
    }
    
    void Options::setSeamlessPanning(bool enabled) {
        {
            std::lock_guard<std::mutex> lock(_mutex);
            if (_seamlessPanning == enabled) {
                return;
            }
            _seamlessPanning = enabled;
        }
        notifyOptionChanged("SeamlessPanning");
    }
        
    bool Options::isTiltGestureReversed() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _tiltGestureReversed;
    }
    
    void Options::setTiltGestureReversed(bool reversed) {
        {
            std::lock_guard<std::mutex> lock(_mutex);
            if (_tiltGestureReversed == reversed) {
                return;
            }
            _tiltGestureReversed = reversed;
        }
        notifyOptionChanged("TiltGestureReversed");
    }
        
    int Options::getEnvelopeThreadPoolSize() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _envelopeThreadPool->getPoolSize();
    }
    
    void Options::setEnvelopeThreadPoolSize(int poolSize) {
        {
            std::lock_guard<std::mutex> lock(_mutex);
            if (_envelopeThreadPool->getPoolSize() == poolSize) {
                return;
            }
            _envelopeThreadPool->setPoolSize(poolSize);
        }
        notifyOptionChanged("EnvelopeThreadPoolSize");
    }
    
    int Options::getTileThreadPoolSize() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _tileThreadPool->getPoolSize();
    }
    
    void Options::setTileThreadPoolSize(int poolSize) {
        {
            std::lock_guard<std::mutex> lock(_mutex);
            if (_tileThreadPool->getPoolSize() == poolSize) {
                return;
            }
            _tileThreadPool->setPoolSize(poolSize);
        }
        notifyOptionChanged("TileThreadPoolSize");
    }
    
    std::shared_ptr<Bitmap> Options::getBackgroundBitmap() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _backgroundBitmap;
    }
    
    void Options::setBackgroundBitmap(const std::shared_ptr<Bitmap>& backgroundBitmap) {
        {
            std::lock_guard<std::mutex> lock(_mutex);
            if (_backgroundBitmap == backgroundBitmap) {
                return;
            }
            _backgroundBitmap = backgroundBitmap;
        }
        notifyOptionChanged("BackgroundBitmap");
    }
    
    std::shared_ptr<Bitmap> Options::getSkyBitmap() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _skyBitmap;
    }
    
    void Options::setSkyBitmap(const std::shared_ptr<Bitmap>& skyBitmap) {
        {
            std::lock_guard<std::mutex> lock(_mutex);
            if (_skyBitmap == skyBitmap) {
                return;
            }
            _skyBitmap = skyBitmap;
        }
        notifyOptionChanged("SkyBitmap");
    }
        
    float Options::getWatermarkAlignmentX() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _watermarkAlignmentX;
    }
    
    void Options::setWatermarkAlignmentX(float alignmentX) {
        {
            std::lock_guard<std::mutex> lock(_mutex);
            float alignmentXClipped = GeneralUtils::Clamp(alignmentX, -1.0f, 1.0f);
            if (_watermarkAlignmentX == alignmentXClipped) {
                return;
            }
            _watermarkAlignmentX = alignmentXClipped;
        }
        notifyOptionChanged("WatermarkAlignment");
    }
        
    float Options::getWatermarkAlignmentY() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _watermarkAlignmentY;
    }
        
    void Options::setWatermarkAlignmentY(float alignmentY) {
        {
            std::lock_guard<std::mutex> lock(_mutex);
            float alignmentYClipped = GeneralUtils::Clamp(alignmentY, -1.0f, 1.0f);
            if (_watermarkAlignmentY == alignmentYClipped) {
                return;
            }
            _watermarkAlignmentY = alignmentYClipped;
        }
        notifyOptionChanged("WatermarkAlignment");
    }
        
    float Options::getWatermarkScale() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _watermarkScale;
    }
        
    void Options::setWatermarkScale(float scale) {
        {
            std::lock_guard<std::mutex> lock(_mutex);
            if (_watermarkScale == scale) {
                return;
            }
            _watermarkScale = scale;
        }
        notifyOptionChanged("WatermarkScale");
    }
        
    std::shared_ptr<Bitmap> Options::getWatermarkBitmap() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _watermarkBitmap;
    }
    
    void Options::setWatermarkBitmap(const std::shared_ptr<Bitmap>& watermarkBitmap) {
        {
            std::lock_guard<std::mutex> lock(_mutex);
            if (_watermarkBitmap == watermarkBitmap) {
                return;
            }
            _watermarkBitmap = watermarkBitmap;
        }
        notifyOptionChanged("WatermarkBitmap");
    }
        
    ScreenPos Options::getWatermarkPadding() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _watermarkPadding;
    }
    
    void Options::setWatermarkPadding(const ScreenPos& padding) {
        {
            std::lock_guard<std::mutex> lock(_mutex);
            if (_watermarkPadding == padding) {
                return;
            }
            _watermarkPadding = padding;
        }
        notifyOptionChanged("WatermarkPadding");
    }
    
    bool Options::isUserInput() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _userInput;
    }
    
    void Options::setUserInput(bool enabled) {
        {
            std::lock_guard<std::mutex> lock(_mutex);
            if (_userInput == enabled) {
                return;
            }
            _userInput = enabled;
        }
        notifyOptionChanged("UserInput");
    }
    
    bool Options::isKineticPan() {
        std::lock_guard<std::mutex> lock(_mutex);
        return _kineticPan;
    }
    
    void Options::setKineticPan(bool enabled) {
        {
            std::lock_guard<std::mutex> lock(_mutex);
            if (_kineticPan == enabled) {
                return;
            }
            _kineticPan = enabled;
        }
        notifyOptionChanged("KineticPan");
    }
    
    bool Options::isKineticRotation() {
        std::lock_guard<std::mutex> lock(_mutex);
        return _kineticRotation;
    }
    
    void Options::setKineticRotation(bool enabled) {
        {
            std::lock_guard<std::mutex> lock(_mutex);
            if (_kineticRotation == enabled) {
                return;
            }
            _kineticRotation = enabled;
        }
        notifyOptionChanged("KineticRotation");
    }
        
    bool Options::isKineticZoom() {
        std::lock_guard<std::mutex> lock(_mutex);
        return _kineticZoom;
    }
    
    void Options::setKineticZoom(bool enabled) {
        {
            std::lock_guard<std::mutex> lock(_mutex);
            if (_kineticZoom == enabled) {
                return;
            }
            _kineticZoom = enabled;
        }
        notifyOptionChanged("KineticZoom");
    }
    
    bool Options::isRotatable() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _rotatable;
    }
    
    void Options::setRotatable(bool rotatable) {
        {
            std::lock_guard<std::mutex> lock(_mutex);
            if (_rotatable == rotatable) {
                return;
            }
            _rotatable = rotatable;
        }
        notifyOptionChanged("Rotatable");
    }
    
    MapRange Options::getTiltRange() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _tiltRange;
    }
    
    void Options::setTiltRange(const MapRange& tiltRange) {
        {
            std::lock_guard<std::mutex> lock(_mutex);
            float min = GeneralUtils::Clamp(tiltRange.getMin(), Const::MIN_SUPPORTED_TILT_ANGLE, 90.0f);
            float max = GeneralUtils::Clamp(tiltRange.getMax(), Const::MIN_SUPPORTED_TILT_ANGLE, 90.0f);
            MapRange tiltRangeClipped(min, max);
            if (_tiltRange == tiltRangeClipped) {
                return;
            }
            _tiltRange = tiltRangeClipped;
        }
        notifyOptionChanged("TiltRange");
    }
    
    MapRange Options::getZoomRange() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _zoomRange;
    }
    
    void Options::setZoomRange(const MapRange& zoomRange) {
        {
            std::lock_guard<std::mutex> lock(_mutex);
            float min = GeneralUtils::Clamp(zoomRange.getMin(), 0.0f,
                static_cast<float>(Const::MAX_SUPPORTED_ZOOM_LEVEL));
            float max = GeneralUtils::Clamp(zoomRange.getMax(), 0.0f,
                static_cast<float>(Const::MAX_SUPPORTED_ZOOM_LEVEL));
            MapRange zoomRangeClipped(min, max);
            if (_zoomRange == zoomRangeClipped) {
                return;
            }
            _zoomRange = zoomRangeClipped;
        }
        notifyOptionChanged("ZoomRange");
    }
        
    MapBounds Options::getInternalPanBounds() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _panBounds;
    }
    
    MapBounds Options::getPanBounds() const {
        std::lock_guard<std::mutex> lock(_mutex);
        const MapPos& projectionMin = _baseProjection->fromInternal(_panBounds.getMin());
        const MapPos& projectionMax = _baseProjection->fromInternal(_panBounds.getMax());
        return MapBounds(projectionMin, projectionMax);
    }
    
    void Options::setPanBounds(const MapBounds& panBounds) {
        {
            std::lock_guard<std::mutex> lock(_mutex);
            double halfWorldSize = Const::HALF_WORLD_SIZE;
            const MapPos& internalMin = _baseProjection->toInternal(panBounds.getMin());
            const MapPos& internalMax = _baseProjection->toInternal(panBounds.getMax());
            double left = GeneralUtils::Clamp(internalMin.getX(), -halfWorldSize, halfWorldSize);
            double bottom = GeneralUtils::Clamp(internalMin.getY(), -halfWorldSize, halfWorldSize);
            double right = GeneralUtils::Clamp(internalMax.getX(), -halfWorldSize, halfWorldSize);
            double top = GeneralUtils::Clamp(internalMax.getY(), -halfWorldSize, halfWorldSize);
            MapBounds panBoundsClipped(MapPos(left, bottom, 0), MapPos(right, top, 0));
            if (_panBounds == panBoundsClipped) {
                return;
            }
            _panBounds = panBoundsClipped;
        }
        notifyOptionChanged("PanBounds");
    }
    
    ScreenPos Options::getFocusPointOffset() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _focusPointOffset;
    }
    
    void Options::setFocusPointOffset(const ScreenPos& offset) {
        {
            std::lock_guard<std::mutex> lock(_mutex);
            if (_focusPointOffset == offset) {
                return;
            }
            _focusPointOffset = offset;
        }
        notifyOptionChanged("FocusPointOffset");
    }
    
    std::shared_ptr<Projection> Options::getBaseProjection() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _baseProjection;
    }
    
    void Options::setBaseProjection(const std::shared_ptr<Projection>& baseProjection) {
        {
            std::lock_guard<std::mutex> lock(_mutex);
            if (_baseProjection == baseProjection) {
            	return;
            }
            _baseProjection = baseProjection;
        }
        notifyOptionChanged("BaseProjection");
    }
    
    std::shared_ptr<Bitmap> Options::GetDefaultBackgroundBitmap() {
        std::lock_guard<std::mutex> lock(_Mutex);
        if (!_DefaultBackgroundBitmap) {
            _DefaultBackgroundBitmap = Bitmap::CreateFromCompressed(default_background_png, default_background_png_len);
        }
        return _DefaultBackgroundBitmap;
    }
    
    std::shared_ptr<Bitmap> Options::GetDefaultSkyBitmap() {
        std::lock_guard<std::mutex> lock(_Mutex);
        if (!_DefaultSkyBitmap) {
            _DefaultSkyBitmap = Bitmap::CreateFromCompressed(default_sky_png, default_sky_png_len);
        }
        return _DefaultSkyBitmap;
    }
        
    std::shared_ptr<Bitmap> Options::GetDefaultWatermarkBitmap() {
        std::lock_guard<std::mutex> lock(_Mutex);
        if (!_DefaultWatermarkBitmap) {
            _DefaultWatermarkBitmap = Bitmap::CreateFromCompressed(carto_watermark_png, carto_watermark_png_len);
        }
        return _DefaultWatermarkBitmap;
    }
        
    void Options::notifyOptionChanged(const std::string& optionName) {
        std::vector<std::shared_ptr<OnChangeListener> > onChangeListeners;
        {
            std::lock_guard<std::mutex> lock(_onChangeListenersMutex);
            onChangeListeners = _onChangeListeners;
        }
        for (const std::shared_ptr<OnChangeListener>& listener : onChangeListeners) {
            listener->onOptionChanged(optionName);
        }
    }

    void Options::registerOnChangeListener(const std::shared_ptr<OnChangeListener>& listener) {
        std::lock_guard<std::mutex> lock(_onChangeListenersMutex);
        _onChangeListeners.push_back(listener);
    }

    void Options::unregisterOnChangeListener(const std::shared_ptr<OnChangeListener>& listener) {
        std::lock_guard<std::mutex> lock(_onChangeListenersMutex);
        _onChangeListeners.erase(std::remove(_onChangeListeners.begin(), _onChangeListeners.end(), listener), _onChangeListeners.end());
    }
    
    std::mutex Options::_Mutex;
    std::shared_ptr<Bitmap> Options::_DefaultBackgroundBitmap;
    std::shared_ptr<Bitmap> Options::_DefaultSkyBitmap;
    std::shared_ptr<Bitmap> Options::_DefaultWatermarkBitmap;
    
}
