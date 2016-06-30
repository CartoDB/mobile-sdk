/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_VIEWSTATE_H_
#define _CARTO_VIEWSTATE_H_

#include "core/MapPos.h"
#include "core/ScreenPos.h"
#include "core/MapVec.h"
#include "components/Options.h"
#include "graphics/Frustum.h"

#include <cglib/mat.h>

namespace carto {
    class Projection;
        
    /**
     * A class containing various view parameters for a view state.
     */
    class ViewState {
    public:
        /** 
         * A container class for a partial rotation matrix for a view state.
         */
        struct RotationState {
            float _cosZ;
            float _sinZ;
            float _m11;
            float _m12;
            float _m21;
            float _m22;
            float _m31;
            float _m32;
        };
    
        /**
         * Constructs an uninitialized ViewState object.
         */
        ViewState();
        virtual ~ViewState();
    
        /**
         * Returns the camera position, allows for modifications. Changing the camera position doesn't 
         * automatically update the view. To update the view cameraChanged() must be called.
         * @return The modifiable camera position.
         */
        MapPos& getCameraPos();
        /**
         * Returns the camera position.
         * @return The camera position.
         */
        const MapPos& getCameraPos() const;
        /**
         * Returns the focus position, allows for modifications. Changing the focus position doesn't
         * automatically update the view. To update the view cameraChanged() must be called.
         * @return The modifiable focus position.
         */
        MapPos& getFocusPos();
        /**
         * Returns the focus position.
         * @return The focus position.
         */
        const MapPos& getFocusPos() const;
        /**
         * Returns the up direction vector, allows for modifications. Changing the up direction vector doesn't
         * automatically update the view. To update the view cameraChanged() must be called.
         * @return The modifiable up direction vector.
         */
        MapVec& getUpVec();
        /**
         * Returns the up direction vector.
         * @return The up direction vector.
         */
        const MapVec& getUpVec() const;
    
        /**
         * Returns the state of the camera changed flag.
         * @return True if camera has changed since the last frame.
         */
        bool isCameraChanged() const;
        /**
         * Sets the state of the camera changed flag to true. This means the view will get
         * updated at the beginning of the next frame.
         */
        void cameraChanged();
        
        /**
         * Returns the camera rotation angle.
         * @return The camera rotation angle in degrees.
         */
        float getRotation() const;
        /**
         * Sets the camera rotation angle. Changing the rotation angle doesn't
         * automatically update the view. To update the view cameraChanged() must be called.
         * @param rotation The new camera rotation angle in degrees.
         */
        void setRotation(float rotation);
        /**
         * Returns the camera tilt angle.
         * @return The camera tilt angle in degrees.
         */
        float getTilt() const;
        /**
         * Sets the camera tilt angle. Changing the tilt angle doesn't
         * automatically update the view. To update the view cameraChanged() must be called.
         * @param tilt The new camera tilt angle in degrees.
         */
        void setTilt(float tilt);
        /**
         * Returns the camera zoom level.
         * @return The camera zoom level.
         */
        float getZoom() const;
        /**
         * Sets the camera zoom level. Changing the zoom level doesn't
         * automatically update the view. To update the view cameraChanged() must be called.
         * @param zoom The new camera zoom level.
         */
        void setZoom(float zoom);
        /**
         * Returns the number 2 lifted to the power of the zoom level: pow(2, zoom level).
         * @return pow(2, zoomLevel).
         */
        float get2PowZoom() const;
        /**
         * Returns the distance between the focus and the camera position, when the zoom level is set to 0.
         * This parameter depends on the screen size, DPI, tile draw size and field of view settings.
         * @return The distance between focus to camera position, when zoom level = 0.
         */
        float getZoom0Distance() const;
        
        /**
         * Get normalized resolution of the view. This is an internal parameter used by vector tile renderer.
         * @return The screen resolution of the view.
         */
        float getNormalizedResolution() const;
    
        /**
         * Returns the width of the map screen.
         * @return The map screen width.
         */
        int getWidth() const;
        /**
         * Returns the height of the map screen.
         * @return The map screen height.
         */
        int getHeight() const;
        /**
         * Returns the width of the map screen divided by 2.
         * @return The map screen width divided by 2.
         */
        float getHalfWidth() const;
        /**
         * Returns the height of the map screen divided by 2.
         * @return The map screen height divided by 2.
         */
        float getHalfHeight() const;
        /**
         * Returns the aspect ratio of the map screen. Equal to width / height.
         * @return The aspect ratio of the map screen.
         */
        float getAspectRatio() const;
    
