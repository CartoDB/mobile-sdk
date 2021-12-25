namespace Carto.Ui {
    using System;
    using UIKit;
    using GLKit;
    using OpenGLES;
    using CoreFoundation;
    using CoreGraphics;
    using Foundation;
    using Carto.Components;
    using Carto.Core;
    using Carto.Layers;
    using Carto.Renderers;
    using Carto.Utils;

    /// <summary>
    /// MapView is an iOS-specific view class supporting map rendering and interaction.
    /// </summary>
    [Foundation.Register("MapView")]
    public partial class MapView : GLKView {
        private bool _active = false;
        private bool _surfaceCreated = false;
        private CGSize _activeDrawableSize = new CGSize(0, 0);
        private float _scale = 1;
        private NSObject _willResignActiveNotificationObserver;
        private NSObject _didEnterBackgroundNotificationObserver;
        private NSObject _willEnterForegroundNotificationObserver;
        private MapRedrawRequestListener _redrawRequestListener;

        private UITouch _pointer1 = null;
        private UITouch _pointer2 = null;

        static MapView() {
            IOSUtils.InitializeLog();
        }

        /// <summary>
        /// Registers the SDK license. This class method and must be called before
        /// creating any actual MapView instances.
        /// </summary>
        /// <param name="licenseKey">The license string provided for this application.</param>
        /// <returns>True if license is valid, false if not.</returns>
        public static bool RegisterLicense(string licenseKey) {
            ReadKeyDelegate readKey = (string key) => { return NSUserDefaults.StandardUserDefaults.StringForKey(key); };
            WriteKeyDelegate writeKey = (string key, string value) => { NSUserDefaults.StandardUserDefaults.SetString(value, key); NSUserDefaults.StandardUserDefaults.Synchronize(); };
            return RegisterLicenseInternal(licenseKey, readKey, writeKey);
        }

        /// <summary>
        /// Constructs a new MapView object.
        /// </summary>
        /// <param name="rect">The view rectangle.</param>
        public MapView(CGRect rect) : base(rect) {
            InitBase();
        }

        /// <summary>
        /// Constructs a new MapView object.
        /// </summary>
        public MapView() : base() {
            InitBase();
        }

        /// <summary>
        /// Constructs a new MapView object from Interface Builder.
        /// </summary>
        /// <param name="ptr">The Interface Builder context pointer</param>
        public MapView(IntPtr ptr) : base(ptr) {
            InitBase();
        }

        private void InitBase() {
            _active = false;
            _surfaceCreated = false;
            _activeDrawableSize = new CGSize(0, 0);
            _scale = (float) UIScreen.MainScreen.NativeScale;
            ContentScaleFactor = _scale;

            _baseMapView = new BaseMapView();
            _baseMapView.GetOptions().DPI = 160 * _scale;

            _redrawRequestListener = new MapRedrawRequestListener(this);
            _baseMapView.SetRedrawRequestListener(_redrawRequestListener);

            if (Window != null) {
                InitContext();
                _active = true;
                SetNeedsDisplay();
            }

            _willResignActiveNotificationObserver = NSNotificationCenter.DefaultCenter.AddObserver(UIApplication.WillResignActiveNotification, OnAppWillResignActive, null);
            _didEnterBackgroundNotificationObserver = NSNotificationCenter.DefaultCenter.AddObserver(UIApplication.DidEnterBackgroundNotification, OnAppDidEnterBackground, null);
            _willEnterForegroundNotificationObserver = NSNotificationCenter.DefaultCenter.AddObserver(UIApplication.WillEnterForegroundNotification, OnAppWillEnterForeground, null);
        }

        private void InitContext() {
            var context = new EAGLContext(EAGLRenderingAPI.OpenGLES2);
            if (context == null) {
                Log.Fatal("MapView.InitContext: Failed to create OpenGL ES 2.0 context");
            }

            Context = context;
            MultipleTouchEnabled = true;
            DrawableColorFormat = GLKViewDrawableColorFormat.RGBA8888;
            DrawableDepthFormat = GLKViewDrawableDepthFormat.Format24;
            DrawableMultisample = GLKViewDrawableMultisample.None;
            DrawableStencilFormat = GLKViewDrawableStencilFormat.Format8;
        }

        public override void WillMoveToWindow(UIWindow newWindow) {
            base.WillMoveToWindow(newWindow);

            if (newWindow == null) {
                Log.Info("MapView.WillMoveToWindow: null");
            } else {
                Log.Info("MapView.WillMoveToWindow: nonnull");
                lock (this) {
                    if (!_active) {
                        InitContext();
                        if (EAGLContext.CurrentContext == null) {
                            EAGLContext.SetCurrentContext(Context);
                        }
                        _active = true;
                        _surfaceCreated = false;
                    }
                }
            }
        }

        public override void MovedToWindow() {
            base.MovedToWindow();

            if (Window == null) {
                Log.Info("MapView.MovedToWindow: null");

                lock (this) {
                    if (_active) {
                        _baseMapView.OnSurfaceDestroyed();
                        if (EAGLContext.CurrentContext == Context) {
                            EAGLContext.SetCurrentContext(null);
                        }
                        _active = false;
                        _surfaceCreated = false;
                    }
                }
            } else {
                Log.Info("MapView.MovedToWindow: nonnull");

                SetNeedsDisplay();
            }
        }

        public override void LayoutSubviews() {
            base.LayoutSubviews();

            SetNeedsDisplay();
        }

        public override void Draw(CGRect rect) {
            lock (this) {
                if (_active) {
                    if (!_surfaceCreated) {
                        _baseMapView.OnSurfaceCreated();
                        _activeDrawableSize = new CGSize(0, 0);
                        _surfaceCreated = true;
                    }

                    float drawableWidth = DrawableWidth;
                    float drawableHeight = DrawableHeight;
                    if (_activeDrawableSize.Width != drawableWidth || _activeDrawableSize.Height != drawableHeight) {
                        _activeDrawableSize = new CGSize(drawableWidth, drawableHeight);
                        _baseMapView.OnSurfaceChanged((int)_activeDrawableSize.Width, (int)_activeDrawableSize.Height);
                    }

                    _baseMapView.OnDrawFrame();
                }
            }
        }

        protected override void Dispose(bool disposing) {
            if (_willResignActiveNotificationObserver != null) {
                NSNotificationCenter.DefaultCenter.RemoveObserver(_willResignActiveNotificationObserver);
                _willResignActiveNotificationObserver = null;
            }
            if (_didEnterBackgroundNotificationObserver != null) {
                NSNotificationCenter.DefaultCenter.RemoveObserver(_didEnterBackgroundNotificationObserver);
                _didEnterBackgroundNotificationObserver = null;
            }
            if (_willEnterForegroundNotificationObserver != null) {
                NSNotificationCenter.DefaultCenter.RemoveObserver(_willEnterForegroundNotificationObserver);
                _willEnterForegroundNotificationObserver = null;
            }

            lock (this) {
                if (_redrawRequestListener != null) {
                    _redrawRequestListener.Detach();
                    _redrawRequestListener = null;
                }
                if (_baseMapView != null) {
                    _baseMapView.OnSurfaceDestroyed();
                    _baseMapView.SetRedrawRequestListener(null);
                    _baseMapView = null;
                    _surfaceCreated = false;
                    _activeDrawableSize = new CGSize(0, 0);
                }
                if (EAGLContext.CurrentContext == Context) {
                    EAGLContext.SetCurrentContext(null);
                }
            }

            base.Dispose(disposing);
        }

        private void OnAppWillResignActive(NSNotification notification) {
            Log.Info("MapView.OnAppWillResignActive");

            lock (this){
                if (_active) {
                    EAGLContext context = EAGLContext.CurrentContext;
                    if (context != Context) {
                        EAGLContext.SetCurrentContext(Context);
                    }

                    _baseMapView.FinishRendering();

                    if (context != Context) {
                        EAGLContext.SetCurrentContext(context);
                    }
                }
                _active = false;
            }
        }

        private void OnAppDidEnterBackground(NSNotification notification) {
            Log.Info("MapView.OnAppDidEnterBackground");

            lock (this) {
                _active = false;
            }
        }

        private void OnAppWillEnterForeground(NSNotification notification) {
            Log.Info("MapView.OnAppWillEnterForeground");

            lock (this) {
                _active = true;
            }

            SetNeedsDisplay();
        }

        public override void TouchesBegan(Foundation.NSSet touches, UIEvent evt) {
            foreach (UITouch pointer in touches) {
                if (_pointer1 == null) {
                    _pointer1 = pointer;
                    CGPoint screenPos1 = _pointer1.LocationInView(this);
                    _baseMapView.OnInputEvent(NativeActionPointer1Down, (float) screenPos1.X * _scale, (float) screenPos1.Y * _scale, NativeNoCoordinate, NativeNoCoordinate);
                    continue;
                }

                if (_pointer2 == null) {
                    _pointer2 = pointer;
                    CGPoint screenPos1 = _pointer1.LocationInView(this);
                    CGPoint screenPos2 = _pointer2.LocationInView(this);
                    _baseMapView.OnInputEvent(NativeActionPointer2Down, (float) screenPos1.X * _scale, (float) screenPos1.Y * _scale, (float) screenPos2.X * _scale, (float) screenPos2.Y * _scale);
                    break;
                }
            }
        }

        public override void TouchesMoved(Foundation.NSSet touches, UIEvent evt) {
            if (_pointer1 != null) {
                CGPoint screenPos1 = _pointer1.LocationInView(this);
                if (_pointer2 != null) {
                    CGPoint screenPos2 = _pointer2.LocationInView(this);
                    _baseMapView.OnInputEvent(NativeActionMove, (float) screenPos1.X * _scale, (float) screenPos1.Y * _scale, (float) screenPos2.X * _scale, (float) screenPos2.Y * _scale);
                } else {
                    _baseMapView.OnInputEvent(NativeActionMove, (float) screenPos1.X * _scale, (float) screenPos1.Y * _scale, NativeNoCoordinate, NativeNoCoordinate);
                }
            }
        }

        public override void TouchesCancelled(Foundation.NSSet touches, UIEvent evt) {
            // Note: should use ACTION_CANCEL here, but Xamarin.Forms uses this
            // for single clicks, so we need to emulate TouchesEnded here actually
            if (_pointer2 != null && touches.Contains(_pointer2)) {
                if (_pointer1 != null && !touches.Contains(_pointer1)) {
                    CGPoint screenPos1 = _pointer1.LocationInView(this);
                    _baseMapView.OnInputEvent(NativeActionPointer2Up, (float) screenPos1.X * _scale, (float) screenPos1.Y * _scale, NativeNoCoordinate, NativeNoCoordinate);
                } else {
                    _baseMapView.OnInputEvent(NativeActionPointer2Up, NativeNoCoordinate, NativeNoCoordinate, NativeNoCoordinate, NativeNoCoordinate);
                }
                _pointer2 = null;
            }

            if (_pointer1 != null && touches.Contains(_pointer1)) {
                if (_pointer2 != null) {
                    CGPoint screenPos2 = _pointer2.LocationInView(this);
                    _baseMapView.OnInputEvent(NativeActionPointer1Up, NativeNoCoordinate, NativeNoCoordinate, (float) screenPos2.X * _scale, (float) screenPos2.Y * _scale);
                    _pointer1 = _pointer2;
                    _pointer2 = null;
                } else {
                    _baseMapView.OnInputEvent(NativeActionPointer1Up, NativeNoCoordinate, NativeNoCoordinate, NativeNoCoordinate, NativeNoCoordinate);
                    _pointer1 = null;
                }
            }
        }

        public override void TouchesEnded(Foundation.NSSet touches, UIEvent evt) {
            if (_pointer2 != null && touches.Contains(_pointer2)) {
                CGPoint screenPos1 = _pointer1.LocationInView(this);
                CGPoint screenPos2 = _pointer2.LocationInView(this);
                _baseMapView.OnInputEvent(NativeActionPointer2Up, (float) screenPos1.X * _scale, (float) screenPos1.Y * _scale, (float) screenPos2.X * _scale, (float) screenPos2.Y * _scale);
                _pointer2 = null;
            }

            if (_pointer1 != null && touches.Contains(_pointer1)) {
                CGPoint screenPos1 = _pointer1.LocationInView(this);
                if (_pointer2 != null) {
                    CGPoint screenPos2 = _pointer2.LocationInView (this);
                    _baseMapView.OnInputEvent(NativeActionPointer1Up, (float) screenPos1.X * _scale, (float) screenPos1.Y * _scale, (float) screenPos2.X * _scale, (float) screenPos2.Y * _scale);
                    _pointer1 = _pointer2;
                    _pointer2 = null;
                } else {
                    _baseMapView.OnInputEvent(NativeActionPointer1Up, (float) screenPos1.X * _scale, (float) screenPos1.Y * _scale, NativeNoCoordinate, NativeNoCoordinate);
                    _pointer1 = null;
                }
            }
        }
    }
}
