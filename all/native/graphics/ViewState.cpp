#include "ViewState.h"
#include "core/ScreenBounds.h"
#include "projections/Projection.h"
#include "projections/ProjectionSurface.h"
#include "utils/Const.h"
#include "utils/GeneralUtils.h"
#include "utils/Log.h"

#include <cmath>
#include <cglib/mat.h>

namespace carto {

    ViewState::ViewState() :
        _cameraPos(0, 0, 1),
        _focusPos(0, 0, 0),
        _upVec(0, 1, 0),
        _cameraChanged(true),
        _rotation(0),
        _tilt(90),
        _zoom(0.0f),
        _2PowZoom(1.0f),
        _zoom0Distance(0.0f),
        _minZoom(0.0f),
        _ignoreMinZoom(false),
        _zoomRange(0.0f, 0.0f),
        _restrictedPanning(false),
        _normalizedResolution(0.0f),
        _width(0),
        _height(0),
        _halfWidth(0.0f),
        _halfHeight(0.0f),
        _aspectRatio(0.0f),
        _screenSizeChanged(false),
        _near(0.0f),
        _far(0.0f),
        _skyVisible(false),
        _fovY(0),
        _halfFOVY(0.0f),
        _tanHalfFOVY(0.0f),
        _cosHalfFOVY(0.0f),
        _tanHalfFOVX(0.0f),
        _cosHalfFOVXY(0.0f),
        _tileDrawSize(0),
        _dpToPX(0),
        _dpi(0),
        _unitToPXCoef(0),
        _unitToDPCoef(0),
        _rotationState(),
        _projectionSurface(),
        _projectionMat(),
        _modelviewMat(),
        _modelviewProjectionMat(),
        _rteModelviewMat(),
        _rteModelviewProjectionMat(),
        _rteSkyProjectionMat(),
        _horizontalLayerOffsetDir(0)
    {
    }
    
    ViewState::~ViewState() {
    }
    
    const cglib::vec3<double>& ViewState::getCameraPos() const {
        return _cameraPos;
    }
    
    void ViewState::setCameraPos(const cglib::vec3<double>& cameraPos) {
        _cameraPos = cameraPos;
    }
    
    const cglib::vec3<double>& ViewState::getFocusPos() const {
        return _focusPos;
    }
    
    void ViewState::setFocusPos(const cglib::vec3<double>& focusPos) {
        _focusPos = focusPos;
    }
    
    const cglib::vec3<double>& ViewState::getUpVec() const {
        return _upVec;
    }
    
    void ViewState::setUpVec(const cglib::vec3<double>& upVec) {
        _upVec = upVec;
    }
    
    float ViewState::getRotation() const {
        return _rotation;
    }
    
    void ViewState::setRotation(float rotation) {
        _rotation = rotation;
    }
    
    float ViewState::getTilt() const {
        return _tilt;
    }
    
    void ViewState::setTilt(float tilt) {
        _tilt = tilt;
    }
    
    float ViewState::getZoom() const {
        return _zoom;
    }
    
    void ViewState::setZoom(float zoom) {
        _zoom = zoom;
        _2PowZoom = std::pow(2.0f, zoom);
    }
    
    bool ViewState::isCameraChanged() const {
        return _cameraChanged;
    }
    
    void ViewState::cameraChanged() {
        _cameraChanged = true;
    }
    
    float ViewState::get2PowZoom() const {
        return _2PowZoom;
    }
    
    float ViewState::getZoom0Distance() const {
        return _zoom0Distance;
    }

    float ViewState::getMinZoom() const {
        return _minZoom;
    }
    
    float ViewState::getNormalizedResolution() const {
        return _normalizedResolution;
    }
    
    int ViewState::getWidth() const {
        return _width;
    }
    
    int ViewState::getHeight() const {
        return _height;
    }
    
    float ViewState::getHalfWidth() const {
        return _halfWidth;
    }
    
    float ViewState::getHalfHeight() const {
        return _halfHeight;
    }
    
    float ViewState::getAspectRatio() const {
        return _aspectRatio;
    }
    
    float ViewState::getNear() const {
        return _near;
    }
    
    float ViewState::getFar() const {
        return _far;
    }
    
    int ViewState::getFOVY() const {
        return _fovY;
    }
    
    float ViewState::getHalfFOVY() const {
        return _halfFOVY;
    }
    
    double ViewState::getTanHalfFOVY() const {
        return _tanHalfFOVY;
    }
    
