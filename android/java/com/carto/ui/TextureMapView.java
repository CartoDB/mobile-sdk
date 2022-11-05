package com.carto.ui;

import java.lang.reflect.Method;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

import android.content.Context;
import android.content.SharedPreferences;
import android.content.res.AssetManager;
import android.content.res.TypedArray;
import android.opengl.GLSurfaceView;
import android.util.AttributeSet;
import android.view.MotionEvent;

import com.carto.components.Options;
import com.carto.components.Layers;
import com.carto.components.LicenseManagerListener;
import com.carto.core.MapBounds;
import com.carto.core.MapPos;
import com.carto.core.ScreenPos;
import com.carto.core.ScreenBounds;
import com.carto.core.MapVec;
import com.carto.renderers.MapRenderer;
import com.carto.renderers.RedrawRequestListener;
import com.carto.utils.AndroidUtils;
import com.carto.utils.AssetUtils;

/**
 * MapView is a view class supporting map rendering and interaction.
 */
public class TextureMapView extends GLTextureView implements GLSurfaceView.Renderer, MapViewInterface {
    private static final int NATIVE_ACTION_POINTER_1_DOWN = 0;
    private static final int NATIVE_ACTION_POINTER_2_DOWN = 1;
    private static final int NATIVE_ACTION_MOVE = 2;
    private static final int NATIVE_ACTION_CANCEL = 3;
    private static final int NATIVE_ACTION_POINTER_1_UP = 4;
    private static final int NATIVE_ACTION_POINTER_2_UP = 5;
    private static final int NATIVE_NO_COORDINATE = -1;

    private static final int INVALID_POINTER_ID = -1;

    static {
        try {
            System.loadLibrary("carto_mobile_sdk");
            AndroidUtils.attachJVM(MapView.class);
        } catch (Throwable t) {
            android.util.Log.e("carto_mobile_sdk", "Failed to initialize Carto Mobile Maps SDK, native .so library failed to load?", t);
        }
    }

    private static AssetManager assetManager;

    private BaseMapView baseMapView;

    private int pointer1Id = INVALID_POINTER_ID;
    private int pointer2Id = INVALID_POINTER_ID;

    /**
     * Registers the SDK license. This class method and must be called before
     * creating any actual MapView instances.
     * @param licenseKey The license string provided for this application.
     * @param context Application context for the license.
     * @return True if license is valid, false if not.
     */
    public static boolean registerLicense(final String licenseKey, Context context) {
        // Connect context info and assets manager to native part
        AndroidUtils.setContext(context);
        if (assetManager == null) {
            assetManager = context.getApplicationContext().getAssets();
            AssetUtils.setAssetManagerPointer(assetManager);
        }

        final String oldKey = "license_key_old";
        final String newKey = "license_key_new";
        final SharedPreferences prefs = context.getSharedPreferences(context.getPackageName() + "_carto_mobile_sdk1_preferences", Context.MODE_PRIVATE);
        LicenseManagerListener listener = new LicenseManagerListener() {
            @Override
            public void onLicenseUpdated(String newLicenseKey) {
                try {
                    prefs.edit().putString(oldKey, licenseKey).putString(newKey, newLicenseKey).commit();
                } catch (Exception e) {
                    com.carto.utils.Log.info("MapView.registerLicense: Failed to save license key");
                }
            }
        };
        String newLicenseKey = null;
        try {
            String oldLicenseKey = prefs.getString(oldKey, null);
            if (oldLicenseKey != null && oldLicenseKey.equals(licenseKey)) {
                newLicenseKey = prefs.getString(newKey, null);
            }
        } catch (Exception e) {
            com.carto.utils.Log.info("MapView.registerLicense: Failed to read license key");
        }
        return BaseMapView.registerLicense(newLicenseKey != null ? newLicenseKey : licenseKey, listener);
    }

    /**
     * Creates a new MapView object from a context object.
     * @param context The context object.
     */
    public TextureMapView(Context context) {
        this(context, null);
    }

