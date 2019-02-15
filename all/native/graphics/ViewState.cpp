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
        _projectionMode(ProjectionMode::PROJECTION_MODE_PERSPECTIVE),
        _projectionSurface(),
        _projectionMat(),
        _modelviewMat(),
        _modelviewProjectionMat(),
        _rteModelviewMat(),
        _rteModelviewProjectionMat(),
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
    
    ProjectionMode::ProjectionMode ViewState::getProjectionMode() const {
        return _projectionMode;
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

        // TODO: fix the implementation: tesselate pan bounds, check against all tesselated points. Use 2 triangles?
        /*
        std::shared_ptr<Projection> baseProjection = options.getBaseProjection();
        std::shared_ptr<ProjectionSurface> projectionSurface = options.getProjectionSurface();
        MapBounds bounds = options.getPanBounds();
        MapPos boundsPoses[2] = { bounds.getMin(), bounds.getMax() };

        ScreenBounds screenBounds(ScreenPos(_width * 0.5f - _height * 0.5f, 0.0f),
                                  ScreenPos(_width * 0.5f + _height * 0.5f, _height));

        cglib::vec3<double> cameraVec = _cameraPos - _focusPos;
        for (int j = 0; j < 4; j++) {
            if (options.isSeamlessPanning()) {
                if (j % 2 == 0) {
                    continue; // ignore X-based check if map is repeating along X
                }
            }

            MapPos edgePos = baseProjection->fromInternal(projectionSurface->calculateMapPos(_focusPos));
            MapPos centerPos = edgePos;
            edgePos[j % 2] = boundsPoses[j / 2][j % 2];
            centerPos[j % 2] = bounds.getCenter()[j % 2];

            ViewState viewState;
            viewState._ignoreMinZoom = true;
            viewState._minZoom = _minZoom;
            viewState.setFocusPos(_focusPos);
            viewState.setCameraPos(_focusPos + projectionSurface->calculateNormal(_focusPos) * cglib::length(cameraVec));
            viewState.setUpVec(projectionSurface->calculateNormal(_focusPos));
            viewState.setZoom(_zoom);
            viewState.setScreenSize(_width, _height);
            viewState.cameraChanged();
            viewState.calculateViewState(options);
            viewState.clampZoom(options);
            ScreenPos screenPos = viewState.worldToScreen(projectionSurface->calculatePosition(baseProjection->toInternal(edgePos)), options);
            if (!screenBounds.contains(screenPos)) {
                continue;
            }

            MapPos focusPos0 = centerPos, focusPos1 = edgePos;
            for (int i = 0; i < 24; i++) {
                MapPos focusPos = focusPos0 + (focusPos1 - focusPos0) * 0.5;
                viewState.setFocusPos(baseProjection->toInternal(focusPos));
                viewState.setCameraPos(viewState.getFocusPos() + projectionSurface->calculateNormal(viewState.getFocusPos()) * cglib::length(cameraVec));
                viewState.setUpVec(projectionSurface->calculateNormal(viewState.getFocusPos()));
                viewState.cameraChanged();
                viewState.calculateViewState(options);

                ScreenPos screenPos = viewState.worldToScreen(baseProjection->toInternal(edgePos), options);
                if (!screenBounds.contains(screenPos)) {
                    focusPos0 = focusPos;
                } else {
                    focusPos1 = focusPos;
                }
            }

            if (baseProjection->toInternal(focusPos0) != getFocusPos()) {
                setFocusPos(baseProjection->toInternal(focusPos0));
                setCameraPos(baseProjection->toInternal(focusPos0) + cameraVec);

                cameraChanged();
            }
        }
        */
    }
    
    void ViewState::calculateViewState(const Options& options) {
        switch (options.getProjectionMode()) {
            // TODO: implement?
            case ProjectionMode::PROJECTION_MODE_ORTHOGONAL: {
                _projectionMode = ProjectionMode::PROJECTION_MODE_ORTHOGONAL;
                break;
            }
            case ProjectionMode::PROJECTION_MODE_PERSPECTIVE:
            default: {
                _projectionMode = ProjectionMode::PROJECTION_MODE_PERSPECTIVE;
        
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
        
                    std::pair<float, float> nearFarPlanes = calculateNearFarPlanes(options);
                    _near = nearFarPlanes.first;
                    _far = nearFarPlanes.second;
        
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
                }
                break;
            }
        }
    }
    
    cglib::vec3<double> ViewState::screenToWorldPlane(const cglib::vec2<float>& screenPos, std::shared_ptr<Options> options) const {
        if (_width <= 0 || _height <= 0) {
            Log::Error("ViewState::screenToWorldPlane: Failed to transform point from screen space to world plane, screen size is unknown");
            return cglib::vec3<double>(0, 0, 0);
        }

        std::shared_ptr<ProjectionSurface> projectionSurface = _projectionSurface;
        cglib::mat4x4<double> modelviewProjectionMat = _modelviewProjectionMat;
        if (options) {
            projectionSurface = options->getProjectionSurface();
            modelviewProjectionMat = calculateModelViewMat(*options);
        }
        if (!projectionSurface) {
            return cglib::vec3<double>(0, 0, 0); // TODO: better impl
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
        if (!projectionSurface->calculateHitPoint(ray, 0, t) || t < 0) {
            return cglib::vec3<double>(0, 0, 0); // TODO: better impl
        }
        return ray(t);
    }
    
    cglib::vec2<float> ViewState::worldToScreen(const cglib::vec3<double>& worldPos, const Options& options) const {
        if (_width <= 0 || _height <= 0) {
            Log::Error("ViewState::worldToScreen: Failed to transform point from world to screen space, screen size is unknown");
            return cglib::vec2<float>(0, 0);
        }
        
        cglib::mat4x4<double> modelviewProjectionMat = calculateModelViewMat(options);

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

    std::pair<float, float> ViewState::calculateNearFarPlanes(const Options& options) const {
        float halfFOVY = options.getFieldOfViewY() * 0.5f;
        float tanHalfFOVY = std::tan(static_cast<float>(halfFOVY * Const::DEG_TO_RAD));
        float zoom0Distance = _height * Const::HALF_WORLD_SIZE / (_tileDrawSize * tanHalfFOVY * (_dpi / Const::UNSCALED_DPI));
        float initialZ = std::pow(2.0f, -_zoom) * zoom0Distance / 64.0f;

        cglib::mat4x4<double> projMat = calculatePerspMat(halfFOVY, initialZ, 2.0f * initialZ, options);
        cglib::mat4x4<double> modelviewMat = calculateLookatMat();
        cglib::mat4x4<double> invModelviewProjMat = cglib::inverse(projMat * modelviewMat);

        cglib::vec3<double> pos0Origin = cglib::transform_point(cglib::vec3<double>(0, 0, -1), invModelviewProjMat);
        cglib::vec3<double> pos1Origin = cglib::transform_point(cglib::vec3<double>(0, 0,  1), invModelviewProjMat);
        cglib::vec3<double> zProjVector = cglib::unit(pos1Origin - pos0Origin);

        double heightMin = 0;
        double heightMax = Const::MAX_HEIGHT / Const::EARTH_CIRCUMFERENCE;

        double zMax = 0;
        double zMin = cglib::dot_product(options.getProjectionSurface()->calculateNearestPoint(pos0Origin, heightMax) - pos0Origin, zProjVector);
        if (zMin < 0) {
            zMin = cglib::dot_product(options.getProjectionSurface()->calculateNearestPoint(pos0Origin, heightMin) - pos0Origin, zProjVector);
        }

        for (double x : { -1, 1 }) {
            for (double y : { -1, 1 }) {
                cglib::vec3<double> pos0 = cglib::transform_point(cglib::vec3<double>(x, y, -1), invModelviewProjMat);
                cglib::vec3<double> pos1 = cglib::transform_point(cglib::vec3<double>(x, y,  1), invModelviewProjMat);
                cglib::ray3<double> ray(pos0, pos1 - pos0);

                double z0 = std::pow(2.0f, -_zoom) * zoom0Distance * options.getDrawDistance();
                double t0 = -1;
                cglib::vec3<double> nearestPos0 = options.getProjectionSurface()->calculateNearestPoint(ray, heightMin, t0);
                if (t0 >= 0) {
                    z0 = cglib::dot_product(nearestPos0 - pos0, zProjVector);
                }

                double z1 = z0 * 0.5f;
                double t1 = -1;
                cglib::vec3<double> nearestPos1 = options.getProjectionSurface()->calculateNearestPoint(ray, heightMax, t1);
                if (t1 >= 0) {
                    z1 = std::min(z1, cglib::dot_product(nearestPos1 - pos0, zProjVector));
                }

                zMax = std::max(zMax, z0);
                zMin = std::min(zMin, z1);
            }
        }

        // TODO: remove
        double clipNear = std::min(_cameraPos(2) * 0.9, std::max(_cameraPos(2) - Const::MAX_HEIGHT, static_cast<double>(Const::MIN_NEAR)));
        if (std::abs(90 - _tilt - halfFOVY) < 90) {
            // Put near plane to intersection between frustum and ground plane
            double cosAminusB = std::cos((90 - _tilt - halfFOVY) * Const::DEG_TO_RAD);
            double cosB = std::cos(halfFOVY * Const::DEG_TO_RAD);
            clipNear = clipNear * cosB / cosAminusB;
        }
        clipNear = std::min(clipNear, static_cast<double>(Const::MAX_NEAR));

        // TODO: remove
        // Hack: compensate focus point offset by increasing tilt
        float tilt = _tilt;
        if (options.getFocusPointOffset().getY() < 0) {
            float delta = -2 * options.getFocusPointOffset().getY() / _height;
            tilt = std::max(0.0f, tilt - static_cast<float>(std::atan2(delta, 1) * Const::RAD_TO_DEG));
        }
        double clipFar = _cameraPos(2) * options.getDrawDistance();
        if (90 - tilt + halfFOVY < 90) {
            // Put far plane to intersection between frustum and ground plane
            double cosAplusB = std::cos((90 - tilt + halfFOVY) * Const::DEG_TO_RAD);
            double cosB = std::cos(halfFOVY * Const::DEG_TO_RAD);
            double distance = _cameraPos(2) * cosB / cosAplusB;
            // Put far plane a bit further to avoid precision issues
            clipFar = std::min(clipFar, 1.1 * distance);
        }

        // TODO:
        return std::make_pair(static_cast<float>(zMin * 0.8f), static_cast<float>(zMax * 1.2f));
    }
    
    float ViewState::calculateMinZoom(const Options& options) const {
        if (!options.isRestrictedPanning() || _width <= 0 || _height <= 0) {
            return options.getZoomRange().getMin();
        }

        // TODO: fix the implementation: tesselate pan bounds, check against all tesselated points. Use 2 triangles?
        /*
        std::shared_ptr<Projection> baseProjection = options.getBaseProjection();
        std::shared_ptr<ProjectionSurface> projectionSurface = options.getProjectionSurface();
        MapBounds bounds = options.getPanBounds();

        std::vector<MapPos> boundsPoses;
        for (int i = 0; i < 4; i++) {
            double x = (i % 2 == 0 ? bounds.getMin().getX() : bounds.getMax().getX());
            double y = (i / 2 == 0 ? bounds.getMin().getY() : bounds.getMax().getY());
            boundsPoses.push_back(baseProjection->toInternal(MapPos(x, y)));
        }

        ScreenBounds screenBounds(ScreenPos(_width * 0.5f - _height * 0.5f, 0.0f),
                                  ScreenPos(_width * 0.5f + _height * 0.5f, _height));

        MapRange range = options.getZoomRange();
        for (int i = 0; i < 24; i++) {
            cglib::vec3<double> cameraVec = getCameraPos() - getFocusPos();
            ViewState viewState;
            viewState._ignoreMinZoom = true;
            viewState.setFocusPos(bounds.getCenter());
            viewState.setCameraPos(bounds.getCenter() + projectionSurface->calculateNormal(bounds.getCenter()) * cglib::length(cameraVec));
            viewState.setZoom((range.getMin() + range.getMax()) * 0.5f);
            viewState.setScreenSize(_width, _height);
            viewState.cameraChanged();
            viewState.calculateViewState(options);

            bool fit = true;
            for (const MapPos& mapPos : boundsPoses) {
                ScreenPos screenPos = viewState.worldToScreen(mapPos, options);
                if (screenBounds.contains(screenPos)) {
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
        */
        return 0;
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
            std::pair<float, float> nearFarPlanes = calculateNearFarPlanes(options);
            
            // Matrices
            cglib::mat4x4<double> projectionMat = calculatePerspMat(options.getFieldOfViewY() * 0.5f, nearFarPlanes.first, nearFarPlanes.second, options);
            cglib::mat4x4<double> modelviewMat = calculateLookatMat();
            return projectionMat * modelviewMat;
        }
        
        // Matrices are up to date, no need to calculate a new one
        return _modelviewProjectionMat;
    }
}