    double ViewState::getCosHalfFOVY() const {
        return _cosHalfFOVY;
    }
    
    double ViewState::getTanHalfFOVX() const {
        return _tanHalfFOVX;
    }
    
    double ViewState::getCosHalfFOVXY() const {
        return _cosHalfFOVXY;
    }
        
    float ViewState::getDPToPX() const {
        return _dpToPX;
    }
        
    float ViewState::getDPI() const {
        return _dpi;
    }
    
    float ViewState::getUnitToPXCoef() const {
        return _unitToPXCoef;
    }
    
    float ViewState::getUnitToDPCoef() const {
        return _unitToDPCoef;
    }
    
    const ViewState::RotationState& ViewState::getRotationState() const {
        return _rotationState;
    }
    
    std::shared_ptr<ProjectionSurface> ViewState::getProjectionSurface() const {
        return _projectionSurface;
    }
    
    const cglib::mat4x4<double>& ViewState::getProjectionMat() const {
        return _projectionMat;
    }
    
    const cglib::mat4x4<double>& ViewState::getModelviewMat() const {
        return _modelviewMat;
    }
    
    const cglib::mat4x4<double>& ViewState::getModelviewProjectionMat() const {
        return _modelviewProjectionMat;
    }
    
    const cglib::mat4x4<float>& ViewState::getRTEModelviewMat() const {
        return _rteModelviewMat;
    }
    
    const cglib::mat4x4<float>& ViewState::getRTEModelviewProjectionMat() const {
        return _rteModelviewProjectionMat;
    }
        
    const cglib::mat4x4<float>& ViewState::getRTESkyProjectionMat() const {
        return _rteSkyProjectionMat;
    }

    const cglib::frustum3<double>& ViewState::getFrustum() const {
        return _frustum;
    }
    
    int ViewState::getScreenWidth() const {
        return _width;
    }
        
    int ViewState::getScreenHeight() const {
        return _height;
    }
    
    void ViewState::setScreenSize(int width, int height) {
        _width = width;
        _height = height;
    
        _halfWidth = _width / 2.0f;
        _halfHeight = _height / 2.0f;
    
        if (_height <= 0) {
            _height = 1;
        }
        _aspectRatio = (float) _width / _height;
    
        _screenSizeChanged = true;
    }

    void ViewState::clampZoom(const Options& options) {
        if (!options.isRestrictedPanning() || _width <= 0 || _height <= 0) {
            return;
        }

        MapRange zoomRange = options.getZoomRange();
        float zoom = GeneralUtils::Clamp(_zoom, getMinZoom(), zoomRange.getMax());

        if (zoom != getZoom() && _zoom0Distance > 0) {
            cglib::vec3<double> cameraVec = _cameraPos - _focusPos;
            double length = cglib::length(cameraVec);
            double newLength = _zoom0Distance / std::pow(2.0f, zoom);
            cglib::vec3<double> cameraPos = _focusPos + cameraVec * (newLength / length);

            setZoom(zoom);
            setCameraPos(cameraPos);

            cameraChanged();
        }
    }