    /**
     * Creates a new MapView object from a context object and attributes.
     * @param context The context object.
     * @param attrs The attributes.
     */
    public TextureMapView(Context context, AttributeSet attrs) {
        super(context, attrs);

        // Unless explictly not clickable, make clickable by default
        boolean clickable = true;
        boolean longClickable = true;
        try {
            TypedArray ta = context.obtainStyledAttributes(attrs, new int[]{ android.R.attr.clickable, android.R.attr.longClickable });
            clickable = ta.getBoolean(0, true);
            longClickable = ta.getBoolean(1, true);
            ta.recycle();
        } catch (Exception e) {
            com.carto.utils.Log.warn("MapView: Failed to read attributes");
        }
        setClickable(clickable);
        setLongClickable(longClickable);

        if (!isInEditMode()) {
            // Connect context info and assets manager to native part
            AndroidUtils.setContext(context);
            if (assetManager == null) {
                com.carto.utils.Log.warn("MapView: MapView created before MapView.registerLicense is called");

                assetManager = context.getApplicationContext().getAssets();
                AssetUtils.setAssetManagerPointer(assetManager);
            }

            // Set asset manager pointer to allow native access to assets
            assetManager = context.getApplicationContext().getAssets();
            AssetUtils.setAssetManagerPointer(assetManager);

            baseMapView = new BaseMapView();
            baseMapView.getOptions().setDPI(getResources().getDisplayMetrics().densityDpi);
            baseMapView.setRedrawRequestListener(new TextureMapRedrawRequestListener(this));

            setPreserveEGLContextOnPause(true);
            setEGLContextClientVersion(2);
            setEGLConfigChooser(new ConfigChooser());
            setRenderer(this);
            setRenderMode(RENDERMODE_WHEN_DIRTY);
        }
    }

    /**
     * Deletes the resources associated with the MapView.
     * The method can be used to dispose native objects immediately,
     * without waiting for next GC cycle.
     */
    public synchronized void delete() {
        if (baseMapView != null) {
            baseMapView.setRedrawRequestListener(null);
            baseMapView.delete();
            baseMapView = null;
        }
    }

    @Override
    public synchronized void onSurfaceCreated(GL10 gl, EGLConfig config) {
        if (baseMapView != null) {
            baseMapView.onSurfaceCreated();
        }
    }

    @Override
    public synchronized void onSurfaceChanged(GL10 gl, int width, int height) {
        if (baseMapView != null) {
            baseMapView.onSurfaceChanged(width, height);
        }
    }

    @Override
    public synchronized void onDrawFrame(GL10 gl) {
        if (baseMapView != null) {
            baseMapView.onDrawFrame();
        }
    }

    @Override
    public synchronized boolean onTouchEvent(MotionEvent event) {
        if (baseMapView == null) {
            return false;
        }

        boolean clickable = isClickable() || isLongClickable();
        if (!isEnabled() || !clickable) {
            return clickable;
        }

        try {
            int pointer1Index;
            int pointer2Index;
            switch (event.getActionMasked()) {
            case MotionEvent.ACTION_DOWN:
                pointer1Index = event.getActionIndex();
                pointer1Id = event.getPointerId(pointer1Index);
                baseMapView.onInputEvent(NATIVE_ACTION_POINTER_1_DOWN, 
                        event.getX(pointer1Index), event.getY(pointer1Index), 
                        NATIVE_NO_COORDINATE, NATIVE_NO_COORDINATE);
                break;
            case MotionEvent.ACTION_POINTER_DOWN:
                if (event.getPointerCount() == 2) {
                    // Check which pointer to use
                    if (pointer1Id != INVALID_POINTER_ID) {
                        pointer1Index = event.findPointerIndex(pointer1Id);
                        pointer2Index = event.getActionIndex();
                        pointer2Id = event.getPointerId(event.getActionIndex());
                    } else if (pointer2Id != INVALID_POINTER_ID) {
                        pointer2Index = event.findPointerIndex(pointer2Id);
                        pointer1Index = event.getActionIndex();
                        pointer1Id = event.getPointerId(event.getActionIndex());
                    } else {
                        break;
                    }
                    baseMapView.onInputEvent(NATIVE_ACTION_POINTER_2_DOWN, 
                            event.getX(pointer1Index), event.getY(pointer1Index),
                            event.getX(pointer2Index), event.getY(pointer2Index));
                }
                break;
            case MotionEvent.ACTION_MOVE:
                if (pointer1Id != INVALID_POINTER_ID && pointer2Id == INVALID_POINTER_ID) {
                    pointer1Index = event.findPointerIndex(pointer1Id);
                    baseMapView.onInputEvent(NATIVE_ACTION_MOVE, 
                            event.getX(pointer1Index), event.getY(pointer1Index), 
                            NATIVE_NO_COORDINATE, NATIVE_NO_COORDINATE);
                } else if (pointer1Id != INVALID_POINTER_ID && pointer2Id != INVALID_POINTER_ID) {
                    pointer1Index = event.findPointerIndex(pointer1Id);
                    pointer2Index = event.findPointerIndex(pointer2Id);
                    baseMapView.onInputEvent(NATIVE_ACTION_MOVE, 
                            event.getX(pointer1Index), event.getY(pointer1Index), 
                            event.getX(pointer2Index), event.getY(pointer2Index));
                }
                break;
            case MotionEvent.ACTION_CANCEL:
                baseMapView.onInputEvent(NATIVE_ACTION_CANCEL, 
                        NATIVE_NO_COORDINATE, NATIVE_NO_COORDINATE, 
                        NATIVE_NO_COORDINATE, NATIVE_NO_COORDINATE);
                pointer1Id = INVALID_POINTER_ID;
                pointer2Id = INVALID_POINTER_ID;
                break;
            case MotionEvent.ACTION_UP:
            case MotionEvent.ACTION_POINTER_UP:
                int pointerIndex = event.getActionIndex();
                int pointerId = event.getPointerId(pointerIndex);
                // Single pointer
                if (pointer1Id == pointerId && pointer2Id == INVALID_POINTER_ID) {
                    pointer1Index = event.findPointerIndex(pointer1Id);
                    baseMapView.onInputEvent(NATIVE_ACTION_POINTER_1_UP, 
                            event.getX(pointer1Index), event.getY(pointer1Index), 
                            NATIVE_NO_COORDINATE, NATIVE_NO_COORDINATE);
                    pointer1Id = INVALID_POINTER_ID;
                    // Dual pointer, first pointer up
                } else if (pointer1Id == pointerId) {
                    pointer1Index = event.findPointerIndex(pointer1Id);
                    pointer2Index = event.findPointerIndex(pointer2Id);
                    baseMapView.onInputEvent(NATIVE_ACTION_POINTER_1_UP, 
                            event.getX(pointer1Index), event.getY(pointer1Index), 
                            event.getX(pointer2Index), event.getY(pointer2Index));
                    pointer1Id = pointer2Id;
                    pointer2Id = INVALID_POINTER_ID;
                    // Dual pointer, second finger up
                } else if (pointer2Id == pointerId) {
                    pointer1Index = event.findPointerIndex(pointer1Id);
                    pointer2Index = event.findPointerIndex(pointer2Id);
                    baseMapView.onInputEvent(NATIVE_ACTION_POINTER_2_UP, 
                            event.getX(pointer1Index), event.getY(pointer1Index), 
                            event.getX(pointer2Index), event.getY(pointer2Index));
                    pointer2Id = INVALID_POINTER_ID;
                }
                break;
            }
        }
        catch (IllegalArgumentException e) {
            com.carto.utils.Log.error("MapView.onTouchEvent: " + e);
        }
        return true;
    }

