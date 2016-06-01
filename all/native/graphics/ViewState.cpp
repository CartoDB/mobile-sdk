#include "ViewState.h"
#include "projections/Projection.h"
#include "utils/Const.h"
#include "utils/Log.h"

#include <cmath>
#include <cglib/mat.h>

namespace carto {

    ViewState::ViewState() :
        _cameraPos(DEFAULT_CAMERA_POS_X, DEFAULT_CAMERA_POS_Y, DEFAULT_CAMERA_POS_Z),
        _focusPos(DEFAULT_FOCUS_POS_X, DEFAULT_FOCUS_POS_Y, DEFAULT_FOCUS_POS_Z),
        _upVec(DEFAULT_UP_VEC_X, DEFAULT_UP_VEC_Y, DEFAULT_UP_VEC_Z),
        _cameraChanged(true),
        _rotation(DEFAULT_ROTATION),
        _tilt(DEFAULT_TILT),
        _zoom(0.0f),
        _2PowZoom(1.0f),
        _zoom0Distance(0.0f),
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
    
    MapPos& ViewState::getCameraPos() {
        return _cameraPos;
    }
    
    const MapPos& ViewState::getCameraPos() const {
        return _cameraPos;
    }
    
    MapPos& ViewState::getFocusPos() {
        return _focusPos;
    }
    
    const MapPos& ViewState::getFocusPos() const {
        return _focusPos;
    }
    
    MapVec& ViewState::getUpVec() {
        return _upVec;
    }
    
    const MapVec& ViewState::getUpVec() const {
        return _upVec;
    }
    
    bool ViewState::isCameraChanged() const {
        return _cameraChanged;
    }
    
