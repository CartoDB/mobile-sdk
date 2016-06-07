{% comment %}
The original resource for this is in:
https://developer.nutiteq.com/getting-started
{% endcomment %}

# Getting Started

This getting started procedure describes how to create simple map applications for your mobile platform. It is recommended to familiarize yourself with the setup for the platform that you are using. You can then add basic and advanced map features, as specified in this high-level workflow for using the Mobile SDK. 

1. Register for a [Nutiteq account](https://developer.nutiteq.com/)

	Currently, you must use the Nutiteq SDK "Free Lite" package in collaboration with CartoDB.

2. View the setup guides for your mobile platform
	
	- [Android](#android)

	- [iOS](/cartodb-platform/mobilesdk/mobilesdk-getting-started/getting-started/ios/)

	- [Xamarin (Android and iOS)](/cartodb-platform/mobilesdk/mobilesdk-getting-started/getting-started/xamarin/)

	- [Windows Phone 8.1](/cartodb-platform/mobilesdk/mobilesdk-getting-started/getting-started/windows-phone/)

3. Download [sample mobile apps](http://localhost:9000/cartodb-platform/mobilesdk/sampleapps/#sample-apps) to view and test working code

4. Add [basic and advanced map features](http://localhost:9000/cartodb-platform/mobilesdk/mobilesdk-guides/#guides)

5. Reference the [API docs](http://localhost:9000/cartodb-platform/mobilesdk/api/#api-references) for specific functions

6. Enable [pro extensions](http://localhost:9000/cartodb-platform/mobilesdk/proextensions/#pro-extensions) for more complex mobile apps

## Android

If using Android as the mobile platform, follow this implementation procedure.

1) Add the following **Android Studio** `build.gradle` SDK files to your project

<pre class="brush: xml" >

allprojects {
    repositories {
        mavenCentral()
        maven {
            url <a href="http://repository-nutiteq.forge.cloudbees.com/release/" target="_blank">"http://repository-nutiteq.forge.cloudbees.com/release/"</a>
        }
        maven {
            url <a href="http://repository-nutiteq.forge.cloudbees.com/snapshot/" target="_blank">"http://repository-nutiteq.forge.cloudbees.com/snapshot/"</a>
        }
    }
}

dependencies {
   compile 'com.nutiteq:nutiteq-sdk:3.2.4@aar'
}
</pre>

2) Download the required **Eclipse** SDK files to your project. 

- From the latest [Android SDK preview 3.3.0](https://nutifront.s3.amazonaws.com/sdk_snapshots/sdk3-android-build-153-bc0defef6c65e89d571d02bb7869f2703982d977.zip) zip package, add these two files to your project **libs/** folder:

    - `nutiteq-maps-sdk.jar`
    - `armeabi-v7a/libnutiteq_maps_sdk.so`

- Add the [`nutibright-v2a.zip`](https://nutifront.s3.amazonaws.com/releases/nutibright-v2a.zip) style file to your project **assets** 

**Note:** Other [SDK downloads](https://developer.nutiteq.com/downloads) are available, but these three are required to work with CartoDB.
 
3) Define INTERNET permission for your AndroidManifest.xml

```xml
<uses-permission android:name="android.permission.INTERNET"/>
```

3) Define your application layout

Define **main layout** as **res/layout/main.xml**, so that it contains `com.nutiteq.ui.MapView` element:

{% highlight xml %}
<?xml version="1.0" encoding="utf-8"?>
<LinearLayout xmlns:android="http://schemas.android.com/apk/res/android"
    xmlns:tools="http://schemas.android.com/tools"
    android:layout_width="fill_parent"
    android:layout_height="fill_parent"
    android:orientation="vertical" >
   <com.nutiteq.ui.MapView
    android:id="@+id/mapView"
    android:layout_width="fill_parent" 
    android:layout_height="fill_parent" 
    />
</LinearLayout>
{% endhighlight %}

4) Find the MapView object

Define the MapView type in your main activity class and load layout. This enables you to load the MapView from the layout. _The object itself was already created during the layout creation process, this step is specific to finding and referencing the MapView object in your request._

<pre class="brush: java">public class HelloMap3DActivity extends Activity {
    private MapView mapView;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.main);
</pre>

5) Initialize the mobile map

The map object needs a default map source in order to initialize. There are several requirements for this to work properly.

- Replace `YOUR_LICENSE_KEY` with your Nutiteq license key

    Once [registered](#getting-started), you can get your license key from the [Nutiteq admin / My apps](https://developer.nutiteq.com/login) page.

- Define the first layer of the map, which will be the basemap layer. This is a vector map layer, which requires that you load and define styles in the assets of this layer. You can also add other map layers once a basemap is configured.

The following example shows the complete request for initializing your Android mobile map.

<pre class="brush: java">
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.main);

        // Register and replace your license key. This must be done before using MapView!
        MapView.registerLicense("YOUR_LICENSE_KEY", getApplicationContext());
 
        // Create map view 
        mapView = (MapView) this.findViewById(R.id.mapView);
 
        // Create basemap layer. Use vector style from assets
        VectorTileLayer baseLayer = new NutiteqOnlineVectorTileLayer("nutibright-v2a.zip");
 
        // Add layer to map
        mapView.getLayers().add(baseLayer);
    }
</pre>

Congratulations! You can now start the application on your phone and view your map. View the [Sample Mobile Apps](/cartodb-platform/mobilesdk/sampleapps/#sample-apps) section if you need help testing and viewing Android maps on your phone.

## iOS

## Xamarin (Android and iOS)

## Windows Phone 8.1


 
