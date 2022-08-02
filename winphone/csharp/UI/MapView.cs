namespace Carto.Ui {
    using System;
    using Carto.Renderers;
    using Carto.Utils;
    using Windows.UI.Xaml.Markup;

    /// <summary>
    /// MapView is a Windows Phone specific view class supporting map rendering and interaction.
    /// </summary>
    public partial class MapView : Windows.UI.Xaml.Controls.UserControl, Windows.UI.Xaml.Markup.IComponentConnector {
        private struct PointerState {
            public uint pointerId;
            public Windows.UI.Input.PointerPoint pointerPoint;
        };

        static EGLContextWrapper _eglContext; // context wrapper needs to be shared between all instances

        bool _contentLoaded = false;
        System.Collections.Generic.List<PointerState> _pointerStates = new System.Collections.Generic.List<PointerState>();
        float _resolutionScale = 1.0f;

        Windows.UI.Xaml.Controls.SwapChainPanel _swapChainPanel;
        Windows.Foundation.Size _swapChainPanelSize = new Windows.Foundation.Size(0, 0);
        object _swapChainPanelSizeLock = new object();
        System.Threading.ManualResetEvent _swapChainEvent = new System.Threading.ManualResetEvent(false);

        System.IntPtr _renderSurface;
        volatile bool _renderSurfaceRedrawPending = false;
        object _renderSurfaceLock = new object();
        bool _surfaceCreated = false;
        int _surfaceWidth = -1;
        int _surfaceHeight = -1;
        
        /// <summary>
        /// Registers the SDK license. This class method and must be called before
        /// creating any actual MapView instances.
        /// </summary>
        /// <param name="licenseKey">The license string provided for this application.</param>
        /// <returns>True if license is valid, false if not.</returns>
        public static bool RegisterLicense(string licenseKey) {
            Windows.Storage.ApplicationDataContainer localSettings = Windows.Storage.ApplicationData.Current.LocalSettings;
            ReadKeyDelegate readKey = (string key) => { return localSettings.Values[key] as string; };
            WriteKeyDelegate writeKey = (string key, string value) => { localSettings.Values[key] = value; };
            return RegisterLicenseInternal(licenseKey, readKey, writeKey);
        }

        /// <summary>
        /// Constructs a new MapView object.
        /// </summary>
        public MapView() {
            _baseMapView = new BaseMapView();

            InitializeComponent();

            this.Loaded += new Windows.UI.Xaml.RoutedEventHandler(OnPageLoaded);
            this.Unloaded += new Windows.UI.Xaml.RoutedEventHandler(OnPageUnloaded);

            Windows.UI.Core.CoreWindow window = Windows.UI.Xaml.Window.Current.CoreWindow;
            window.VisibilityChanged += new Windows.Foundation.TypedEventHandler<Windows.UI.Core.CoreWindow, Windows.UI.Core.VisibilityChangedEventArgs > (OnVisibilityChanged);

            _swapChainPanel.LayoutUpdated += new System.EventHandler<object>(OnSwapChainPanelLayoutUpdated);
            _swapChainPanel.SizeChanged += new Windows.UI.Xaml.SizeChangedEventHandler(OnSwapChainPanelSizeChanged);
            _swapChainPanelSize = new Windows.Foundation.Size(_swapChainPanel.RenderSize.Width, _swapChainPanel.RenderSize.Height);

            Windows.Graphics.Display.DisplayInformation dispInfo = Windows.Graphics.Display.DisplayInformation.GetForCurrentView();
            _resolutionScale = (float) dispInfo.RawPixelsPerViewPixel;
            _baseMapView.GetOptions().DPI = (float) dispInfo.RawDpiX;

            _baseMapView.SetRedrawRequestListener(new MapViewRedrawRequestListener(this));
        }

        ~MapView() {
            lock (_renderSurfaceLock) {
                DestroyRenderSurface();
            }
        }

        public void InitializeComponent() {
            if (_eglContext == null) {
                _eglContext = new EGLContextWrapper();
            }

            if (_contentLoaded) {
                return;
            }

            _swapChainPanel = new Windows.UI.Xaml.Controls.SwapChainPanel();
            Content = _swapChainPanel;
            _contentLoaded = true;
        }

        void IComponentConnector.Connect(int connectionId, object target) {
            _contentLoaded = true;
        }

        public void Redraw() {
            if (_renderSurfaceRedrawPending) {
                return;
            }
            _renderSurfaceRedrawPending = true;

            _swapChainPanel.Dispatcher.RunAsync(Windows.UI.Core.CoreDispatcherPriority.Low, new Windows.UI.Core.DispatchedHandler(() => {
                _renderSurfaceRedrawPending = false;

                int panelWidth = 0;
                int panelHeight = 0;
                GetSwapChainPanelSize(out panelWidth, out panelHeight);

                bool redraw = false;
                lock (_renderSurfaceLock) {
                    if (_renderSurface == IntPtr.Zero) {
                        return;
                    }
                    _eglContext.MakeCurrent(_renderSurface);

                    // Signal 'surface created' when needed
                    if (!_surfaceCreated) {
                        _baseMapView.OnSurfaceCreated();
                        _surfaceCreated = true;
                    }

                    // Signal 'surface changed' when needed
                    if (panelWidth != _surfaceWidth || panelHeight != _surfaceHeight) {
                        _baseMapView.OnSurfaceChanged(panelWidth, panelHeight);
                        _surfaceWidth = panelWidth;
                        _surfaceHeight = panelHeight;
                    }

                    // Do actual rendering
                    _baseMapView.OnDrawFrame();

                    // Display rendered image
                    if (!_eglContext.SwapBuffers(_renderSurface)) {
                        DestroyRenderSurface();
                        CreateRenderSurface();
                        redraw = true;
                    }
                }

                if (redraw) {
                    Redraw();
                }
            }));
        }

        void OnPageLoaded(object sender, Windows.UI.Xaml.RoutedEventArgs args) {
            lock (_renderSurfaceLock) {
                CreateRenderSurface();
            }
        }

        void OnPageUnloaded(object sender, Windows.UI.Xaml.RoutedEventArgs args) {
            lock (_renderSurfaceLock) {
                DestroyRenderSurface();
            }
        }

        void OnVisibilityChanged(Windows.UI.Core.CoreWindow sender, Windows.UI.Core.VisibilityChangedEventArgs args) {
            lock (_renderSurfaceLock) {
                if (args.Visible) {
                    CreateRenderSurface();
                }
            }
            Redraw();
        }

        protected override void OnPointerPressed(Windows.UI.Xaml.Input.PointerRoutedEventArgs args) {
            Windows.UI.Input.PointerPoint currentPoint = args.GetCurrentPoint(this);
            for (int i = 0; i < _pointerStates.Count; i++) {
                if (_pointerStates[i].pointerId == currentPoint.PointerId) {
                    _pointerStates.RemoveAt(i);
                }
            }
            if (_pointerStates.Count >= 2) {
                Log.Warn("MapView.OnPointerPressed: More than 2 pointers active");
            }
            PointerState state;
            state.pointerId = currentPoint.PointerId;
            state.pointerPoint = currentPoint;
            _pointerStates.Insert(0, state);
            UpdateInputCoordinates(_pointerStates.Count > 1 ? NativeActionPointer2Down : NativeActionPointer1Down);
            args.Handled = true;
        }

        protected override void OnPointerMoved(Windows.UI.Xaml.Input.PointerRoutedEventArgs args) {
            Windows.UI.Input.PointerPoint currentPoint = args.GetCurrentPoint(this);
            for (int i = 0; i < _pointerStates.Count; i++) {
                PointerState state = _pointerStates[i];
                if (state.pointerId == currentPoint.PointerId) {
                    state.pointerPoint = currentPoint;
                    _pointerStates[i] = state;
                    UpdateInputCoordinates(NativeActionMove);
                    args.Handled = true;
                    break;
                }
            }
        }

        protected override void OnPointerReleased(Windows.UI.Xaml.Input.PointerRoutedEventArgs args) {
            Windows.UI.Input.PointerPoint currentPoint = args.GetCurrentPoint(this);
            for (int i = 0; i < _pointerStates.Count; i++) {
                PointerState state = _pointerStates[i];
                if (state.pointerId == currentPoint.PointerId) {
                    state.pointerPoint = currentPoint;
                    UpdateInputCoordinates(i == 0 ? NativeActionPointer1Up : NativeActionPointer2Up);
                    _pointerStates.RemoveAt(i);
                    args.Handled = true;
                    break;
                }
            }
        }

        protected override void OnPointerCanceled(Windows.UI.Xaml.Input.PointerRoutedEventArgs args) {
            _pointerStates.Clear();
            UpdateInputCoordinates(NativeActionCancel);
            args.Handled = true;
        }

        protected override void OnPointerExited(Windows.UI.Xaml.Input.PointerRoutedEventArgs args) {
            _pointerStates.Clear();
            UpdateInputCoordinates(NativeActionCancel);
            base.OnPointerExited(args);
        }

        protected override void OnPointerWheelChanged(Windows.UI.Xaml.Input.PointerRoutedEventArgs args) {
            Windows.UI.Input.PointerPoint currentPoint = args.GetCurrentPoint(this);
            int delta = currentPoint.Properties.MouseWheelDelta / 120;
            if (delta != 0) {
                _baseMapView.OnWheelEvent(delta, (float)ConvertDipsToPixels(currentPoint.Position.X), (float)ConvertDipsToPixels(currentPoint.Position.Y));
                args.Handled = true;
            }
        }

        void OnSwapChainPanelLayoutUpdated(object sender, object args) {
            Redraw();
        }

        void OnSwapChainPanelSizeChanged(object sender, Windows.UI.Xaml.SizeChangedEventArgs args) {
            lock (_swapChainPanelSizeLock) {
                _swapChainPanelSize = new Windows.Foundation.Size(args.NewSize.Width, args.NewSize.Height);
            }
            lock (_renderSurfaceLock) {
                CreateRenderSurface();
            }
            Redraw();
        }

        void GetSwapChainPanelSize(out int width, out int height) {
            lock (_swapChainPanelSizeLock) {
                width = (int) (ConvertDipsToPixels(_swapChainPanelSize.Width) + 0.5f);
                height = (int) (ConvertDipsToPixels(_swapChainPanelSize.Height) + 0.5f);
            }
        }

        void CreateRenderSurface() {
            if (_renderSurface == IntPtr.Zero) {
                int width = (int)(ConvertDipsToPixels(_swapChainPanelSize.Width) + 0.5f);
                int height = (int)(ConvertDipsToPixels(_swapChainPanelSize.Height) + 0.5f);
                if (width > 0 && height > 0) {
                    _renderSurface = _eglContext.CreateSurface(_swapChainPanel);
                    _surfaceCreated = false;
                    _surfaceWidth = -1;
                    _surfaceHeight = -1;
                }
            }
        }

        void DestroyRenderSurface() {
            if (_renderSurface != IntPtr.Zero) {
                _eglContext.DestroySurface(_renderSurface);
                _renderSurface = IntPtr.Zero;
            }
        }

        void UpdateInputCoordinates(int eventType) {
            switch (_pointerStates.Count) {
                case 0:
                    _baseMapView.OnInputEvent(eventType, NativeNoCoordinate, NativeNoCoordinate, NativeNoCoordinate, NativeNoCoordinate);
                    break;
                case 1:
                    _baseMapView.OnInputEvent(eventType, (float)ConvertDipsToPixels(_pointerStates[0].pointerPoint.Position.X), (float)ConvertDipsToPixels(_pointerStates[0].pointerPoint.Position.Y), NativeNoCoordinate, NativeNoCoordinate);
                    break;
                default:
                    _baseMapView.OnInputEvent(eventType, (float)ConvertDipsToPixels(_pointerStates[0].pointerPoint.Position.X), (float)ConvertDipsToPixels(_pointerStates[0].pointerPoint.Position.Y), (float)ConvertDipsToPixels(_pointerStates[1].pointerPoint.Position.X), (float)ConvertDipsToPixels(_pointerStates[1].pointerPoint.Position.Y));
                    break;
            }
        }

        double ConvertDipsToPixels(double dips) {
            return dips * _resolutionScale;
        }
    }
}