    void ViewState::clampFocusPos(const Options& options) {
        if (!options.isRestrictedPanning() || _width <= 0 || _height <= 0) {
            return;
        }

        std::shared_ptr<Projection> baseProjection = options.getBaseProjection();
        std::shared_ptr<ProjectionSurface> projectionSurface = options.getProjectionSurface();
        
        MapBounds boundsBase = options.getPanBounds();
        MapPos boundsPoses[2] = { baseProjection->toInternal(boundsBase.getMin()), baseProjection->toInternal(boundsBase.getMax()) };

        for (int j = 0; j < 4; j++) {
            if (options.isSeamlessPanning() && options.getRenderProjectionMode() == RenderProjectionMode::RENDER_PROJECTION_MODE_PLANAR) {
                if (j % 2 == 0) {
                    continue; // ignore X-based check if map is repeating along X
                }
            }

            cglib::vec3<double> cameraVec = _cameraPos - _focusPos;

            MapPos edgePos = projectionSurface->calculateMapPos(_focusPos);
            MapPos centerPos = edgePos;
            edgePos[j % 2] = boundsPoses[j / 2][j % 2];
            centerPos[j % 2] = (boundsPoses[0][j % 2] + boundsPoses[1][j % 2]) * 0.5;

            ViewState viewState;
            viewState._ignoreMinZoom = true;
            viewState._minZoom = _minZoom;
            viewState.setFocusPos(_focusPos);
            viewState.setCameraPos(_focusPos + projectionSurface->calculateNormal(projectionSurface->calculateMapPos(_focusPos)) * cglib::length(cameraVec));
            viewState.setUpVec(projectionSurface->calculateVector(projectionSurface->calculateMapPos(_focusPos), MapVec(0, 1, 0)));
            viewState.setZoom(_zoom);
            viewState.setScreenSize(_height, _height); // TODO: test this for regressions (NOT _width, _height)
            viewState.cameraChanged();
            viewState.calculateViewState(options);
            viewState.clampZoom(options);
            if (!viewState.getFrustum().inside(projectionSurface->calculatePosition(edgePos))) {
                continue;
            }

            MapBounds range(centerPos, edgePos);
            for (int i = 0; i < 24; i++) {
                MapPos focusPos = range.getCenter();
                viewState.setFocusPos(projectionSurface->calculatePosition(focusPos));
                viewState.setCameraPos(viewState.getFocusPos() + projectionSurface->calculateNormal(focusPos) * cglib::length(cameraVec));
                viewState.setUpVec(projectionSurface->calculateVector(focusPos, MapVec(0, 1, 0)));
                viewState.cameraChanged();
                viewState.calculateViewState(options);

                if (!viewState.getFrustum().inside(projectionSurface->calculatePosition(edgePos))) {
                    range.setMin(focusPos);
                } else {
                    range.setMax(focusPos);
                }
            }

            cglib::vec3<double> focusPos = projectionSurface->calculatePosition(range.getMin());
            if (focusPos != getFocusPos()) {
                cglib::mat4x4<double> transform = projectionSurface->calculateTranslateMatrix(getFocusPos(), focusPos, 1.0);
                setFocusPos(cglib::transform_point(getFocusPos(), transform));
                setCameraPos(cglib::transform_point(getCameraPos(), transform));
                setUpVec(cglib::transform_vector(getUpVec(), transform));

                cameraChanged();
            }
        }
    }

    cglib::vec3<float> ViewState::getFocusPosNormal() const {
        if (!_projectionSurface) {
            return cglib::vec3<float>(0, 0, 1);
        }
        return cglib::vec3<float>::convert(_projectionSurface->calculateNormal(_projectionSurface->calculateMapPos(_focusPos)));
    }

    bool ViewState::isSkyVisible() const {
        return _skyVisible;
    }
    
