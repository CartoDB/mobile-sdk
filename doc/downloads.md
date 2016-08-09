# SDK Downloads

There are two ways to download SDK: suggested method is using SDK management features, depending on your IDE, and as alternative you can download specific versions of SDK packages as zip files and add them to you projects.

Here are some additional resources, as style files what could be useful.

## Managed library usage
### Android

Add to *project build.gradle*:

```groovy
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
```

Add to *module build.gradle*:
```groovy
dependencies {
    compile 'com.carto:carto-mobile-sdk:snapshot@aar'
}
```

### iOS
In preview phase managed download is not available, use zip download below. When SDK is released, then CocoaPod will be made available.

### Xamarin
Add *nuget* https://www.nuget.org/packages/CartoMobileSDK/ to your project.
To find it make sure you enable "Show preview" in Xamarin Studio package management.

## Download packages and release notes
### Version: 4.0.0-preview

Following are live snapshots, they are updated daily right now.

* Android: [sdk4-android-snapshot-latest.zip](https://nutifront.s3.amazonaws.com/sdk_snapshots/sdk4-android-snapshot-latest.zip)
* iOS: [sdk4-ios-snapshot-latest.zip](https://nutifront.s3.amazonaws.com/sdk_snapshots/sdk4-ios-snapshot-latest.zip)
* Xamarin: separate download not available, use nuget.
