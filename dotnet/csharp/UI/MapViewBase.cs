namespace Carto.Ui {
    using Carto.Components;
    using Carto.Core;
    using Carto.Layers;
    using Carto.Renderers;
    using Carto.Utils;

    /// <summary>
    /// MapView is a platform-specific map class supporting map rendering and interaction.
    /// </summary>
    public partial class MapView : IMapView {
        private const int NativeActionPointer1Down = 0;
        private const int NativeActionPointer2Down = 1;
        private const int NativeActionMove = 2;
        private const int NativeActionCancel = 3;
        private const int NativeActionPointer1Up = 4;
        private const int NativeActionPointer2Up = 5;
        private const int NativeNoCoordinate = -1;
        
        private const int InvalidPointerId = -1;

        private delegate string ReadKeyDelegate(string key);
        private delegate void WriteKeyDelegate(string key, string value);

        private static bool RegisterLicenseInternal(string licenseKey, ReadKeyDelegate readKey, WriteKeyDelegate writeKey) {
            string oldKey = "license_key_old";
            string newKey = "license_key_new";
            LicenseManagerListener listener = new MapLicenseManagerListener((string updatedLicenseKey) => {
                try {
                    writeKey(newKey, updatedLicenseKey);
                }
                catch (System.Exception e) {
                    Carto.Utils.Log.Error("MapView.RegisterLicense: Failed to save license key: " + e);
                }
            });
            string newLicenseKey = null;
            try {
                string oldLicenseKey = readKey(oldKey);
                if (oldLicenseKey != null && oldLicenseKey != licenseKey) {
                    newLicenseKey = readKey(newKey);
                }
            }
            catch (System.Exception e) {
                Carto.Utils.Log.Error("MapView.RegisterLicense: Failed to read license key: " + e);
            }
            return BaseMapView.RegisterLicense(newLicenseKey != null ? newLicenseKey : licenseKey, listener);
        }

        /// <summary>
        /// Internal native map view object. Not part of the public API.
        /// </summary>
        protected BaseMapView _baseMapView;
        
        /// <summary>
        /// The map event listener property. May be null.
        /// </summary>
        public MapEventListener MapEventListener {
            set {
                _baseMapView.SetMapEventListener(value);
            }
            get {
                return _baseMapView.GetMapEventListener();
            }
        }

        /// <summary>
        /// The Layers property, that can be used for adding and removing map layers.
        /// </summary>
        public Layers Layers {
            get {
                return _baseMapView.GetLayers();
            }
        }

        /// <summary>
        /// The Options property, that can be used for modifying various map options.
        /// </summary>
        public Options Options {
            get {
                return _baseMapView.GetOptions();
            }
        }

        /// <summary>
        /// The MapView property, that can be used for controlling rendering options.
        /// </summary>
        public MapRenderer MapRenderer {
            get {
                return _baseMapView.GetMapRenderer();
            }
        }

        /// <summary>
        /// The position that the camera is currently looking at.
        /// </summary>
        public MapPos FocusPos {
            set {
                _baseMapView.SetFocusPos(value, 0.0f);
            }
            get {
                return _baseMapView.GetFocusPos();
            }
        }

        /// <summary>
        /// The map rotation in degrees. 0 means looking north, 90 means west, -90 means east and 180 means south.
        /// </summary>
        public float MapRotation {
            set {
                _baseMapView.SetRotation(value, 0.0f);
            }
            get {
                return _baseMapView.GetRotation();
            }
        }

        /// <summary>
        /// The map tilt angle property in degrees. 0 means looking directly at the horizon, 90 means looking directly down.
        /// </summary>
        public float Tilt {
            set {
                _baseMapView.SetTilt(value, 0.0f);
            }
            get {
                return _baseMapView.GetTilt();
            }
        }

        /// <summary>
        /// The map zoom level property. The value returned is never negative, 0 means absolutely zoomed out and all other
        /// values describe some level of zoom.
        /// </summary>
        public float Zoom {
            set {
                _baseMapView.SetZoom(value, 0.0f);
            }
            get {
                return _baseMapView.GetZoom();
            }
        }

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
        public void Pan(MapVec deltaPos, float durationSeconds) {
            _baseMapView.Pan(deltaPos, durationSeconds);
        }

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
        public void SetFocusPos(MapPos pos, float durationSeconds) {
            _baseMapView.SetFocusPos(pos, durationSeconds);
        }

        /// <summary>
        /// Rotates the view relative to the current rotation value. Positive values rotate clockwise, negative values counterclockwise.
        /// The new calculated rotation value will be wrapped to the range of (-180 .. 180]. Rotations are ignored if Options::setRotatable 
        /// is set to false.
        /// 
        /// If durationSeconds &gt; 0 the rotating operation will be animated over time. If the previous rotating animation has not
        /// finished by the time this method is called, it will be stopped.
        /// 
        /// </summary>
        /// <param name="deltaAngle">The delta angle value in degrees.</param>
        /// <param name="durationSeconds">The duration in which the zooming operation will be completed in seconds.</param>
        public void Rotate(float deltaAngle, float durationSeconds) {
            _baseMapView.Rotate(deltaAngle, durationSeconds);
        }

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
        public void Rotate(float deltaAngle, MapPos targetPos, float durationSeconds) {
            _baseMapView.Rotate(deltaAngle, targetPos, durationSeconds);
        }

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
        public void SetMapRotation(float angle, float durationSeconds) {
            _baseMapView.SetRotation(angle, durationSeconds);
        }

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
        public void SetMapRotation(float angle, MapPos targetPos, float durationSeconds) {
            _baseMapView.SetRotation(angle, targetPos, durationSeconds);
        }

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
        public void SetRelativeTilt(float deltaTilt, float durationSeconds) {
            _baseMapView.Tilt(deltaTilt, durationSeconds);
        }

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
        public void SetTilt(float tilt, float durationSeconds) {
            _baseMapView.SetTilt(tilt, durationSeconds);
        }

        /// <summary>
        /// Zooms the view relative to the current zoom value. Positive values zoom in, negative values zoom out.
        /// The new calculated zoom value will be clamped to the range of [0 .. 24] and to the range set by Options::setZoomRange.
        /// 
        /// If durationSeconds &gt; 0 the zooming operation will be animated over time. If the previous zooming animation has not
        /// finished by the time this method is called, it will be stopped.
        /// </summary>
        /// <param name="deltaZoom">The delta zoom value.</param>
        /// <param name="durationSeconds">The duration in which the zooming operation will be completed in seconds.</param>
        public void SetRelativeZoom(float deltaZoom, float durationSeconds) {
            _baseMapView.Zoom(deltaZoom, durationSeconds);
        }

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
        public void SetRelativeZoom(float deltaZoom, MapPos targetPos, float durationSeconds) {
            _baseMapView.Zoom(deltaZoom, targetPos, durationSeconds);
        }

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
        public void SetZoom(float zoom, float durationSeconds) {
            _baseMapView.SetZoom(zoom, durationSeconds);
        }

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
        public void SetZoom(float zoom, MapPos targetPos, float durationSeconds) {
            _baseMapView.SetZoom(zoom, targetPos, durationSeconds);
        }

        /// <summary>
        /// Animate the view parameters (focus position, tilt, rotation, zoom) so that the specified bounding box becomes fully visible.
        /// This method does not work before the screen size is set.
        /// </summary>
        /// <param name="mapBounds">The bounding box on the map to be made visible in the base projection's coordinate system.</param>
        /// <param name="screenBounds">The screen bounding box where to fit the map bounding box.</param>
        /// <param name="integerZoom">If true, then closest integer zoom level will be used. If false, exact fractional zoom level will be used.</param>
        /// <param name="durationSeconds">The duration in which the operation will be completed in seconds.</param>
        public void MoveToFitBounds(MapBounds mapBounds, ScreenBounds screenBounds, bool integerZoom, float durationSeconds) {
            _baseMapView.MoveToFitBounds(mapBounds, screenBounds, integerZoom, durationSeconds);
        }

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
        public void MoveToFitBounds(MapBounds mapBounds, ScreenBounds screenBounds, bool integerZoom, bool resetRotation, bool resetTilt, float durationSeconds) {
            _baseMapView.MoveToFitBounds(mapBounds, screenBounds, integerZoom, resetRotation, resetTilt, durationSeconds);
        }
        
        /// <summary>
        /// Calculates the map position corresponding to a screen position, using the current view parameters.
        /// </summary>
        /// <param name="screenPos">The screen position.</param>
        /// <returns>The calculated map position in base projection coordinate system.</returns>
        public MapPos ScreenToMap(ScreenPos screenPos) {
            return _baseMapView.ScreenToMap(screenPos);
        }

        /// <summary>
        /// Calculates the screen position corresponding to a map position, using the current view parameters.
        /// </summary>
        /// <param name="mapPos">The map position in base projection coordinate system.</param>
        /// <returns>The calculated screen position.</returns>
        public ScreenPos MapToScreen(MapPos mapPos) {
            return _baseMapView.MapToScreen(mapPos);
        }

        /// <summary>
        /// Cancels all qued tasks such as tile and vector data fetches. Tasks that have already started
        /// may continue until they finish. Tasks that are added after this method call are not affected.
        /// </summary>
        public void CancelAllTasks() {
            _baseMapView.CancelAllTasks();
        }

        /// <summary>
        /// Releases the memory occupied by the preloading area. Calling this method releases some
        /// memory if preloading is enabled, but means that the area right outside the visible area has to be
        /// fetched again.
        /// </summary>
        public void ClearPreloadingCaches() {
            _baseMapView.ClearPreloadingCaches();
        }

        /// <summary>
        /// Releases memory occupied by all caches. Calling this means that everything has to be fetched again,
        /// including the visible area.
        /// </summary>
        public void ClearAllCaches() {
            _baseMapView.ClearAllCaches();	
        }
    }
}