    void ViewState::calculateViewState(const Options& options) {
        // If FOV or tile draw size changed, recalculate zoom0Distance
        std::shared_ptr<ProjectionSurface> projectionSurface = options.getProjectionSurface();
        int FOVY = options.getFieldOfViewY();
        int tileDrawSize = options.getTileDrawSize();
        float dpi = options.getDPI();
        MapRange zoomRange = options.getZoomRange();
        bool restrictedPanning = options.isRestrictedPanning();
        if (projectionSurface != _projectionSurface || FOVY != _fovY || tileDrawSize != _tileDrawSize || dpi != _dpi || zoomRange != _zoomRange || restrictedPanning != _restrictedPanning || _screenSizeChanged) {
            _fovY = FOVY;
            _tileDrawSize = tileDrawSize;
            _dpToPX = dpi / Const::UNSCALED_DPI;
            _dpi = dpi;
            _screenSizeChanged = false;

            _halfFOVY = _fovY * 0.5f;
            _tanHalfFOVY = std::tan(static_cast<double>(_halfFOVY * Const::DEG_TO_RAD));
            _cosHalfFOVY = std::cos(static_cast<double>(_halfFOVY * Const::DEG_TO_RAD));

            _tanHalfFOVX = _aspectRatio * _tanHalfFOVY;
            _cosHalfFOVXY = std::cos(std::atan(_tanHalfFOVX)) * _cosHalfFOVY;

            _zoom0Distance = static_cast<float>(_height * Const::HALF_WORLD_SIZE / (tileDrawSize * _tanHalfFOVY * (_dpi / Const::UNSCALED_DPI)));

            if (!_ignoreMinZoom) {
                _minZoom = calculateMinZoom(options);
            }

            _zoomRange = zoomRange;
            _restrictedPanning = restrictedPanning;

            _normalizedResolution = 2 * tileDrawSize * (_dpi / Const::UNSCALED_DPI);

            // Recalculate camera orientation on projection change
            if (_projectionSurface != projectionSurface) {
                MapPos focusPosInternal(0, 0, 0);
                if (_projectionSurface) {
                    focusPosInternal = _projectionSurface->calculateMapPos(_focusPos);
                }
                _focusPos = projectionSurface->calculatePosition(focusPosInternal);

                double sin = std::sin(_rotation * Const::DEG_TO_RAD);
                double cos = std::cos(_rotation * Const::DEG_TO_RAD);

                // TODO: test this
                _cameraPos = _focusPos + projectionSurface->calculateNormal(focusPosInternal);
                _upVec = cglib::unit(projectionSurface->calculateVector(focusPosInternal, MapVec(sin, cos, 0)));

                cglib::vec3<double> axis = cglib::vector_product(_cameraPos - _focusPos, _upVec);
                if (cglib::length(axis) != 0) {
                    cglib::mat4x4<double> transform = cglib::rotate4_matrix(axis, (90 - _tilt) * Const::DEG_TO_RAD);
                    _cameraPos = _focusPos + cglib::transform_vector(_cameraPos - _focusPos, transform);
                    _upVec = cglib::transform_vector(_upVec, transform);
                }

                _projectionSurface = projectionSurface;
            }

            // Calculate new camera position
            if (_zoom0Distance > 0) {
                cglib::vec3<double> cameraVec = _cameraPos - _focusPos;
                double length = cglib::length(cameraVec);
                double newLength = _zoom0Distance / std::pow(2.0f, _zoom);
                _cameraPos = _focusPos + cameraVec * (newLength / length);
            }

            _cameraChanged = true;
        }

        if (_cameraChanged) {
            _cameraChanged = false;

            // Calculate scaling factor for vector elements
            _unitToPXCoef = static_cast<float>(_zoom0Distance / (_height * _tanHalfFOVY) / _2PowZoom);
            _unitToDPCoef = _unitToPXCoef * _dpi / Const::UNSCALED_DPI;

            calculateViewDistances(options, _near, _far, _skyVisible);

            // Matrices
            _projectionMat = calculatePerspMat(_halfFOVY, _near, _far, options);
            _modelviewMat = calculateLookatMat();

            // Rotation state
            cglib::mat4x4<double> invCameraMatrix = cglib::inverse(_modelviewMat);
            _rotationState.xAxis = cglib::vec3<float>::convert(cglib::proj_o(cglib::col_vector(invCameraMatrix, 0)));
            _rotationState.yAxis = cglib::vec3<float>::convert(cglib::proj_o(cglib::col_vector(invCameraMatrix, 1)));

            // Double precision mvp matrix and frustum
            _modelviewProjectionMat = _projectionMat * _modelviewMat;
            _frustum = cglib::gl_projection_frustum(_modelviewProjectionMat);

            // Rte modleview matrix only requires float precision
            _rteModelviewMat = cglib::mat4x4<float>::convert(_modelviewMat);
            _rteModelviewMat(0, 3) = 0.0f;
            _rteModelviewMat(1, 3) = 0.0f;
            _rteModelviewMat(2, 3) = 0.0f;

            // Float precision Rte mvp matrix
            _rteModelviewProjectionMat = cglib::mat4x4<float>::convert(_projectionMat) * _rteModelviewMat;

            // Calculate Rte sky matrix
            float skyFar = _zoom0Distance * options.getDrawDistance();
            cglib::mat4x4<double> skyProjectionMat = calculatePerspMat(_halfFOVY, _near, skyFar, options);
            _rteSkyProjectionMat = cglib::mat4x4<float>::convert(skyProjectionMat) * _rteModelviewMat;
        }
    }
    
