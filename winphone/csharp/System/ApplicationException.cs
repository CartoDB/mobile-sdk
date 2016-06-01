namespace System {

    // Class for compatibility between Swig-generated wrappers and WP8
    internal class ApplicationException : Exception {

        public ApplicationException() {
        }

        public ApplicationException(string message) : base(message) {
        }

        public ApplicationException(string message, Exception innerException) : base(message, innerException) {
        }
    }
}