    void ViewState::cameraChanged() {
        _cameraChanged = true;
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
    
    float ViewState::get2PowZoom() const {
        return _2PowZoom;
    }
    
    float ViewState::getZoom0Distance() const {
        return _zoom0Distance;
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
    
    double ViewState::getNear() const {
        return _near;
    }
    
    double ViewState::getFar() const {
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
        
    const Frustum& ViewState::getFrustum() const {
        return _frustum;
    }
    
    cglib::mat4x4<double> ViewState::GetLocalMat(const MapPos &mapPos, const Projection &proj) {
        const MapBounds &bounds = proj.getBounds();
        const MapVec& boundsDelta = bounds.getDelta();
        double scaleX = Const::WORLD_SIZE / boundsDelta.getX();
        double scaleY = Const::WORLD_SIZE / boundsDelta.getY();
        double scaleZ = std::min(scaleX, scaleY); // TODO: projection should supply this
        double localScale = proj.getLocalScale(mapPos);
        MapPos mapPosInternal = proj.toInternal(mapPos);
        cglib::mat4x4<double> localMat(cglib::mat4x4<double>::identity());
        localMat(0, 0) = scaleX * localScale;
        localMat(1, 1) = scaleY * localScale;
        localMat(2, 2) = scaleZ * localScale;
        localMat(0, 3) = mapPosInternal.getX();
        localMat(1, 3) = mapPosInternal.getY();
        localMat(2, 3) = mapPosInternal.getZ();
        return localMat;
    }
    
    cglib::mat4x4<float> ViewState::getRTELocalMat(const MapPos &mapPos, const Projection &proj) const {
        const MapBounds &bounds = proj.getBounds();
        const MapVec& boundsDelta = bounds.getDelta();
        double scaleX = Const::WORLD_SIZE / boundsDelta.getX();
        double scaleY = Const::WORLD_SIZE / boundsDelta.getY();
        double scaleZ = std::min(scaleX, scaleY); // TODO: projection should supply this
        double localScale = proj.getLocalScale(mapPos);
        MapVec delta = proj.toInternal(mapPos) - _cameraPos;
        cglib::mat4x4<float> localMat(cglib::mat4x4<float>::identity());
        localMat(0, 0) = (float) (scaleX * localScale);
        localMat(1, 1) = (float) (scaleY * localScale);
        localMat(2, 2) = (float) (scaleZ * localScale);
        localMat(0, 3) = (float) delta.getX();
        localMat(1, 3) = (float) delta.getY();
        localMat(2, 3) = (float) delta.getZ();
        return localMat;
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
    
    void ViewState::calculateViewState(const Options& options) {
    
        switch (options.getProjectionMode()) {
        case ProjectionMode::PROJECTION_MODE_ORTHOGONAL: {
            _projectionMode = ProjectionMode::PROJECTION_MODE_ORTHOGONAL;
            break;
        }
        case ProjectionMode::PROJECTION_MODE_PERSPECTIVE:
        default: {
            _projectionMode = ProjectionMode::PROJECTION_MODE_PERSPECTIVE;
    
            // If FOV or tile draw size changed, recalculate zoom0Distance
            int FOVY = options.getFieldOfViewY();
            int tileDrawSize = options.getTileDrawSize();
            float dpi = options.getDPI();
            if (FOVY != _fovY || tileDrawSize != _tileDrawSize || dpi != _dpi || _screenSizeChanged) {
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
                
                _normalizedResolution = 2 * tileDrawSize * (_dpi / Const::UNSCALED_DPI);
    
                // Calculate new camera position
                MapVec cameraVec = _cameraPos - _focusPos;
                double length = cameraVec.length();
                double newLength = _zoom0Distance / std::pow(2.0f, _zoom);
                cameraVec *= newLength / length;
                _cameraPos = _focusPos;
                _cameraPos += cameraVec;
    
                _cameraChanged = true;
            }
    
            if (_cameraChanged) {
                _cameraChanged = false;
    
                // Calculate concatenated matrix for XZ rotation (X before, then Z) for billboard vector elements
                float cosX = static_cast<float>(std::cos((_tilt - 90) * Const::DEG_TO_RAD));
                float sinX = static_cast<float>(std::sin((_tilt - 90) * Const::DEG_TO_RAD));
                _rotationState._cosZ = static_cast<float>(std::cos(-_rotation * Const::DEG_TO_RAD));
                _rotationState._sinZ = static_cast<float>(std::sin(-_rotation * Const::DEG_TO_RAD));
                _rotationState._m11 = _rotationState._cosZ;
                _rotationState._m12 = cosX * _rotationState._sinZ;
                _rotationState._m21 = -_rotationState._sinZ;
                _rotationState._m22 = cosX * _rotationState._cosZ;
                _rotationState._m31 = 0;
                _rotationState._m32 = -sinX;
    
                // Calculate scaling factor for vector elements
                _unitToPXCoef = static_cast<float>(_zoom0Distance / (_height * _tanHalfFOVY) / _2PowZoom);
                _unitToDPCoef = _unitToPXCoef * _dpi / Const::UNSCALED_DPI;
    
                _near = calculateNearPlanePersp(_cameraPos, _tilt, _halfFOVY);
                _far = calculateFarPlanePersp(_cameraPos, _tilt, _halfFOVY, options);
    
                // Matrices
                _projectionMat = calculatePerspMat(_halfFOVY, _near, _far, options);
                _modelviewMat = calculateLookatMat();
    
                // Double precision mvp matrix and frustum
                _modelviewProjectionMat = _projectionMat * _modelviewMat;
                _frustum = Frustum(_modelviewProjectionMat);
    
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
    
    MapPos ViewState::screenToWorldPlane(const ScreenPos& screenPos, std::shared_ptr<Options> options) const {
        if (_width <= 0 || _height <= 0) {
            Log::Error("ViewState::screenToWorldPlane: Failed to transform point from screen space to world plane, screen size is unknown");
            return MapPos();
        }

        cglib::mat4x4<double> modelviewProjectionMat = options ? calculateModelViewMat(*options) : _modelviewProjectionMat;
        cglib::mat4x4<double> invModelviewProjectionMat = cglib::inverse(modelviewProjectionMat);

        // Transform 2 points with different z values from world to screen
        cglib::vec3<double> screenCGPos0(screenPos.getX() / _width * 2 - 1, 1 - screenPos.getY() / _height * 2, -1);
        cglib::vec3<double> screenCGPos1(screenPos.getX() / _width * 2 - 1, 1 - screenPos.getY() / _height * 2,  1);
        cglib::vec3<double> worldCGPos0 = cglib::transform_point(screenCGPos0, invModelviewProjectionMat);
        cglib::vec3<double> worldCGPos1 = cglib::transform_point(screenCGPos1, invModelviewProjectionMat);
        cglib::vec3<double> worldCGDir = worldCGPos1 - worldCGPos0;

        // Calculate ray intersection with z=0 plane
        double z = 0;
        double t = (z - worldCGPos0(2)) / worldCGDir(2);
        cglib::vec3<double> hitPos = worldCGPos0 + worldCGDir * t;
        return MapPos(hitPos(0), hitPos(1), hitPos(2));
    }
    
    ScreenPos ViewState::worldToScreen(const MapPos& worldPos, const Options& options) const {
        if (_width <= 0 || _height <= 0) {
            Log::Error("ViewState::worldToScreen: Failed to transform point from world to screen space, screen size is unknown");
            return ScreenPos();
        }
        
        cglib::mat4x4<double> modelviewProjectionMat = calculateModelViewMat(options);

        // Transfrom world pos to screen
        const cglib::vec3<double>& worldCGPos = cglib::vec3<double>(worldPos.getX(), worldPos.getY(), worldPos.getZ());
        const cglib::vec3<double>& screenCGPos = cglib::transform_point(worldCGPos, modelviewProjectionMat);
        ScreenPos screenPos(static_cast<float>(screenCGPos(0)), static_cast<float>(screenCGPos(1)));
        screenPos.setX((screenPos.getX() + 1) * 0.5f * _width);
        screenPos.setY((screenPos.getY() - 1) * -0.5f * _height);
        return screenPos;
    }
    
    int ViewState::getHorizontalLayerOffsetDir() const {
        return _horizontalLayerOffsetDir;
    }
    
    void ViewState::setHorizontalLayerOffsetDir(int horizontalLayerOffsetDir) {
        _horizontalLayerOffsetDir = horizontalLayerOffsetDir;
    }
    
    float ViewState::calculateNearPlanePersp(const MapPos& cameraPos, float tilt, float halfFOVY) const {
        double clipNear = std::min(cameraPos.getZ() * 0.9, std::max(cameraPos.getZ() - Const::MAX_HEIGHT, static_cast<double>(Const::MIN_NEAR)));
        if (std::abs(90 - tilt - halfFOVY) < 90) {
            // Put near plane to intersection between frustum and ground plane
            double cosAminusB = std::cos((90 - tilt - halfFOVY) * Const::DEG_TO_RAD);
            double cosB = std::cos(halfFOVY * Const::DEG_TO_RAD);
            clipNear = clipNear * cosB / cosAminusB;
        }
        clipNear = std::min(clipNear, static_cast<double>(Const::MAX_NEAR));
        return static_cast<float>(clipNear);
    }
    
    float ViewState::calculateFarPlanePersp(const MapPos& cameraPos, float tilt, float halfFOVY, const Options& options) const {
        // Hack: compensate focus point offset by increasing tilt
        if (options.getFocusPointOffset().getY() < 0) {
            float delta = -2 * options.getFocusPointOffset().getY() / _height;
            tilt = std::max(0.0f, tilt - static_cast<float>(std::atan2(delta, 1) * Const::RAD_TO_DEG));
        }
        double clipFar = cameraPos.getZ() * options.getDrawDistance();
        if (90 - tilt + halfFOVY < 90) {
            // Put far plane to intersection between frustum and ground plane
            double cosAplusB = std::cos((90 - tilt + halfFOVY) * Const::DEG_TO_RAD);
            double cosB = std::cos(halfFOVY * Const::DEG_TO_RAD);
            double distance = cameraPos.getZ() * cosB / cosAplusB;
            // Put far plane a bit further to avoid precision issues
            clipFar = std::min(clipFar, 1.1 * distance);
        }
        return static_cast<float>(clipFar);
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
        return cglib::lookat4_matrix(
            cglib::vec3<double>(_cameraPos.getX(), _cameraPos.getY(), _cameraPos.getZ()),
            cglib::vec3<double>(_focusPos.getX(), _focusPos.getY(), _focusPos.getZ()),
            cglib::vec3<double>(_upVec.getX(), _upVec.getY(), _upVec.getZ()));
    }
    
    cglib::mat4x4<double> ViewState::calculateModelViewMat(const carto::Options &options) const {
        cglib::mat4x4<double> modelviewProjectionMat;
        if (_cameraChanged) {
            // Camera has changed, but the matrices have not been updated yet from the render thread
            
            // Calculate far and near distances
            float fovY = options.getFieldOfViewY();
            float halfFOVY = fovY * 0.5f;
            float near = calculateNearPlanePersp(_cameraPos, _tilt, halfFOVY);
            float far = calculateFarPlanePersp(_cameraPos, _tilt, halfFOVY, options);
            
            // Matrices
            cglib::mat4x4<double> projectionMat = calculatePerspMat(halfFOVY, near, far, options);
            cglib::mat4x4<double> modelviewMat = calculateLookatMat();
            return projectionMat * modelviewMat;
        }
        
        // Matrices are up to date, no need to calculate a new one
        return _modelviewProjectionMat;
    }
}

