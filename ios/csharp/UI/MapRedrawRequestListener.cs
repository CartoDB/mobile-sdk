namespace Carto.Ui {
    using System;
    using CoreFoundation;
    using GLKit;
    using Carto.Renderers;

    // iOS-specific implementation of RedrawRequestListener.
    internal class MapRedrawRequestListener : RedrawRequestListener {
        private readonly WeakReference<GLKView> _viewRef;

        public MapRedrawRequestListener(GLKView view) {
            _viewRef = new WeakReference<GLKView>(view);
        }

        public override void OnRedrawRequested() {
            GLKView view = null;
            if (_viewRef.TryGetTarget (out view)) {
                if (view != null) { // check probably not needed
                    DispatchQueue.MainQueue.DispatchAsync (
                        new System.Action (view.SetNeedsDisplay)
                    );
                }
            }
        }
    }
}
