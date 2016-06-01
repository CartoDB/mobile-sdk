namespace Carto.Ui
{
    using Carto.Core;
    using Carto.Components;
    using Carto.Layers;
    using Carto.Renderers;

    /// <summary>
    /// IMapView is a platform independent interface to the
    /// platform-specific map rendering and interaction class.
    /// </summary>
    public interface IMapView
    {
        /// <summary>
        /// The map event listener property. May be null.
        /// </summary>
        MapEventListener MapEventListener { set; get; }

        /// <summary>
        /// The Layers property, that can be used for adding and removing map layers.
        /// </summary>
        Layers Layers { get; }

        /// <summary>
        /// The Options property, that can be used for modifying various map options.
        /// </summary>
        Options Options { get; }

        /// <summary>
        /// The MapRenderer property, that can be used for controlling rendering operations.
        /// </summary>
        MapRenderer MapRenderer { get; }

        /// <summary>
        /// The position that the camera is currently looking at.
        /// </summary>
        MapPos FocusPos { set; get; }

        /// <summary>
        /// The map rotation in degrees. 0 means looking north, 90 means west, -90 means east and 180 means south.
        /// </summary>
        float MapRotation { set; get; }

        /// <summary>
        /// The map tilt angle property in degrees. 0 means looking directly at the horizon, 90 means looking directly down.
        /// </summary>
        float Tilt { set; get; }

        /// <summary>
        /// The map zoom level property. The value returned is never negative, 0 means absolutely zoomed out and all other
        /// values describe some level of zoom.
        /// </summary>
        float Zoom { set; get; }

        /// <summary>
        /// Pans the view relative to the current focus position. The deltaPos vector is expected to be in 
        /// the coordinate system of the base projection. The new calculated focus position will be clamped to
        /// the world bounds and to the bounds set by Options::setPanBounds.
        /// 
        /// If durationSeconds &gt; 0 the panning operation will be animated over time. If the previous panning animation has not
        /// finished by the time this method is called, it will be stopped.
        /// </summary>
        /// <param name="deltaPos">The coordinate difference the map should be moved by.</param>
        /// <param name="durationSeconds">The duration in which the tilting operation will be completed in seconds.</param>
        void Pan(MapVec deltaPos, float durationSeconds);

        /// <summary>
        /// Sets the new absolute focus position. The new focus position is expected to be in
        /// the coordinate system of the base projection. The new focus position will be clamped to
        /// the world bounds and to the bounds set by Options::setPanBounds.
        /// 
        /// If durationSeconds &gt; 0 the panning operation will be animated over time. If the previous panning animation has not
        /// finished by the time this method is called, it will be stopped.
        /// </summary>
        /// <param name="pos">The new focus point position in base coordinate system.</param>
        /// <param name="durationSeconds">The duration in which the tilting operation will be completed in seconds.</param>
        void SetFocusPos(MapPos pos, float durationSeconds);

        /// <summary>
        /// Rotates the view relative to the current rotation value. Positive values rotate clockwise, negative values counterclockwise.
        /// The new calculated rotation value will be wrapped to the range of (-180 .. 180]. Rotations are ignored if Options::setRotatable 
        /// is set to false.
        /// 
        /// If durationSeconds &gt; 0 the rotating operation will be animated over time. If the previous rotating animation has not
        /// finished by the time this method is called, it will be stopped.
        /// </summary>
        /// <param name="deltaAngle">The delta angle value in degrees.</param>
        /// <param name="durationSeconds">The duration in which the zooming operation will be completed in seconds.</param>
        void Rotate(float deltaAngle, float durationSeconds);

        /// <summary>
        /// Rotates the view relative to the current rotation value. Positive values rotate clockwise, negative values counterclockwise.
        /// The new calculated rotation value will be wrapped to the range of (-180 .. 180]. Rotations are ignored if Options::setRotatable
        /// is set to false.
        /// 
        /// Rotating is done around the specified target position, keeping it at the same location on the screen.
        /// 
        /// If durationSeconds &gt; 0 the rotating operation will be animated over time. If the previous rotating animation has not
        /// finished by the time this method is called, it will be stopped.
        /// </summary>
        /// <param name="deltaAngle">The delta angle value in degrees.</param>
        /// <param name="targetPos">The zooming target position in the coordinate system of the base projection.</param>
        /// <param name="durationSeconds">The duration in which the zooming operation will be completed in seconds.</param>
        void Rotate(float deltaAngle, MapPos targetPos, float durationSeconds);

        /// <summary>
        /// Sets the new absolute rotation value. 0 means look north, 90 means west, -90 means east and 180 means south.
        /// The rotation value will be wrapped to the range of (-180 .. 180]. Rotations are ignored if Options::setRotatable
        /// is set to false.
        /// 
        /// If durationSeconds &gt; 0 the rotating operation will be animated over time. If the previous rotating animation has not
        /// finished by the time this method is called, it will be stopped.
        /// </summary>
        /// <param name="angle">The new absolute rotation angle value in degrees.</param>
        /// <param name="durationSeconds">The duration in which the zooming operation will be completed in seconds.</param>
        void SetMapRotation(float angle, float durationSeconds);

        /// <summary>
        /// Sets the new absolute rotation value. 0 means look north, 90 means west, -90 means east and 180 means south.
        /// The rotation value will be wrapped to the range of (-180 .. 180]. Rotations are ignored if Options::setRotatable 
        /// is set to false.
        /// 
        /// Rotating is done around the specified target position, keeping it at the same location on the screen.
        /// 
        /// If durationSeconds &gt; 0 the rotating operation will be animated over time. If the previous rotating animation has not
        /// finished by the time this method is called, it will be stopped.
        /// </summary>
        /// <param name="angle">The new absolute rotation angle value in degrees.</param>
        /// <param name="targetPos">The zooming target position in the coordinate system of the base projection.</param>
        /// <param name="durationSeconds">The duration in which the zooming operation will be completed in seconds.</param>
        void SetMapRotation(float angle, MapPos targetPos, float durationSeconds);

        /// <summary>
        /// Tilts the view relative to the current tilt value. Positive values tilt the view down towards the map, 
        /// negative values tilt the view up towards the horizon. The new calculated tilt value will be clamped to
        /// the range of [30 .. 90] and to the range set by Options::setZoomRange.
        /// 
        /// If durationSeconds &gt; 0 the tilting operation will be animated over time. If the previous tilting animation has not
        /// finished by the time this method is called, it will be stopped.
        /// </summary>
        /// <param name="deltaTilt">The number of degrees the camera should be tilted by.</param>
        /// <param name="durationSeconds">The duration in which the tilting operation will be completed in seconds.</param>
        void SetRelativeTilt(float deltaTilt, float durationSeconds);

        /// <summary>
        /// Sets the new absolute tilt value. 0 means look directly at the horizon, 90 means look directly down. The
        /// minimum tilt angle is 30 degrees and the maximum is 90 degrees. The tilt value can be further constrained
        /// by the Options::setTiltRange method. Values exceeding these ranges will be clamped.
        /// 
        /// If durationSeconds &gt; 0 the tilting operation will be animated over time. If the previous tilting animation has not
        /// finished by the time this method is called, it will be stopped.
        /// </summary>
        /// <param name="tilt">The new absolute tilt value in degrees.</param>
        /// <param name="durationSeconds">The duration in which the tilting operation will be completed in seconds.</param>
        void SetTilt(float tilt, float durationSeconds);

        /// <summary>
        /// Zooms the view relative to the current zoom value. Positive values zoom in, negative values zoom out.
        /// The new calculated zoom value will be clamped to the range of [0 .. 24] and to the range set by Options::setZoomRange.
        /// 
        /// If durationSeconds &gt; 0 the zooming operation will be animated over time. If the previous zooming animation has not
        /// finished by the time this method is called, it will be stopped.
        /// </summary>
        /// <param name="deltaZoom">The delta zoom value.</param>
        /// <param name="durationSeconds">The duration in which the zooming operation will be completed in seconds.</param>
        void SetRelativeZoom(float deltaZoom, float durationSeconds);

        /// <summary>
        /// Zooms the view relative to the current zoom value. Positive values zoom in, negative values zoom out.
        /// The new calculated zoom value will be clamped to the range of [0 .. 24] and to the range set by Options::setZoomRange.
        /// 
        /// Zooming is done towards the specified target position, keeping it at the same location on the screen.
        /// 
        /// If durationSeconds &gt; 0 the zooming operation will be animated over time. If the previous zooming animation has not
        /// finished by the time this method is called, it will be stopped.
        /// </summary>
        /// <param name="deltaZoom">The delta zoom value.</param>
        /// <param name="targetPos">The zooming target position in the coordinate system of the base projection.</param>
        /// <param name="durationSeconds">The duration in which the zooming operation will be completed in seconds.</param>
        void SetRelativeZoom(float deltaZoom, MapPos targetPos, float durationSeconds);

        /// <summary>
        /// Sets the new absolute zoom value. The minimum zoom value is 0, which means absolutely zoomed out and the maximum
        /// zoom value is 24. The zoom value can be further constrained by the Options::setZoomRange method. Values
        /// exceeding these ranges will be clamped. 
        /// 
        /// If durationSeconds &gt; 0 the zooming operation will be animated over time. If the previous zooming animation has not
        /// finished by the time this method is called, it will be stopped.
        /// </summary>
        /// <param name="zoom">The new absolute zoom value.</param>
        /// <param name="durationSeconds">The duration in which the zooming operation will be completed in seconds.</param>
        void SetZoom(float zoom, float durationSeconds);

        /// <summary>
        /// Sets the new absolute zoom value. The minimum zoom value is 0, which means absolutely zoomed out and the maximum 
        /// zoom value is 24. The zoom value can be further constrained by the Options::setZoomRange method. Values 
        /// exceeding these ranges will be clamped.
        /// 
        /// Zooming is done towards the specified target position, keeping it at the same location on the screen.
        /// 
        /// If durationSeconds &gt; 0, the zooming operation will be animated over time. If the previous zooming animation has not
        /// finished by the time this method is called, it will be stopped.
        /// </summary>
        /// <param name="zoom">The new absolute zoom value.</param>
        /// <param name="targetPos">The zooming target position in the coordinate system of the base projection.</param>
        /// <param name="durationSeconds">The duration in which the zooming operation will be completed in seconds.</param>
        void SetZoom(float zoom, MapPos targetPos, float durationSeconds);

        /// <summary>
        /// Animate the view parameters (focus position, tilt, rotation, zoom) so that the specified bounding box becomes fully visible.
        /// This method does not work before the screen size is set.
        /// </summary>
        /// <param name="mapBounds">The bounding box on the map to be made visible in the base projection's coordinate system.</param>
        /// <param name="screenBounds">The screen bounding box where to fit the map bounding box.</param>
        /// <param name="integerZoom">If true, then closest integer zoom level will be used. If false, exact fractional zoom level will be used.</param>
        /// <param name="durationSeconds">The duration in which the operation will be completed in seconds.</param>
        void MoveToFitBounds(MapBounds mapBounds, ScreenBounds screenBounds, bool integerZoom, float durationSeconds);

        /// <summary>
        /// Animate the view parameters (focus position, tilt, rotation, zoom) so that the specified bounding box becomes fully visible.
        /// Also supports resetting the tilt and rotation angles over the course of the animation.
        /// This method does not work before the screen size is set.
        /// </summary>
        /// <param name="mapBounds">The bounding box on the map to be made visible in the base projection's coordinate system.</param>
        /// <param name="screenBounds">The screen bounding box where to fit the map bounding box.</param>
        /// <param name="integerZoom">If true, then closest integer zoom level will be used. If false, exact fractional zoom level will be used.</param>
        /// <param name="resetTilt">If true, view will be untilted. If false, current tilt will be kept.</param>
        /// <param name="resetRotation">If true, rotation will be reset. If false, current rotation will be kept.</param>
        /// <param name="durationSeconds">The duration in which the operation will be completed in seconds.</param>
        void MoveToFitBounds(MapBounds mapBounds, ScreenBounds screenBounds, bool integerZoom, bool resetRotation, bool resetTilt, float durationSeconds);

        /// <summary>
        /// Calculates the map position corresponding to a screen position, using the current view parameters.
        /// </summary>
        /// <param name="screenPos">The screen position.</param>
        /// <returns>The calculated map position in base projection coordinate system.</returns>
        MapPos ScreenToMap(ScreenPos screenPos);

        /// <summary>
        /// Calculates the screen position corresponding to a map position, using the current view parameters.
        /// </summary>
        /// <param name="mapPos">The map position in base projection coordinate system.</param>
        /// <returns>The calculated screen position.</returns>
        ScreenPos MapToScreen(MapPos mapPos);

        /// <summary>
        /// Cancels all qued tasks such as tile and vector data fetches. Tasks that have already started
        /// may continue until they finish. Tasks that are added after this method call are not affected.
        /// </summary>
        void CancelAllTasks();

        /// <summary>
        /// Releases the memory occupied by the preloading area. Calling this method releases some
        /// memory if preloading is enabled, but means that the area right outside the visible area has to be
        /// fetched again.
        /// </summary>
        void ClearPreloadingCaches();

        /// <summary>
        /// Releases memory occupied by all caches. Calling this means that everything has to be fetched again,
        /// including the visible area.
        /// </summary>
        void ClearAllCaches();
    }
}