        /**
         * Returns the near plane distance.
         * @return The new plane distance.
         */
        float getNear() const;
        /**
         * Returns the far plane distance.
         * @return The far plane distance.
         */
        float getFar() const;
    
        /**
         * Returns the vertical field of view angle.
         * @return The vertical field of view angle in degrees.
         */
        int getFOVY() const;
        /**
         * Returns the vertical field of view angle, divided by 2.
         * @return The vertical field of view angle in degrees, divided by 2.
         */
        float getHalfFOVY() const;
        /**
         * Returns the tangent of the half vertical FOV angle.
         * @return The tangent of the half vertical FOV angle.
         */
        double getTanHalfFOVY() const;
        /**
         * Returns the cosine of the half vertical FOV angle.
         * @return The cosine of the half vertical FOV angle.
         */
        double getCosHalfFOVY() const;
        /**
         * Returns the tangent of the half horizontal FOV angle.
         * @return The tangent of the half horizontal FOV angle.
         */
        double getTanHalfFOVX() const;
        /**
         * Returns the cosine of the half horizontal FOV angle multiplied 
         * by the cosine of the half vertical FOV angle.
         * @return The cosine of the half horizontal FOV angle multiplied
         * by the cosine of the half vertical FOV angle.
         */
        double getCosHalfFOVXY() const;
        
        /**
         * Returns a value that is used for converting display independent pixels (dp) to pixels (px).
         * This values depends on the screen density.
         * @return The dp to px value.
         */
        float getDPToPX() const;
        /**
         * Returns the dots per inch parameter of the screen.
         * @return The screen DPI parameter.
         */
        float getDPI() const;
    
        /**
         * Returns the conversion ratio between internal map units and screen pixels. This parameter is dependent
         * on the zoom level and other screen parameters.
         * @return The conversion ratio between internal map units and screen pixels.
         */
        float getUnitToPXCoef() const;
        /**
         * Returns the conversion ratio between internal map units and screen density independent pixels (DP or DIP). 
         * This parameter is dependent on the zoom level, DPI and other screen parameters.
         * @return The conversion ratio between internal map units and screen density independent pixels.
         */
        float getUnitToDPCoef() const;
        /**
         * Returns the partial rotation matrix representing this view state.
         * @return The partial rotation matrix representing this view state.
         */
        const RotationState& getRotationState() const;
    
        /** 
         * Returns the projection mode.
         * @return The projection mode.
         */
        ProjectionMode::ProjectionMode getProjectionMode() const;
    
        /**
         * Returns the projection matrix.
         * @return The projection matrix.
         */
        const cglib::mat4x4<double>& getProjectionMat() const;
        /**
         * Returns the modelview matrix.
         * @return The modelview matrix.
         */
        const cglib::mat4x4<double>& getModelviewMat() const;
        /**
         * Returns the modelview-projection matrix. This is equivalent to the the projeciton matrix multiplied with the
         * modelview matrix.
         * @return The modelview-projection matrix.
         */
        const cglib::mat4x4<double>& getModelviewProjectionMat() const;
        /**
         * Returns the relative-to-eye modelview matrix. This equivalent to the modelview matrix with the first
         * three elements of the last column set to 0.
         * @return The relative-to-eye modelview matrix.
         */
        const cglib::mat4x4<float>& getRTEModelviewMat() const;
        /**
         * Returns the relative-to-eye modelview-projection matrix. This is equivalent to the projection matrix 
         * multiplied with the relative-to-eye modelview matrix.
         * @return The relative-to-eye modelview-projection matrix.
         */
        const cglib::mat4x4<float>& getRTEModelviewProjectionMat() const;
        
        /**
         * Returns the view frustum.
         * @return The view frustum.
         */
        const Frustum& getFrustum() const;
    
        // TODO: get rid of them at later stage
        static cglib::mat4x4<double> GetLocalMat(const MapPos& mapPos, const Projection& proj);
        cglib::mat4x4<float> getRTELocalMat(const MapPos &mapPos, const Projection &proj) const;
    