    cglib::vec3<double> ViewState::screenToWorld(const cglib::vec2<float>& screenPos, double height, std::shared_ptr<Options> options) const {
        if (_width <= 0 || _height <= 0) {
            Log::Error("ViewState::screenToWorld: Failed to transform point from screen space to world plane, screen size is unknown");
            return cglib::vec3<double>(0, 0, 0);
        }

        std::shared_ptr<ProjectionSurface> projectionSurface = _projectionSurface;
        cglib::mat4x4<double> modelviewProjectionMat = _modelviewProjectionMat;
        if (options) {
            projectionSurface = options->getProjectionSurface();
            modelviewProjectionMat = calculateModelViewMat(*options);
        }
        if (!projectionSurface) {
            return cglib::vec3<double>(std::numeric_limits<double>::quiet_NaN(), std::numeric_limits<double>::quiet_NaN(), std::numeric_limits<double>::quiet_NaN());
        }
        cglib::mat4x4<double> invModelviewProjectionMat = cglib::inverse(modelviewProjectionMat);

        // Transform 2 points with different z values from world to screen
        cglib::vec3<double> screenCGPos0(screenPos(0) / _width * 2 - 1, 1 - screenPos(1) / _height * 2, -1);
        cglib::vec3<double> screenCGPos1(screenPos(0) / _width * 2 - 1, 1 - screenPos(1) / _height * 2,  1);
        cglib::vec3<double> worldCGPos0 = cglib::transform_point(screenCGPos0, invModelviewProjectionMat);
        cglib::vec3<double> worldCGPos1 = cglib::transform_point(screenCGPos1, invModelviewProjectionMat);
        cglib::ray3<double> ray(worldCGPos0, worldCGPos1 - worldCGPos0);

        // TODO: check calling sites. Decide what to do when no hit.
        double t = -1;
        if (!projectionSurface->calculateHitPoint(ray, height, t) || t < 0) {
            return cglib::vec3<double>(std::numeric_limits<double>::quiet_NaN(), std::numeric_limits<double>::quiet_NaN(), std::numeric_limits<double>::quiet_NaN());
        }
        return ray(t);
    }
    
    cglib::vec2<float> ViewState::worldToScreen(const cglib::vec3<double>& worldPos, std::shared_ptr<Options> options) const {
        if (_width <= 0 || _height <= 0) {
            Log::Error("ViewState::worldToScreen: Failed to transform point from world to screen space, screen size is unknown");
            return cglib::vec2<float>(0, 0);
        }
        
        cglib::mat4x4<double> modelviewProjectionMat = _modelviewProjectionMat;
        if (options) {
            modelviewProjectionMat = calculateModelViewMat(*options);
        }

        // Transfrom world pos to screen
        cglib::vec3<float> screenCGPos = cglib::vec3<float>::convert(cglib::transform_point(worldPos, modelviewProjectionMat));
        return cglib::vec2<float>((screenCGPos(0) + 1) * 0.5f * _width, (1 - screenCGPos(1)) * 0.5f * _height);
    }
    
    int ViewState::getHorizontalLayerOffsetDir() const {
        return _horizontalLayerOffsetDir;
    }
    
    void ViewState::setHorizontalLayerOffsetDir(int horizontalLayerOffsetDir) {
        _horizontalLayerOffsetDir = horizontalLayerOffsetDir;
    }

    void ViewState::calculateViewDistances(const Options& options, float& near, float& far, bool& skyVisible) const {
        float halfFOVY = options.getFieldOfViewY() * 0.5f;
        float tanHalfFOVY = std::tan(static_cast<float>(halfFOVY * Const::DEG_TO_RAD));
        float zoom0Distance = _height * Const::HALF_WORLD_SIZE / (_tileDrawSize * tanHalfFOVY * (_dpi / Const::UNSCALED_DPI));
        float initialZ = std::pow(2.0f, -_zoom) * zoom0Distance / 64.0f;
        cglib::vec3<double> zProjVector = cglib::unit(_focusPos - _cameraPos);

        cglib::mat4x4<double> projMat = calculatePerspMat(halfFOVY, initialZ, 2.0f * initialZ, options);
        cglib::mat4x4<double> modelviewMat = calculateLookatMat();
        cglib::mat4x4<double> invModelviewProjMat = cglib::inverse(projMat * modelviewMat);

        double heightMin = Const::MIN_HEIGHT;
        double heightMax = Const::MAX_HEIGHT;

        near = static_cast<float>(cglib::dot_product(options.getProjectionSurface()->calculateNearestPoint(_cameraPos, heightMax) - _cameraPos, zProjVector));
        far  = near;
        skyVisible = false;
        for (double xx : { -1, 0, 1 }) {
            for (double yy : { -1, 0, 1 }) {
                double x0 = 0, y0 = 0, x1 = xx, y1 = yy;
                for (int iter = -1; iter < 16; iter++) {
                    double x = (iter < 0 ? x1 : (x0 + x1) * 0.5), y = (iter < 0 ? y1 : (y0 + y1) * 0.5);
                    cglib::vec3<double> pos0 = cglib::transform_point(cglib::vec3<double>(x, y, -1), invModelviewProjMat);
                    cglib::vec3<double> pos1 = cglib::transform_point(cglib::vec3<double>(x, y,  1), invModelviewProjMat);
                    cglib::ray3<double> ray(pos0, pos1 - pos0);

                    double t = -1;
                    if (options.getProjectionSurface()->calculateHitPoint(ray, heightMin, t) && t > 0) {
                        float z = static_cast<float>(cglib::dot_product(ray(t) - pos0, zProjVector));
                        near = std::min(near, z);
                        far  = std::max(far,  z);

                        if (iter < 0) {
                            break;
                        }
                        
                        x0 = x; y0 = y;
                    } else {
                        skyVisible = true;
                        
                        x1 = x; y1 = y;
                    }
                }
            }
        }

        near = std::max(Const::MIN_NEAR, near) * 0.8f;
        far  = std::max(Const::MIN_NEAR, std::min(std::pow(2.0f, -_zoom) * zoom0Distance * options.getDrawDistance(), far)) * 1.1f;
    }
    
