namespace System.Runtime.InteropServices {

    // For some reason, .NET for Windows Store Apps does not provide HandleRef.
    // SWIG-generated code depends on this, though could be converted to plain IntPtr.
    // It seems that .NET runtime still supports this internally.
    // It is important to keep the struct layout as defined here, as PINVOKE system assumes fixed layout.
    internal struct HandleRef {

        internal object mWrapper;
        internal System.IntPtr mHandle;

        public HandleRef(object wrapper, System.IntPtr handle) {
            mWrapper = wrapper;
            mHandle = handle;
        }

        public object Wrapper {
            get {
                return mWrapper;
            }
        }

        public IntPtr Handle {
            get {
                return mHandle;
            }
        }

        public static explicit operator IntPtr(HandleRef value) {
            return value.mHandle;
        }

        public static IntPtr ToIntPtr(HandleRef value) {
            return value.mHandle;
        }
    }
}
