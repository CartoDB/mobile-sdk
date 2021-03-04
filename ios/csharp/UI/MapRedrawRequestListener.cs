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

        public void Detach() {
            lock (this) {
                _viewRef.SetTarget(null);
            }
        }

        private void SetNeedsDisplay() {
            try {
                GLKView view = null;
                lock (this) {
                    _viewRef.TryGetTarget(out view);
                }

                if (view != null) {
                    view.SetNeedsDisplay();
                }
            }
            catch (System.Exception e) {
                Carto.Utils.Log.Error("MapRedrawRequestListener.SetNeedsDisplay: " + e);
            }
        }

        public override void OnRedrawRequested() {
            DispatchQueue.MainQueue.DispatchAsync (
                new System.Action (SetNeedsDisplay)
            );
        }
    }
}
