/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_BASEMAPVIEW_H_
#define _CARTO_BASEMAPVIEW_H_

#include <memory>
#include <mutex>
#include <thread>

namespace carto {
    class CancelableThreadPool;
    class Layers;
    class MapBounds;
    class MapPos;
    class MapVec;
    class MapRenderer;
    class ScreenBounds;
    class ScreenPos;
    class Options;
    class LicenseManagerListener;
    class MapEventListener;
    class RedrawRequestListener;
    class TouchHandler;
    
    /**
     * A platform independent main view class for all mapping operations.
     * Allows the user to manipulate the map and access various related components.
     */
    class BaseMapView {
    public:
        /**
         * Registers the SDK license. This class method and must be called before
         * creating any actual MapView instances.
         * @param licenseKey The license string provided for this application.
         * @param listener The listener that receives notifications when the license has been updated.
         * @return True if license is valid, false if not.
         */
        static bool RegisterLicense(const std::string& licenseKey, const std::shared_ptr<LicenseManagerListener>& listener);

        /**
         * Returns the SDK version and build info. The result should be used only for reporting purposes.
         * @return The SDK version and build info.
         */
        static std::string GetSDKVersion();
        
        BaseMapView();
        virtual ~BaseMapView();
    
        /**
         * Prepares renderers for drawing. Has to be called again if the graphics context was lost.
         */
        void onSurfaceCreated();
        /**
         * Changes the screen size of the map view. Calling this method before
         * onSurfaceCreated is called results in undefined behaviour.
         * @param width The new width of the map view.
         * @param height The new height of the map view.
         */
        void onSurfaceChanged(int width, int height);
        /**
         * Draws a single frame to the current graphics context. Calling this method before
         * onSurfaceCreated and onSurfaceChanged are called results in undefined behaviour.
         */
        void onDrawFrame();
        /**
         * Stops renderer. Rendering may resume only after onSurfaceCreated is called again.
         */
        void onSurfaceDestroyed();
        /**
         * Handles a user input event.
         * @param event The event type. First pointer down = 0, second pointer down = 1, either pointer moved = 2, 
                        gesture canceled = 3, first pointer up = 4, second pointer up = 5.
         * @param x1 The x coordinate of the first pointer. -1 if there are no coordinates.
         * @param y1 The y coordinate of the first pointer. -1 if there are no coordinates.
         * @param x2 The x coordinate of the second pointer. -1 if there are no coordinates.
         * @param y2 The y coordinate of the second pointer. -1 if there are no coordinates.
         */
        void onInputEvent(int event, float x1, float y1, float x2, float y2);
        
        /**
         * Returns the Layers object, that can be used for adding and removing map layers.
         * @return The Layer object.
         */
        const std::shared_ptr<Layers>& getLayers() const;
        /**
         * Returns the Options object, that can be used for modifying various map options.
         * @return the Option object.
         */
        const std::shared_ptr<Options>& getOptions() const;
        /**
         * Returns the MapRenderer object, that can be used for controlling rendering options.
         * @return the MapRenderer object.
         */
        const std::shared_ptr<MapRenderer>& getMapRenderer() const;
    
        /**
         * Returns the position that the camera is currently looking at.
         * @return The current focus position in the coordinate system of the base projection.
         */
        MapPos getFocusPos() const;
        /**
         * Returns the map rotation in degrees. 0 means looking north, 90 means west, -90 means east and 180 means south.
         * @return The map rotation in degrees in range of (-180 .. 180].
         */
        float getRotation() const;
        
        /**
         * Returns the tilt angle in degrees. 0 means looking directly at the horizon, 90 means looking directly down.
         * @return The tilt angle in degrees.
         */
        float getTilt() const;
        /**
         * Returns the zoom level. The value returned is never negative, 0 means absolutely zoomed out and all other
         * values describe some level of zoom.
         * @return The zoom level.
         */
        float getZoom() const;
    