        /**
         * Returns the screen width.
         * @return The current screen width.
         */
        int getScreenWidth() const;
        /**
         * Returns the screen height.
         * @return The current screen height.
         */
        int getScreenHeight() const;
        /**
         * Sets the screen size. The view will be updated at the beginning of the next frame.
         * @param width The new width of the screen.
         * @param height The new height of the screen.
         */
        void setScreenSize(int width, int height);
        /**
         * Updates the view when the screen size, view state or some other view options have changed. This is automatically
         * called at the beginning of every frame.
         * @param options The state of options.
         */
        void calculateViewState(const Options& options);
    
        /**
         * Projects a screen position into the map plane using the current view state.
         * @param screenPos The screen position.
         * @param options The options object.
         * @return The screen position projected into the map plane, in internal coordinates.
         */
        MapPos screenToWorldPlane(const ScreenPos& screenPos, std::shared_ptr<Options> options = std::shared_ptr<Options>()) const;
        /**
         * Projects a world position onto the screen using the current view state.
         * @param worldPos The world position.
         * @param options The options object.
         * @return The world position projected onto the screen, in pixel coordinates.
         */
        ScreenPos worldToScreen(const MapPos& worldPos, const Options& options) const;
    
        /**
         * Returns the horizontal offset direction, caused by seamless panning horizontally over the map borders.
         * @return The horizontal offset direction.
         */
        int getHorizontalLayerOffsetDir() const;
        /**
         * Sets the horizontal offset direction. This method is called when seamless panning is enabled and
         * the user pans horizontally over the map borders.
         * @param horizontalLayerOffsetDir The horizontal offset direction, -1 for left, 0 for none, 1 for right.
         */
        void setHorizontalLayerOffsetDir(int horizontalLayerOffsetDir);
    
    private:
        float calculateNearPlanePersp(const MapPos& cameraPos, float tilt, float halfFOVY) const;
        float calculateFarPlanePersp(const MapPos& cameraPos, float tilt, float halfFOVY, const Options& options) const;
        cglib::mat4x4<double> calculatePerspMat(float halfFOVY, float near, float far, const Options& options) const;
        cglib::mat4x4<double> calculateLookatMat() const;
        cglib::mat4x4<double> calculateModelViewMat(const Options& options) const;
    
        static const int DEFAULT_CAMERA_POS_X = 0;
        static const int DEFAULT_CAMERA_POS_Y = 0;
        static const int DEFAULT_CAMERA_POS_Z = 10;
        static const int DEFAULT_FOCUS_POS_X = 0;
        static const int DEFAULT_FOCUS_POS_Y = 0;
        static const int DEFAULT_FOCUS_POS_Z = 0;
        static const int DEFAULT_UP_VEC_X = 0;
        static const int DEFAULT_UP_VEC_Y = 1;
        static const int DEFAULT_UP_VEC_Z = 0;
        static const int DEFAULT_ROTATION = 0;
        static const int DEFAULT_TILT = 90;
    
        MapPos _cameraPos;
        MapPos _focusPos;
        MapVec _upVec;
        bool _cameraChanged;
    
        float _rotation;
        float _tilt;
        float _zoom;
        float _2PowZoom;
        float _zoom0Distance;
        float _normalizedResolution;
    
        int _width;
        int _height;
        float _halfWidth;
        float _halfHeight;
        float _aspectRatio;
        bool _screenSizeChanged;
    
        float _near;
        float _far;
    
        int _fovY;
        float _halfFOVY;
        double _tanHalfFOVY;
        double _cosHalfFOVY;
        double _tanHalfFOVX;
        double _cosHalfFOVXY;
    
        int _tileDrawSize;
        
        float _dpToPX;
        float _dpi;
    
        float _unitToPXCoef;
        float _unitToDPCoef;
        RotationState _rotationState;
    
        ProjectionMode::ProjectionMode _projectionMode;
    
        cglib::mat4x4<double> _projectionMat;
        cglib::mat4x4<double> _modelviewMat;
        cglib::mat4x4<double> _modelviewProjectionMat;
        cglib::mat4x4<float> _rteModelviewMat;
        cglib::mat4x4<float> _rteModelviewProjectionMat;
        
        Frustum _frustum;
    
        int _horizontalLayerOffsetDir;
    };
    
}

#endif