    /**
     * Returns the Layers object, that can be used for adding and removing map layers.
     * @return The Layer object.
     */
    public Layers getLayers() {
        return baseMapView.getLayers();
    }

    /**
     * Returns the Options object, that can be used for modifying various map options.
     * @return the Option object.
     */
    public Options getOptions() {
        return baseMapView.getOptions();
    }

    /**
     * Returns the MapRenderer object, that can be used for controlling rendering related options.
     * @return the MapRenderer object.
     */
    public MapRenderer getMapRenderer() {
        return baseMapView.getMapRenderer();
    }

    /**
     * Returns the position that the camera is currently looking at.
     * @return The current focus position in the coordinate system of the base projection.
     */
    public MapPos getFocusPos() {
        return baseMapView.getFocusPos();
    }

    /**
     * Returns the map rotation in degrees. 0 means looking north, 90 means west, -90 means east and 180 means south.
     * @return The map rotation in degrees in range of (-180 .. 180].
     */
    public float getMapRotation() {
        return baseMapView.getRotation();
    }

    /**
     * Returns the tilt angle in degrees. 0 means looking directly at the horizon, 90 means looking directly down.
     * @return The tilt angle in degrees.
     */
    public float getTilt() {
        return baseMapView.getTilt();
    }

    /**
     * Returns the zoom level. The value returned is never negative, 0 means absolutely zoomed out and all other
     * values describe some level of zoom.
     * @return The zoom level.
     */
    public float getZoom() {
        return baseMapView.getZoom();
    }

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
    public void pan(MapVec deltaPos, float durationSeconds) {
        baseMapView.pan(deltaPos, durationSeconds);
    }

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
    public void setFocusPos(MapPos pos, float durationSeconds) {
        baseMapView.setFocusPos(pos, durationSeconds);
    }

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
    public void rotate(float deltaAngle, float durationSeconds) {
        baseMapView.rotate(deltaAngle, durationSeconds);
    }

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
    public void rotate(float deltaAngle, MapPos targetPos, float durationSeconds) {
        baseMapView.rotate(deltaAngle, targetPos, durationSeconds);
    }

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
    public void setMapRotation(float angle, float durationSeconds) {
        baseMapView.setRotation(angle, durationSeconds);
    }

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
    public void setMapRotation(float angle, MapPos targetPos, float durationSeconds) {
        baseMapView.setRotation(angle, targetPos, durationSeconds);
    }

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
    public void tilt(float deltaTilt, float durationSeconds) {
        baseMapView.tilt(deltaTilt, durationSeconds);
    }

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
    public void setTilt(float tilt, float durationSeconds) {
        baseMapView.setTilt(tilt, durationSeconds);
    }