        /**
         * Pans the view relative to the current focus position. The deltaPos vector is expected to be in 
         * the coordinate system of the base projection. The new calculated focus position will be clamped to
         * the world bounds and to the bounds set by Options::setPanBounds.
         *
         * If durationSeconds > 0 the panning operation will be animated over time. If the previous panning animation has not
         * finished by the time this method is called, it will be stopped.
         * @param deltaPos The relative coordinate shift.
         * @param durationSeconds The duration in which the panning operation will be completed in seconds.
         */
        void pan(const MapVec& deltaPos, float durationSeconds);
        /**
         * Sets the new absolute focus position. The new focus position is expected to be in
         * the coordinate system of the base projection. The new focus position will be clamped to
         * the world bounds and to the bounds set by Options::setPanBounds.
         *
         * If durationSeconds > 0 the panning operation will be animated over time. If the previous panning animation has not
         * finished by the time this method is called, it will be stopped.
         * @param pos The new absolute focus position.
         * @param durationSeconds The duration in which the panning operation will be completed in seconds.
         */
        void setFocusPos(const MapPos& pos, float durationSeconds);
        
        /**
         * Rotates the view relative to the current rotation value. Positive values rotate clockwise, negative values counterclockwise.
         * The new calculated rotation value will be wrapped to the range of (-180 .. 180]. Rotations are ignored if Options::setRotatable 
         * is set to false.
         *
         * If durationSeconds > 0 the rotating operation will be animated over time. If the previous rotating animation has not
         * finished by the time this method is called, it will be stopped.
         * @param deltaAngle The delta rotation value in degrees.
         * @param durationSeconds The duration in which the rotation operation will be completed in seconds.
         */
        void rotate(float deltaAngle, float durationSeconds);
        /**
         * Rotates the view relative to the current rotation value. Positive values rotate clockwise, negative values counterclockwise.
         * The new calculated rotation value will be wrapped to the range of (-180 .. 180]. Rotations are ignored if Options::setRotatable
         * is set to false.
         *
         * Rotating is done around the specified target position, keeping it at the same location on the screen.
         *
         * If durationSeconds > 0 the rotating operation will be animated over time. If the previous rotating animation has not
         * finished by the time this method is called, it will be stopped.
         * @param deltaAngle The delta angle value in degrees.
         * @param targetPos The zooming target position in the coordinate system of the base projection.
         * @param durationSeconds The duration in which the rotation operation will be completed in seconds.
         */
        void rotate(float deltaAngle, const MapPos& targetPos, float durationSeconds);
        /**
         * Sets the new absolute rotation value. 0 means look north, 90 means west, -90 means east and 180 means south.
         * The rotation value will be wrapped to the range of (-180 .. 180]. Rotations are ignored if Options::setRotatable
         * is set to false.
         *
         * If durationSeconds > 0 the rotating operation will be animated over time. If the previous rotating animation has not
         * finished by the time this method is called, it will be stopped.
         * @param angle The new absolute angle value in degrees.
         * @param durationSeconds The duration in which the rotation operation will be completed in seconds.
         */
        void setRotation(float angle, float durationSeconds);
        /**
         * Sets the new absolute rotation value. 0 means look north, 90 means west, -90 means east and 180 means south.
         * The rotation value will be wrapped to the range of (-180 .. 180]. Rotations are ignored if Options::setRotatable 
         * is set to false.
         *
         * Rotating is done around the specified target position, keeping it at the same location on the screen.
         *
         * If durationSeconds > 0 the rotating operation will be animated over time. If the previous rotating animation has not
         * finished by the time this method is called, it will be stopped.
         * @param angle The new absolute angle value in degrees.
         * @param targetPos The zooming target position in the coordinate system of the base projection.
         * @param durationSeconds The duration in which the rotation operation will be completed in seconds.
         */
        void setRotation(float angle, const MapPos& targetPos, float durationSeconds);
        
