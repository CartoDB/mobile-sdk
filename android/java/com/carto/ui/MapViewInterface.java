package com.carto.ui;

import com.carto.components.Options;
import com.carto.components.Layers;
import com.carto.core.MapPos;
import com.carto.core.MapVec;
import com.carto.core.MapBounds;
import com.carto.core.ScreenPos;
import com.carto.core.ScreenBounds;
import com.carto.renderers.MapRenderer;

/**
 * MapViewInterface is a common interface for map rendering and interaction implementations.
 */
public interface MapViewInterface {
    
    /**
     * Returns the Layers object, that can be used for adding and removing map layers.
     * @return The Layer object.
     */
    public Layers getLayers();

    /**
     * Returns the Options object, that can be used for modifying various map options.
     * @return the Option object.
     */
    public Options getOptions();

    /**
     * Returns the MapRenderer object, that can be used for controlling rendering related options.
     * @return the MapRenderer object.
     */
    public MapRenderer getMapRenderer();

    /**
     * Returns the position that the camera is currently looking at.
     * @return The current focus position in the coordinate system of the base projection.
     */
    public MapPos getFocusPos();

    /**
     * Returns the map rotation in degrees. 0 means looking north, 90 means west, -90 means east and 180 means south.
     * @return The map rotation in degrees in range of (-180 .. 180].
     */
    public float getMapRotation();

    /**
     * Returns the tilt angle in degrees. 0 means looking directly at the horizon, 90 means looking directly down.
     * @return The tilt angle in degrees.
     */
    public float getTilt();

    /**
     * Returns the zoom level. The value returned is never negative, 0 means absolutely zoomed out and all other
     * values describe some level of zoom.
     * @return The zoom level.
     */
    public float getZoom();

    /**
     * Pans the view relative to the current focus position. The deltaPos vector is expected to be in
     * the coordinate system of the base projection. The new calculated focus position will be clamped to
     * the world bounds and to the bounds set by Options::setPanBounds.
     * 
     * If durationSeconds &gt; 0 the panning operation will be animated over time. If the previous panning animation has not
     * finished by the time this method is called, it will be stopped.
     * 
     * @param deltaPos The coordinate difference the map should be moved by.
     * @param durationSeconds The duration in which the tilting operation will be completed in seconds.
     */
    public void pan(MapVec deltaPos, float durationSeconds);

    /**
     * Sets the new absolute focus position. The new focus position is expected to be in
     * the coordinate system of the base projection. The new focus position will be clamped to
     * the world bounds and to the bounds set by Options::setPanBounds.
     * 
     * If durationSeconds &gt; 0 the panning operation will be animated over time. If the previous panning animation has not
     * finished by the time this method is called, it will be stopped.
     * 
     * @param pos The new focus point position in base coordinate system.
     * @param durationSeconds The duration in which the tilting operation will be completed in seconds.
     */
    public void setFocusPos(MapPos pos, float durationSeconds);

    /**
     * Rotates the view relative to the current rotation value. Positive values rotate clockwise, negative values counterclockwise.
     * The new calculated rotation value will be wrapped to the range of (-180 .. 180]. Rotations are ignored if Options::setRotatable 
     * is set to false.
     * 
     * If durationSeconds &gt; 0 the rotating operation will be animated over time. If the previous rotating animation has not
     * finished by the time this method is called, it will be stopped.
     * 
     * @param deltaAngle The delta angle value in degrees.
     * @param durationSeconds The duration in which the zooming operation will be completed in seconds.
     */
    public void rotate(float deltaAngle, float durationSeconds);

    /**
     * Rotates the view relative to the current rotation value. Positive values rotate clockwise, negative values counterclockwise.
     * The new calculated rotation value will be wrapped to the range of (-180 .. 180]. Rotations are ignored if Options::setRotatable
     * is set to false.
     * 
     * Rotating is done around the specified target position, keeping it at the same location on the screen.
     * 
     * If durationSeconds &gt; 0 the rotating operation will be animated over time. If the previous rotating animation has not
     * finished by the time this method is called, it will be stopped.
     * 
     * @param deltaAngle The delta angle value in degrees.
     * @param targetPos The zooming target position in the coordinate system of the base projection.
     * @param durationSeconds The duration in which the zooming operation will be completed in seconds.
     */
    public void rotate(float deltaAngle, MapPos targetPos, float durationSeconds);

