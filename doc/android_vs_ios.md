# Android vs iOS implementation differences

Nutiteq SDK core code is written in C++, uses OpenGL ES 2 graphics API and is 99% unified for iOs, Android and other target platforms in the future. However, depending on your target platform and programming language there are some dfferences:

 * In **iOS/ObjectiveC namespace prefix NT** is used, under Android/Java or DotNet (either iOS or Android) there is no prefix. Therefore for example NTVectorLayer under iOS is VectorLayer in Android.
 * In constructors of some objects - MapView and defining license key there are platform-specific context objects.
 * In addition to unified API, there are some platform-specific convenience methods. For example, BitmapUtils/NTBitmapUtils class can be used to convert platform-specific bitmaps to SDK bitmap objects. 
 
Behaviour, classes, methods, method parameters of Nutiteq classes are generally exactly identical under iOs and Android. Therefore the Guides here are given in platform-agnostic way. Sample code is generally given for all the platforms/languages. In few places where this is not the case, it should be easy to convert the code as needed.