        /**
         * Tilts the view relative to the current tilt value. Positive values tilt the view down towards the map, 
         * negative values tilt the view up towards the horizon. The new calculated tilt value will be clamped to
         * the range of [30 .. 90] and to the range set by Options::setZoomRange.
         *
         * If durationSeconds > 0 the tilting operation will be animated over time. If the previous tilting animation has not
         * finished by the time this method is called, it will be stopped.
         * @param deltaTilt The number of degrees the camera should be tilted by.
         * @param durationSeconds The duration in which the tilting operation will be completed in seconds.
         */
        void tilt(float deltaTilt, float durationSeconds);
        /**
         * Sets the new absolute tilt value. 0 means look directly at the horizon, 90 means look directly down. The
         * minimum tilt angle is 30 degrees and the maximum is 90 degrees. The tilt value can be further constrained
         * by the Options::setTiltRange method. Values exceeding these ranges will be clamped.
         *
         * If durationSeconds > 0 the tilting operation will be animated over time. If the previous tilting animation has not
         * finished by the time this method is called, it will be stopped.
         * @param tilt The new absolute tilt value in degrees.
         * @param durationSeconds The duration in which the tilting operation will be completed in seconds.
         */
        void setTilt(float tilt, float durationSeconds);
        
        /**
         * Zooms the view relative to the current zoom value. Positive values zoom in, negative values zoom out.
         * The new calculated zoom value will be clamped to the range of [0 .. 24] and to the range set by Options::setZoomRange.
         *
         * If durationSeconds > 0 the zooming operation will be animated over time. If the previous zooming animation has not
         * finished by the time this method is called, it will be stopped.
         * @param deltaZoom The delta zoom value.
         * @param durationSeconds The duration in which the zooming operation will be completed in seconds.
         */
        void zoom(float deltaZoom, float durationSeconds);
        /**
         * Zooms the view relative to the current zoom value. Positive values zoom in, negative values zoom out.
         * The new calculated zoom value will be clamped to the range of [0 .. 24] and to the range set by Options::setZoomRange.
         *
         * Zooming is done towards the specified target position, keeping it at the same location on the screen.
         *
         * If durationSeconds > 0 the zooming operation will be animated over time. If the previous zooming animation has not
         * finished by the time this method is called, it will be stopped.
         * @param deltaZoom The delta zoom value.
         * @param targetPos The zooming target position in the coordinate system of the base projection.
         * @param durationSeconds The duration in which the zooming operation will be completed in seconds.
         */
        void zoom(float deltaZoom, const MapPos& targetPos, float durationSeconds);
        /**
         * Sets the new absolute zoom value. The minimum zoom value is 0, which means absolutely zoomed out and the maximum
         * zoom value is 24. The zoom value can be further constrained by the Options::setZoomRange method. Values
         * exceeding these ranges will be clamped. 
         *
         * If durationSeconds > 0 the zooming operation will be animated over time. If the previous zooming animation has not
         * finished by the time this method is called, it will be stopped.
         * @param zoom The new absolute zoom value.
         * @param durationSeconds The duration in which the zooming operation will be completed in seconds.
         */
        void setZoom(float zoom, float durationSeconds);
        /**
         * Sets the new absolute zoom value. The minimum zoom value is 0, which means absolutely zoomed out and the maximum 
         * zoom value is 24. The zoom value can be further constrained by the Options::setZoomRange method. Values 
         * exceeding these ranges will be clamped.
         *
         * Zooming is done towards the specified target position, keeping it at the same location on the screen.
         *
         * If durationSeconds > 0, the zooming operation will be animated over time. If the previous zooming animation has not
         * finished by the time this method is called, it will be stopped.
         * @param zoom The new absolute zoom value.
         * @param targetPos The zooming target position in the coordinate system of the base projection.
         * @param durationSeconds The duration in which the zooming operation will be completed in seconds.
         */
        void setZoom(float zoom, const MapPos& targetPos, float durationSeconds);
        