    float ViewState::calculateMinZoom(const Options& options) const {
        if (!options.isRestrictedPanning() || _width <= 0 || _height <= 0) {
            return options.getZoomRange().getMin();
        }

        std::shared_ptr<Projection> baseProjection = options.getBaseProjection();
        std::shared_ptr<ProjectionSurface> projectionSurface = options.getProjectionSurface();

        MapBounds boundsBase = options.getPanBounds();
        MapPos boundsPoses[2] = { baseProjection->toInternal(boundsBase.getMin()), baseProjection->toInternal(boundsBase.getMax()) };

        MapRange range = options.getZoomRange();
        for (int i = 0; i < 24; i++) {
            MapPos centerPos = boundsPoses[0] + (boundsPoses[1] - boundsPoses[0]) * 0.5;
            cglib::vec3<double> cameraVec = getCameraPos() - getFocusPos();
            ViewState viewState;
            viewState._ignoreMinZoom = true;
            viewState.setFocusPos(projectionSurface->calculatePosition(centerPos));
            viewState.setCameraPos(projectionSurface->calculatePosition(centerPos) + projectionSurface->calculateNormal(centerPos) * cglib::length(cameraVec));
            viewState.setUpVec(projectionSurface->calculateVector(centerPos, MapVec(0, 1, 0)));
            viewState.setZoom(range.getMidrange());
            viewState.setScreenSize(_height, _height); // TODO: test this for regressions (NOT _width, _height)
            viewState.cameraChanged();
            viewState.calculateViewState(options);

            bool fit = true;
            for (int j = 0; j < 4; j++) {
                MapPos mapPos(boundsPoses[j % 2].getX(), boundsPoses[j / 2].getY());
                if (viewState.getFrustum().inside(projectionSurface->calculatePosition(mapPos))) {
                    fit = false;
                    break;
                }
            }

            if (fit) {
                range.setMax(viewState.getZoom());
            } else {
                range.setMin(viewState.getZoom());
            }
        }

        return range.getMin();
    }
    
    cglib::mat4x4<double> ViewState::calculatePerspMat(float halfFOVY, float near, float far, const Options& options) const {
        double tanHalfFOVY = std::tan(halfFOVY * Const::DEG_TO_RAD);
        double top = near * tanHalfFOVY;
        double bottom = -top;
        double left = bottom * _aspectRatio;
        double right = top * _aspectRatio;

        double dx =  2 * near * tanHalfFOVY * options.getFocusPointOffset().getX() / _height;
        double dy = -2 * near * tanHalfFOVY * options.getFocusPointOffset().getY() / _height;
        
        top += dy;
        bottom += dy;
        left += dx;
        right += dx;
        
        return cglib::frustum4_matrix(left, right, bottom, top, static_cast<double>(near), static_cast<double>(far));
    }
    
    cglib::mat4x4<double> ViewState::calculateLookatMat() const {
        return cglib::lookat4_matrix(_cameraPos, _focusPos, _upVec);
    }
    
    cglib::mat4x4<double> ViewState::calculateModelViewMat(const carto::Options& options) const {
        if (_cameraChanged) {
            // Camera has changed, but the matrices have not been updated yet from the render thread. Calculate far and near distances
            float near = 0;
            float far = 0;
            bool skyVisible = false;
            calculateViewDistances(options, near, far, skyVisible);
            
            // Matrices
            cglib::mat4x4<double> projectionMat = calculatePerspMat(options.getFieldOfViewY() * 0.5f, near, far, options);
            cglib::mat4x4<double> modelviewMat = calculateLookatMat();
            return projectionMat * modelviewMat;
        }
        
        // Matrices are up to date, no need to calculate a new one
        return _modelviewProjectionMat;
    }
}