    /**
     * Sets the new absolute rotation value. 0 means look north, 90 means west, -90 means east and 180 means south.
     * The rotation value will be wrapped to the range of (-180 .. 180]. Rotations are ignored if Options::setRotatable
     * is set to false.
     * 
     * If durationSeconds &gt; 0 the rotating operation will be animated over time. If the previous rotating animation has not
     * finished by the time this method is called, it will be stopped.
     * 
     * @param angle The new absolute rotation angle value in degrees.
     * @param durationSeconds The duration in which the zooming operation will be completed in seconds.
     */
    public void setMapRotation(float angle, float durationSeconds);

    /**
     * Sets the new absolute rotation value. 0 means look north, 90 means west, -90 means east and 180 means south.
     * The rotation value will be wrapped to the range of (-180 .. 180]. Rotations are ignored if Options::setRotatable 
     * is set to false.
     * 
     * Rotating is done around the specified target position, keeping it at the same location on the screen.
     * 
     * If durationSeconds &gt; 0 the rotating operation will be animated over time. If the previous rotating animation has not
     * finished by the time this method is called, it will be stopped.
     * 
     * @param angle The new absolute rotation angle value in degrees.
     * @param targetPos The zooming target position in the coordinate system of the base projection.
     * @param durationSeconds The duration in which the zooming operation will be completed in seconds.
     */
    public void setMapRotation(float angle, MapPos targetPos, float durationSeconds);

    /**
     * Tilts the view relative to the current tilt value. Positive values tilt the view down towards the map, 
     * negative values tilt the view up towards the horizon. The new calculated tilt value will be clamped to
     * the range of [30 .. 90] and to the range set by Options::setZoomRange.
     * 
     * If durationSeconds &gt; 0 the tilting operation will be animated over time. If the previous tilting animation has not
     * finished by the time this method is called, it will be stopped.
     * @param deltaTilt The number of degrees the camera should be tilted by.
     * @param durationSeconds The duration in which the tilting operation will be completed in seconds.
     */
    public void tilt(float deltaTilt, float durationSeconds);

    /**
     * Sets the new absolute tilt value. 0 means look directly at the horizon, 90 means look directly down. The
     * minimum tilt angle is 30 degrees and the maximum is 90 degrees. The tilt value can be further constrained
     * by the Options::setTiltRange method. Values exceeding these ranges will be clamped.
     * 
     * If durationSeconds &gt; 0 the tilting operation will be animated over time. If the previous tilting animation has not
     * finished by the time this method is called, it will be stopped.
     * @param tilt The new absolute tilt value in degrees.
     * @param durationSeconds The duration in which the tilting operation will be completed in seconds.
     */
    public void setTilt(float tilt, float durationSeconds);

    /**
     * Zooms the view relative to the current zoom value. Positive values zoom in, negative values zoom out.
     * The new calculated zoom value will be clamped to the range of [0 .. 24] and to the range set by Options::setZoomRange.
     * 
     * If durationSeconds &gt; 0 the zooming operation will be animated over time. If the previous zooming animation has not
     * finished by the time this method is called, it will be stopped.
     * @param deltaZoom The delta zoom value.
     * @param durationSeconds The duration in which the zooming operation will be completed in seconds.
     */
    public void zoom(float deltaZoom, float durationSeconds);

    /**
     * Zooms the view relative to the current zoom value. Positive values zoom in, negative values zoom out.
     * The new calculated zoom value will be clamped to the range of [0 .. 24] and to the range set by Options::setZoomRange.
     * 
     * Zooming is done towards the specified target position, keeping it at the same location on the screen.
     * 
     * If durationSeconds &gt; 0 the zooming operation will be animated over time. If the previous zooming animation has not
     * finished by the time this method is called, it will be stopped.
     * @param deltaZoom The delta zoom value.
     * @param targetPos The zooming target position in the coordinate system of the base projection.
     * @param durationSeconds The duration in which the zooming operation will be completed in seconds.
     */
    public void zoom(float deltaZoom, MapPos targetPos, float durationSeconds);