        /**
         * Animate the view parameters (focus position, tilt, rotation, zoom) so that the specified bounding box becomes fully visible.
         * This method does not work before the screen size is set.
         * @param mapBounds The bounding box on the map to be made visible in the base projection's coordinate system.
         * @param screenBounds The screen bounding box where to fit the map bounding box.
         * @param integerZoom If true, then closest integer zoom level will be used. If false, exact fractional zoom level will be used.
         * @param durationSeconds The duration in which the operation will be completed in seconds.
         */
        void moveToFitBounds(const MapBounds& mapBounds, const ScreenBounds& screenBounds, bool integerZoom, float durationSeconds);
        /**
         * Animate the view parameters (focus position, tilt, rotation, zoom) so that the specified bounding box becomes fully visible.
         * Also supports resetting the tilt and rotation angles over the course of the animation.
         * This method does not work before the screen size is set.
         * @param mapBounds The bounding box on the map to be made visible in the base projection's coordinate system.
         * @param screenBounds The screen bounding box where to fit the map bounding box.
         * @param integerZoom If true, then closest integer zoom level will be used. If false, exact fractional zoom level will be used.
         * @param resetTilt If true, view will be untilted. If false, current tilt will be kept.
         * @param resetRotation If true, rotation will be reset. If false, current rotation will be kept.
         * @param durationSeconds The duration in which the operation will be completed in seconds.
         */
        void moveToFitBounds(const MapBounds& mapBounds, const ScreenBounds& screenBounds, bool integerZoom, bool resetRotation, bool resetTilt, float durationSeconds);
        
        /**
         * Returns the map event listener. May be null.
         * @return The map event listener.
         */
        std::shared_ptr<MapEventListener> getMapEventListener() const;
        /**
         * Sets the map event listener. If a null pointer is passed no map events will be generated. The default is null.
         * @param mapEventListener The new map event listener.
         */
        void setMapEventListener(const std::shared_ptr<MapEventListener>& mapEventListener);
        
        /**
         * Returns the redraw request listener.
         * @return The redraw request listener.
         */
        std::shared_ptr<RedrawRequestListener> getRedrawRequestListener() const;
        /**
         * Sets the listener which will notified when the map needs to be redrawn
         * @param listener The redraw listener.
         */
        void setRedrawRequestListener(const std::shared_ptr<RedrawRequestListener>& listener);

        /**
         * Calculates the map position corresponding to a screen position, using the current view parameters.
         * @param screenPos The screen position.
         * @return The calculated map position in base projection coordinate system.
         */
        MapPos screenToMap(const ScreenPos& screenPos);
        /**
         * Calculates the screen position corresponding to a map position, using the current view parameters.
         * @param mapPos The map position in base projection coordinate system.
         * @return The calculated screen position.
         */
        ScreenPos mapToScreen(const MapPos& mapPos);
        
        /**
         * Cancels all qued tasks such as tile and vector data fetches. Tasks that have already started
         * may continue until they finish. Tasks that are added after this method call are not affected.
         */
        void cancelAllTasks();
    
        /**
         * Releases the memory occupied by the preloading area. Calling this method releases some
         * memory if preloading is enabled, but means that the area right outside the visible area has to be
         * fetched again.
         */
        void clearPreloadingCaches();
    
        /**
         * Releases memory occupied by all caches. Calling this means that everything has to be fetched again,
         * including the visible area.
         */
        void clearAllCaches();
    
    private:
        std::shared_ptr<CancelableThreadPool> _envelopeThreadPool;
        std::shared_ptr<CancelableThreadPool> _tileThreadPool;
        std::shared_ptr<Options> _options;
        std::shared_ptr<Layers> _layers;
        std::shared_ptr<MapRenderer> _mapRenderer;
        
        std::shared_ptr<TouchHandler> _touchHandler;
        
        mutable std::mutex _mutex;
    };
    
}

#endif
