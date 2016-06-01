namespace Carto.Ui {
    using Android.Content;
    using Android.Content.Res;
    using Android.Preferences;
    using Android.Opengl;
    using Android.Util;
    using Android.Views;
    using Carto.Renderers;
    using Carto.Utils;

    /// <summary>
    /// MapView is an Android-specific view class supporting map rendering and interaction.
    /// </summary>
    public partial class MapView : GLSurfaceView {
        private static AssetManager _assetManager;
        private MapRedrawRequestListener _redrawRequestListener;
        private BaseMapViewRenderer _baseMapViewRenderer;
        private ConfigChooser _configChooser;		
        private int _pointer1Id = InvalidPointerId;
        private int _pointer2Id = InvalidPointerId;

        static MapView() {
            AndroidUtils.AttachJVM (Android.Runtime.JNIEnv.Handle);
        }

        /// <summary>
        /// Registers the SDK license. This class method and must be called before
        /// creating any actual MapView instances.
        /// </summary>
        /// <param name="licenseKey">The license string provided for this application.</param>
        /// <param name="context">Application context for the license.</param>
        /// <returns>True if license is valid, false if not.</returns>
        public static bool RegisterLicense(string licenseKey, Context context) {
            // Connect context info and assets manager to native part
            AndroidUtils.SetContext (context);
            if (_assetManager == null) {
                _assetManager = context.ApplicationContext.Assets;
                AssetUtils.SetAssetManagerPointer(_assetManager);
            }

            ISharedPreferences prefs = context.GetSharedPreferences(context.PackageName + "_carto_mobile_sdk1_preferences", FileCreationMode.Private);
            ReadKeyDelegate readKey = (string key) => { return prefs.GetString(key, null); };
            WriteKeyDelegate writeKey = (string key, string value) => { prefs.Edit().PutString(key, value); };
            return RegisterLicenseInternal(licenseKey, readKey, writeKey);
        }
        
        /// <summary>
        /// Constructs a new MapView object from a context object.
        /// </summary>
        /// <param name="context">The context object.</param>
        public MapView(Context context) : this(context, null) {
        }

        /// <summary>
        /// Constructs a new MapView object from a context object and attributes.
        /// </summary>
        /// <param name="context">The context object.</param>
        /// <param name="attrs">The attributes.</param>
        public MapView(Context context, IAttributeSet attrs) : base(context, attrs) {
            // Connect context info and assets manager to native part
            AndroidUtils.SetContext (context);
            if (_assetManager == null) {
                Carto.Utils.Log.Warn("MapView: MapView created before MapView.RegisterLicense is called");

                _assetManager = context.ApplicationContext.Assets;
                AssetUtils.SetAssetManagerPointer(_assetManager);
            }

            // Create base map view, attach redraw listener
            _baseMapView = new BaseMapView();
            _baseMapView.GetOptions().DPI = (int)Resources.DisplayMetrics.DensityDpi;

            _redrawRequestListener = new MapRedrawRequestListener(this);
            _baseMapView.SetRedrawRequestListener(_redrawRequestListener);

            // Create base map renderer and EGL configuration chooser
            _baseMapViewRenderer = new BaseMapViewRenderer(_baseMapView);

            _configChooser = new ConfigChooser();

            try {
                System.Reflection.PropertyInfo prop = typeof(GLSurfaceView).GetProperty("PreserveEGLContextOnPause");
                prop.SetValue(this, true);
            } catch (System.Exception) {
                Carto.Utils.Log.Info("MapView: Preserving EGL context on pause is not possible");
            }

            SetEGLContextClientVersion(2);
            SetEGLConfigChooser(_configChooser);
            SetRenderer(_baseMapViewRenderer);
            RenderMode = Rendermode.WhenDirty;
        }
        
        ~MapView() {
            Dispose(true);
        }

        protected override void Dispose(bool disposing) {
            lock (this) {
                // Detach objects
                if (_baseMapViewRenderer != null) {
                    _baseMapViewRenderer.Detach();
                }
                if (_redrawRequestListener != null) {
                    _redrawRequestListener.Detach();
                }
                if (_baseMapView != null) {
                    _baseMapView.SetRedrawRequestListener(null);
                }

                // Dispose objects
                if (_configChooser != null) {
                    _configChooser.Dispose();
                    _configChooser = null;
                }
                if (_baseMapViewRenderer != null) {
                    _baseMapViewRenderer.Dispose(); // allow the Java object to be collected later
                    _baseMapViewRenderer = null;
                }
                if (_baseMapView != null) {
                    _baseMapView.Dispose();
                    _baseMapView = null;
                }
                if (_redrawRequestListener != null) {
                    _redrawRequestListener.Dispose();
                    _redrawRequestListener = null;
                }
            }
            base.Dispose(disposing);
        }

        public override bool OnTouchEvent(MotionEvent motionEvent) {
            lock (this) {
                if (_baseMapView == null) {
                    return false;
                }

                try {
                    int pointer1Index;
                    int pointer2Index;
                    switch (motionEvent.ActionMasked) {
                    case MotionEventActions.Down:
                        pointer1Index = motionEvent.ActionIndex;
                        _pointer1Id = motionEvent.GetPointerId(pointer1Index);
                        _baseMapView.OnInputEvent(NativeActionPointer1Down, 
                            motionEvent.GetX(pointer1Index), motionEvent.GetY(pointer1Index), 
                            NativeNoCoordinate, NativeNoCoordinate);
                        break;
                    case MotionEventActions.PointerDown:
                        if (motionEvent.PointerCount == 2) {
                            // Check which pointer to use
                            if (_pointer1Id != InvalidPointerId) {
                                pointer1Index = motionEvent.FindPointerIndex(_pointer1Id);
                                pointer2Index = motionEvent.ActionIndex;
                                _pointer2Id = motionEvent.GetPointerId(motionEvent.ActionIndex);
                            } else if (_pointer2Id != InvalidPointerId) {
                                pointer2Index = motionEvent.FindPointerIndex(_pointer2Id);
                                pointer1Index = motionEvent.ActionIndex;
                                _pointer1Id = motionEvent.GetPointerId(motionEvent.ActionIndex);
                            } else {
                                break;
                            }
                            _baseMapView.OnInputEvent(NativeActionPointer2Down, 
                                motionEvent.GetX(pointer1Index), motionEvent.GetY(pointer1Index),
                                motionEvent.GetX(pointer2Index), motionEvent.GetY(pointer2Index));
                        }
                        break;
                    case MotionEventActions.Move:
                        if (_pointer1Id != InvalidPointerId && _pointer2Id == InvalidPointerId) {
                            pointer1Index = motionEvent.FindPointerIndex(_pointer1Id);
                            _baseMapView.OnInputEvent(NativeActionMove, 
                                motionEvent.GetX(pointer1Index), motionEvent.GetY(pointer1Index), 
                                NativeNoCoordinate, NativeNoCoordinate);
                        } else if (_pointer1Id != InvalidPointerId && _pointer2Id != InvalidPointerId) {
                            pointer1Index = motionEvent.FindPointerIndex(_pointer1Id);
                            pointer2Index = motionEvent.FindPointerIndex(_pointer2Id);
                            _baseMapView.OnInputEvent(NativeActionMove, 
                                motionEvent.GetX(pointer1Index), motionEvent.GetY(pointer1Index), 
                                motionEvent.GetX(pointer2Index), motionEvent.GetY(pointer2Index));
                        }
                        break;
                    case MotionEventActions.Cancel:
                        _baseMapView.OnInputEvent(NativeActionCancel, 
                            NativeNoCoordinate, NativeNoCoordinate, 
                            NativeNoCoordinate, NativeNoCoordinate);
                        break;
                    case MotionEventActions.Up:
                    case MotionEventActions.PointerUp:
                        int pointerIndex = motionEvent.ActionIndex;
                        int pointerId = motionEvent.GetPointerId(pointerIndex);
                        // Single pointer
                        if (_pointer1Id == pointerId && _pointer2Id == InvalidPointerId) {
                            pointer1Index = motionEvent.FindPointerIndex(_pointer1Id);
                            _baseMapView.OnInputEvent(NativeActionPointer1Up, 
                                motionEvent.GetX(pointer1Index), motionEvent.GetY(pointer1Index), 
                                NativeNoCoordinate, NativeNoCoordinate);
                            _pointer1Id = InvalidPointerId;
                            // Dual pointer, first pointer up
                        } else if (_pointer1Id == pointerId) {
                            pointer1Index = motionEvent.FindPointerIndex(_pointer1Id);
                            pointer2Index = motionEvent.FindPointerIndex(_pointer2Id);
                            _baseMapView.OnInputEvent(NativeActionPointer1Up, 
                                motionEvent.GetX(pointer1Index), motionEvent.GetY(pointer1Index), 
                                motionEvent.GetX(pointer2Index), motionEvent.GetY(pointer2Index));
                            _pointer1Id = _pointer2Id;
                            _pointer2Id = InvalidPointerId;
                            // Dual pointer, second finger up
                        } else if (_pointer2Id == pointerId) {
                            pointer1Index = motionEvent.FindPointerIndex(_pointer1Id);
                            pointer2Index = motionEvent.FindPointerIndex(_pointer2Id);
                            _baseMapView.OnInputEvent(NativeActionPointer2Up, 
                                motionEvent.GetX(pointer1Index), motionEvent.GetY(pointer1Index), 
                                motionEvent.GetX(pointer2Index), motionEvent.GetY(pointer2Index));
                            _pointer2Id = InvalidPointerId;
                        }
                        break;
                    }
                }
                catch (Java.Lang.Exception e) {
                    Carto.Utils.Log.Error("MapView.OnTouchEvent: Java exception: " + e);
                }
                catch (System.Exception e) {
                    Carto.Utils.Log.Error("MapView.OnTouchEvent: " + e);
                }
                return true;
            }
        }
    }
}