    /**
     * Sets the new absolute zoom value. The minimum zoom value is 0, which means absolutely zoomed out and the maximum
     * zoom value is 24. The zoom value can be further constrained by the Options::setZoomRange method. Values
     * exceeding these ranges will be clamped. 
     * 
     * If durationSeconds &gt; 0 the zooming operation will be animated over time. If the previous zooming animation has not
     * finished by the time this method is called, it will be stopped.
     * @param zoom The new absolute zoom value.
     * @param durationSeconds The duration in which the zooming operation will be completed in seconds.
     */
    public void setZoom(float zoom, float durationSeconds);

    /**
     * Sets the new absolute zoom value. The minimum zoom value is 0, which means absolutely zoomed out and the maximum 
     * zoom value is 24. The zoom value can be further constrained by the Options::setZoomRange method. Values 
     * exceeding these ranges will be clamped.
     * 
     * Zooming is done towards the specified target position, keeping it at the same location on the screen.
     * 
     * If durationSeconds &gt; 0, the zooming operation will be animated over time. If the previous zooming animation has not
     * finished by the time this method is called, it will be stopped.
     * @param zoom The new absolute zoom value.
     * @param targetPos The zooming target position in the coordinate system of the base projection.
     * @param durationSeconds The duration in which the zooming operation will be completed in seconds.
     */
    public void setZoom(float zoom, MapPos targetPos, float durationSeconds);

    /**
     * Animate the view parameters (focus position, tilt, rotation, zoom) so that the specified bounding box becomes fully visible.
     * This method does not work before the screen size is set.
     * @param mapBounds The bounding box on the map to be made visible in the base projection's coordinate system.
     * @param screenBounds The screen bounding box where to fit the map bounding box.
     * @param integerZoom If true, then closest integer zoom level will be used. If false, exact fractional zoom level will be used.
     * @param durationSeconds The duration in which the operation will be completed in seconds.
     */
    public void moveToFitBounds(MapBounds mapBounds, ScreenBounds screenBounds, boolean integerZoom, float durationSeconds);

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
    public void moveToFitBounds(MapBounds mapBounds, ScreenBounds screenBounds, boolean integerZoom, boolean resetRotation, boolean resetTilt, float durationSeconds);
    
    /**
     * Returns the map event listener. May be null.
     * @return The map event listener.
     */
    public MapEventListener getMapEventListener();

    /**
     * Sets the map event listener. If a null pointer is passed no map events will be generated. The default is null.
     * @param mapEventListener The new map event listener.
     */
    public void setMapEventListener(MapEventListener mapEventListener);

    /**
     * Calculates the map position corresponding to a screen position, using the current view parameters.
     * @param screenPos The screen position.
     * @return The calculated map position in base projection coordinate system. If the given screen position is not on the map, then NaNs are returned.
     */
    public MapPos screenToMap(ScreenPos screenPos);

    /**
     * Calculates the screen position corresponding to a map position, using the current view parameters.
     * @param mapPos The map position in base projection coordinate system.
     * @return The calculated screen position. Can be off-screen.
     */
    public ScreenPos mapToScreen(MapPos mapPos);

    /**
     * Cancels all qued tasks such as tile and vector data fetches. Tasks that have already started
     * may continue until they finish. Tasks that are added after this method call are not affected.
     */
    public void cancelAllTasks();

    /**
     * Releases the memory occupied by the preloading area. Calling this method releases some
     * memory if preloading is enabled, but means that the area right outside the visible area has to be
     * fetched again.
     */
    public void clearPreloadingCaches();

    /**
     * Releases memory occupied by all caches. Calling this means that everything has to be fetched again,
     * including the visible area.
     */
    public void clearAllCaches();
    
}
