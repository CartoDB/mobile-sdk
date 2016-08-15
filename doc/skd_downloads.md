# SDK Downloads

To begin using the Mobile SDK, download the required SDK packages and libraries based on your mobile platform. _Some mobile platforms are only available for select account plans._ [Contact us](mailto:support@carto.com) if you have questions about Mobile SDK packages and what features are included.

There are multiple ways to download SDK packages:

- Use the Managed library package based on your platform (this is the suggested method for managing SDK features, and also contains some default style files)

- Download specific versions of SDK packages as zip files, and add them to you projects

## Managed SDK Libraries

The following platform SDK libraries are managed by CARTO and include all the required libraries to support the Mobile SDK.

### Android SDK

Download and install the Android SDK package from CARTO.

- Add to *project build.gradle*:

{% highlight groovy %}
allprojects {
    repositories {
        jcenter()
        maven {
            url "http://repository-nutiteq.forge.cloudbees.com/release/"
        }
        maven {
            url "http://repository-nutiteq.forge.cloudbees.com/snapshot/"
        }
    }
}
{% endhighlight %}

- Add to *module build.gradle*:

{% highlight groovy %}
dependencies {
    compile 'com.carto:carto-mobile-sdk:snapshot@aar'
}
{% endhighlight %}

_See [Android Implementation](/docs/carto-engine/mobile-sdk/mobile-platforms-implementation/#android-implementation) for help getting started with this type of mobile platform._

### iOS SDK

Download and install the iOS SDK package from CARTO.

_**Note:** The beta Mobile SDK managed download is currently not available. Use the zip file [download package](#download-package) instead._ When the final Production version of Mobile SDK is release, the iOS SDK package will be available. 

_See [iOS Implementation](/docs/carto-engine/mobile-sdk/mobile-platforms-implementation/#ios-implementation) for help getting started with this type of mobile platform._

### Xamarin SDK

Download and install the Xamarin SDK package from CARTO.

- Add *nuget* [https://www.nuget.org/packages/CartoMobileSDK/](https://www.nuget.org/packages/CartoMobileSDK/) to your project

- To locate the downloaded package, ensure you enable "Show preview" in Xamarin Studio [package manager](/docs/carto-engine/mobile-sdk/mobile-platforms-implementation/#xamarin-android-and-ios-implementation)

_See [Xamarin Implementation](/docs/carto-engine/mobile-sdk/mobile-platforms-implementation/#xamarin-android-and-ios-implementation) for help getting started with this type of mobile platform._

### Windows Phone SDK

_**Note:** Currently, there is no managed Windows Phone (WP) SDK package. The mobile development team is still testing Windows Phone implementation with the Mobile SDK_
.

## Download Packages 

The following links will download the platform specific package directly to your local Downloads folder, and includes Release Notes.

_**Note:** The current version, 4.0.0, is a beta release. These zip files are subject to frequent changes._

- Android: [sdk4-android-snapshot-latest.zip](https://nutifront.s3.amazonaws.com/sdk_snapshots/sdk4-android-snapshot-latest.zip)

- iOS: [sdk4-ios-snapshot-latest.zip](https://nutifront.s3.amazonaws.com/sdk_snapshots/sdk4-ios-snapshot-latest.zip)

- Xamarin: there is no individual download package for Xamarin, download [nuget](https://www.nuget.org/packages/CartoMobileSDK/) instead

- Windows Phone: Install the [Visual Studio Extension Package](https://msdn.microsoft.com/en-us/library/bb166441.aspxf) from Windows and see the [Windows Phone implementation](/docs/carto-engine/mobile-sdk/mobile-platforms-implementation/#windows-phone-implementation) details
