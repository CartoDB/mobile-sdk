namespace System {

    // Class for compatibility between Swig-generated wrappers and WP8
    internal class SystemException : Exception {

        public SystemException() {
        }

        public SystemException(string message) : base(message) {
        }

        public SystemException(string message, Exception innerException) : base(message, innerException) {
        }
    }
}