    /**
     * Zooms the view relative to the current zoom value. Positive values zoom in, negative values zoom out.
     * The new calculated zoom value will be clamped to the range of [0 .. 24] and to the range set by Options::setZoomRange.
     *
     * If durationSeconds &gt; 0 the zooming operation will be animated over time. If the previous zooming animation has not
     * finished by the time this method is called, it will be stopped.
     * @param deltaZoom The delta zoom value.
     * @param durationSeconds The duration in which the zooming operation will be completed in seconds.
     */
    public void zoom(float deltaZoom, float durationSeconds) {
        baseMapView.zoom(deltaZoom, durationSeconds);
    }

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
    public void zoom(float deltaZoom, MapPos targetPos, float durationSeconds) {
        baseMapView.zoom(deltaZoom, targetPos, durationSeconds);
    }

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
    public void setZoom(float zoom, float durationSeconds) {
        baseMapView.setZoom(zoom, durationSeconds);
    }

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
    public void setZoom(float zoom, MapPos targetPos, float durationSeconds) {
        baseMapView.setZoom(zoom, targetPos, durationSeconds);
    }

    /**
     * Animate the view parameters (focus position, tilt, rotation, zoom) so that the specified bounding box becomes fully visible.
     * This method does not work before the screen size is set.
     * @param mapBounds The bounding box on the map to be made visible in the base projection's coordinate system.
     * @param screenBounds The screen bounding box where to fit the map bounding box.
     * @param integerZoom If true, then closest integer zoom level will be used. If false, exact fractional zoom level will be used.
     * @param durationSeconds The duration in which the operation will be completed in seconds.
     */
    public void moveToFitBounds(MapBounds mapBounds, ScreenBounds screenBounds, boolean integerZoom, float durationSeconds) {
        baseMapView.moveToFitBounds(mapBounds, screenBounds, integerZoom, durationSeconds);
    }

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
    public void moveToFitBounds(MapBounds mapBounds, ScreenBounds screenBounds, boolean integerZoom, boolean resetRotation,
                                boolean resetTilt, float durationSeconds) {
        baseMapView.moveToFitBounds(mapBounds, screenBounds, integerZoom, resetRotation, resetTilt, durationSeconds);
    }

    /**
     * Returns the map event listener. May be null.
     * @return The map event listener.
     */
    public MapEventListener getMapEventListener() {
        return baseMapView.getMapEventListener();
    }

    /**
     * Sets the map event listener. If a null pointer is passed no map events will be generated. The default is null.
     * @param mapEventListener The new map event listener.
     */
    public void setMapEventListener(MapEventListener mapEventListener) {
        baseMapView.setMapEventListener(mapEventListener);
    }

    /**
     * Calculates the map position corresponding to a screen position, using the current view parameters.
     * @param screenPos The screen position.
     * @return The calculated map position in base projection coordinate system. If the given screen position is not on the map, then NaNs are returned.
     */
    public MapPos screenToMap(ScreenPos screenPos) {
        return baseMapView.screenToMap(screenPos);
    }

    /**
     * Calculates the screen position corresponding to a map position, using the current view parameters.
     * @param mapPos The map position in base projection coordinate system.
     * @return The calculated screen position. Can be off-screen.
     */
    public ScreenPos mapToScreen(MapPos mapPos) {
        return baseMapView.mapToScreen(mapPos);
    }

    /**
     * Cancels all qued tasks such as tile and vector data fetches. Tasks that have already started
     * may continue until they finish. Tasks that are added after this method call are not affected.
     */
    public void cancelAllTasks() {
        baseMapView.cancelAllTasks();
    }

    /**
     * Releases the memory occupied by the preloading area. Calling this method releases some
     * memory if preloading is enabled, but means that the area right outside the visible area has to be
     * fetched again.
     */
    public void clearPreloadingCaches() {
        baseMapView.clearPreloadingCaches();
    }

    /**
     * Releases memory occupied by all caches. Calling this means that everything has to be fetched again,
     * including the visible area.
     */
    public void clearAllCaches() {
        baseMapView.clearAllCaches();
    }

}
