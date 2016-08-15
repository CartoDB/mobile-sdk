# Mobile Platform Implementation

Once your mobile apps are registered and you have your API Keys, it is recommended to familiarize yourself with the setup for the platform that you are using. Some of these platforms contain unique map features that are only available based on the mobile platform. You can then use sample mobile apps and add basic and advanced map features.

## Android Implementation

If using Android as the mobile platform, follow this implementation procedure.

1) Add the **[Android Studio](/docs/carto-engine/mobile-sdk/sdk-downloads-and-sample-apps/#android-sdk)** `build.gradle` SDK files to your project

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
   compile 'com.carto:carto-mobile-sdk:4.0.0-snapshot@aar'
}
</pre>
 
2) Define INTERNET permission for your AndroidManifest.xml

{% highlight xml %}
<uses-permission android:name="android.permission.INTERNET"/>
{% endhighlight %}

3) Define your application layout

  Define **main layout** as **res/layout/main.xml**, so that it contains `com.carto.ui.MapView` element:

{% highlight xml %}
<?xml version="1.0" encoding="utf-8"?>
<LinearLayout xmlns:android="http://schemas.android.com/apk/res/android"
    xmlns:tools="http://schemas.android.com/tools"
    android:layout_width="fill_parent"
    android:layout_height="fill_parent"
    android:orientation="vertical" >
   <com.carto.ui.MapView
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

  - Replace `YOUR_LICENSE_KEY` with your [Mobile Apps and API Key](/docs/carto-engine/mobile-sdk/mobile-apps-and-api-keys/#mobile-apps-and-api-keys)

  - Define the first layer of the map, which will be the basemap layer. This is a vector map layer, which requires that you load and define styles in the assets of this layer. You can also add other map layers once a basemap is configured

### Example - Android Request

The following example shows the complete request for initializing your Android mobile map.

<pre class="brush: java">
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.main);

        // 1. Register and replace your [Mobile API license key. This must be done before using MapView!
        MapView.registerLicense("YOUR_LICENSE_KEY", getApplicationContext());
 
        // 2. Create map view 
        mapView = (MapView) this.findViewById(R.id.mapView);
 
        // 3. Create basemap layer. Use vector style from assets
        VectorTileLayer baseLayer = new CartoOnlineVectorTileLayer("nutiteq.osm");
 
        // 4. Add layer to map
        mapView.getLayers().add(baseLayer);
    }
</pre>

You can now start using the application on your phone and view your map.

## iOS Implementation

If using iOS as the mobile platform, follow this implementation procedure.

1) Create new project in Xcode and add Mobile SDK framework using Podfile

  -  Create a new ‘Single View application’ in your Xcode project

  -  For beta preview: 
      - Get SDK package latest dev build: [sdk4-ios-snapshot-latest.zip](https://nutifront.s3.amazonaws.com/sdk_snapshots/sdk4-ios-snapshot-latest.zip)
      - Unzip it and copy *CartoMobileSDK.framework* to the Xcode project root folder

  -  For live SDK release: create **Podfile** in your project folder with dependency
    [‘Carto-Mobile-SDK’](https://cocoapods.org/pods/Carto-Mobile-SDK) , install the Pods and open *Workspace* file. **Note: This is not available yet**

2)  Modify Controller for Map View

  - Extend ViewController and add MapView manipulation code into it. _Ensure it is Objective C++, not plain Objective C class_

  - Rename `ViewController.m` (comes with template) to `ViewController.mm`, to avoid compilation issues

  -  Implement **ViewController.h** to extend **GLKViewController**, instead of
    UIViewController

  - You must replace `YOUR_LICENSE_KEY` with your [Mobile Apps and API Key](/docs/carto-engine/mobile-sdk/mobile-apps-and-api-keys/#mobile-apps-and-api-keys) in the code below

<pre class="brush: objc">
<pre class="brush: objc">
#import 

@interface ViewController : GLKViewController

@end
</pre>

#import "ViewController.h"
#import 

@implementation ViewController

- (void)loadView {
  // 1. The initial step: register your license. 
  // **Note:** This must be done before using MapView
  [NTMapView registerLicense:@"YOUR_LICENSE_KEY"];
  [super loadView];
}


- (void)viewDidLoad
{

 [super viewDidLoad];
  // 1. Add minimal map definition code 

  // 2. Ensure the storyboard has NTMapView connected as a view
  NTMapView* mapView = (NTMapView*) self.view;

  // 3. Create online vector tile layer, use style asset embedded in the project
  NTVectorTileLayer* vectorTileLayer = [[NTCartoOnlineVectorTileLayer alloc] initWithSource: @"nutiteq.osm" styleAssetName:@"nutibright-v3.zip"];

  // 4. Add vector tile layer
  [[mapView getLayers] add:vectorTileLayer];


@end
</pre>

3) Modify storyboard to enable Map View

  _The default storyboard template uses UIView class, you must use NTMapView instead._

  **Note:** If you are using iPhone (**Main\_iPhone.storyboard**) or iPad (**Main\_iPad.storyboard**) files for iOS, you must repeat this step to change the default storyboard.

  -  Open Main.Storyboard, select *View Controller Scene -&gt; View Controller* -&gt; *View*

  - From Navigator window, select **Identity Inspector**, change the first parameter (Custom Class) to **NTMapView** (from the default UIView).

  <span class="wrap-border"><img src="{{ site.baseurl }}/img/layout/mobile/xcode_storyboard.jpg" alt="Xcode storyboard" /></span>

4) Run the iOS app

  The map should appear with the default world map provided by OpenStreetMap, as defined in your ViewController. You can zoom, rotate, and tilt with these default settings.

5) View the **MapView** object method changes

<pre class="brush: objc">
// 1. Set the base projection to be used for MapView, MapEventListener and Options methods
  NTEPSG3857* proj = [[NTEPSG3857 alloc] init];
  [[mapView getOptions] setBaseProjection:proj]; // Since EPSG3857 is the default base projection, this is not needed

  // 2. General options
  [[mapView getOptions] setRotatable:YES]; // allows the map to rotate (this is the default behavior)
  [[mapView getOptions] setTileThreadPoolSize:2]; // use two threads to download tiles

  // 3.Set initial location and other parameters, _do not animate_
  [mapView setFocusPos:[proj fromWgs84:[[NTMapPos alloc] initWithX:24.650415 y:59.428773]]  durationSeconds:0];
  [mapView setZoom:14 durationSeconds:0];
  [mapView setRotation:0 durationSeconds:0];
</pre>

## Xamarin (Android and iOS) Implementation

If using Xamarin as the mobile platform, follow these implementation procedures for Xamarin (Android) and Xamarin (iOS).

1) Add library as nuget [CartoMobileSDK](https://www.nuget.org/packages/CartoMobileSDK/) from the main repo, and add to your mobile app

2) Register app license key with your [Mobile Apps and API Key](/docs/carto-engine/mobile-sdk/mobile-apps-and-api-keys/#mobile-apps-and-api-keys)

  **Note:** If you are using both Xamarin Android and iOS, register each platform as its own app.

3) Create a cross-platform project for your apps

  Each platform still needs to be registered as its own app, since many app aspects (such as UI, file system, and so on) are platform-specific. However, when executing API requests with the Mobile SDK, you can create one Xamarin project for Android and iOS and share the code. _Some exceptions apply in regards to API calls which need Android context, or file system references._ For example, the following API requests are platform specific:

  - Register license key: `MapView.RegisterLicense()`
  - Create package manager: `new CartoPackageManager()`

  Almost all of the map related API code (such as adding layers and objects to map, handling interactions and clicks, etc.) can be shared for iOS and Android through one project!

### Xamarin Forms App

Xamarin Forms (version 3.3.0 and higher) support *Native Controls*. If you add Mobile SDK apps for iOS and Android platforms, Xamarin Native Controls is available by default. See the blog, [_Embedding Native Controls into Xamarin.Forms_](https://blog.xamarin.com/embedding-native-controls-into-xamarin-forms) for details. 

While you can share most of code using Native Controls, you just need to specify the platform when creating the project:

<pre class="brush: csharp">
#if __IOS__
 // 1. iOS specific code
            var mapView = new Carto.Ui.MapView();
            mapView.Frame = new CGRect(20, 20, 280, 80);
            stack.Children.Add(mapView);
#endif
 // 2. Indicate the common code from both platforms
    var baseLayer = new Carto.Layers.CartoOnlineVectorTileLayer("carto.osm");
     mapView.Layers.Add(baseLayer);
</pre>

**Note:** Native Controls only work if you create or update `Form` in the code, using the xml definition will not work. This Xamrin development requirement is subject to change with each release. _Mobile SDK with Xamarin Forms is currently in being tested with Native apps._ Please [contact us](mailto:support@carto.com) if you have an issues.

### Android Native App

Follow these steps to add native apps to your Xamarin Android package.

1) Add the nuget package [CartoMobileSDK](https://www.nuget.org/packages/CartoMobileSDK/) to your mobile app project

2) Copy **vector style file** (as *.zip* file) to your project *Assets* folder

  You can use the [sample app](/docs/carto-engine/mobile-sdk/sample-apps/#android-samples) resources for your assets. This is required for vector basemaps

3) Add MapView to your application main layout

```xml
<LinearLayout xmlns:android="http://schemas.android.com/apk/res/android"
    android:layout_width="fill_parent"
    android:layout_height="fill_parent"
    android:orientation="vertical" >
   <carto.ui.MapView
    android:id="@+id/mapView"
    android:layout_width="fill_parent" 
    android:layout_height="fill_parent" 
    />
</LinearLayout>
```

4) Create MapView object, add a base layer

  Load layout from a xml, and load the MapView from Layout. Or, create it with code. A definition of a base layer is enough for minimal map configuration.

{% highlight csharp %}
using Carto.Ui;
using Carto.Layers;
using Carto.DataSources;

[Activity (Label = "Carto.HelloMap", MainLauncher = true)]
public class MainActivity : Activity
{

  protected override void OnCreate ( Bundle bundle )
  {
    base.OnCreate ( bundle );

    // 1. Register license BEFORE creating MapView (done in SetContentView)
    MapView.registerLicense("YOUR_LICENSE_KEY", this);

    /// 2. Set our view from the "main" layout resource
    SetContentView ( Resource.Layout.Main );
  
    /// 3. Get our map from the layout resource 
    var mapView = FindViewById<MapView> ( Resource.Id.mapView );

    /// 4. Online vector base layer
    var baseLayer = new CartoOnlineVectorTileLayer("osmbright.zip");

    /// 5. Set online base layer  
    mapView.Layers.Add(baseLayer);
  }
{% endhighlight %}

### Xamarin iOS App

Follow these steps to add apps to your Xamarin iOS package.

1) Uncompress CARTO Xamarin iOS SDK package to your project

  **Note:** All the .dll files should be located in the **Assemblies** folder

2) **Copy vector style file** (*osmbright.zip*) to your project. You can take it from samples. This is needed for vector basemap.

3) **Add Map object to app view**. When using Storyboards, use *OpenGL ES View Controller* (GLKit.GLKViewController)
as a template for the map and replace *GLKView* with *MapView* as the underlying view class.
In the example below, it is assumed that the outlet name of the map view is *Map*.

4) **Initiate map, set base layer**

Add into MainViewController.cs:

<pre class="brush: csharp">
using Carto.Ui;
using Carto.Layers;
using Carto.DataSources;

public class MainViewController : GLKit.GLKViewController
{
  public override void ViewDidLoad ()
  {
    base.ViewDidLoad ();

    // GLKViewController-specific parameters for smoother animations
    ResumeOnDidBecomeActive = false;
    PreferredFramesPerSecond = 60;

    // Register license BEFORE creating MapView 
    MapView.RegisterLicense("YOUR_LICENSE_KEY");

    // Online vector base layer
    var baseLayer = new CartoOnlineVectorTileLayer("osmbright.zip");

    // Set online base layer.
    // Note: assuming here that Map is an outlet added to the controller.
    Map.Layers.Add(baseLayer);
  }

  public override void ViewWillAppear(bool animated)
  {
    base.ViewWillAppear (animated);

    // GLKViewController-specific, do on-demand rendering instead of constant redrawing
    // This is VERY IMPORTANT as it stops battery drain when nothing changes on the screen!
    Paused = true;
  }
</pre>

### Xamarin Common Code

The following common code can be used for both Xamarin Android and Xamarin iOS apps. This is useful if you are sharing code between projects.

1) Add a marker, based on defined coordinates

  Add the following code after creating the [mapView](#xamarin-android-and-ios-implementation).

  **Note:** You must have *Icon.png* that is a bitmap, located in the Assets folder of your project

<pre class="brush: csharp">
  // 1. Create overlay layer for markers
  var proj = new EPSG3857();
  var dataSource = new LocalVectorDataSource (proj);
  var overlayLayer = new VectorLayer (dataSource);
  mapView.Layers.Add (overlayLayer);

  // 2. Create Marker style
  var markersStyleBuilder = new MarkerStyleBuilder ();
  markersStyleBuilder.Size = 20;
  UnsignedCharVector iconBytes = AssetUtils.LoadBytes("Icon.png");
  var bitmap = new Bitmap (iconBytes, true);
  markersStyleBuilder.Bitmap = bitmap;

  // Example Marker for London
  var marker = new Marker (proj.FromWgs84(new MapPos(-0.8164,51.2383)), markersStyleBuilder.BuildStyle ());
  dataSource.Add (marker);

</pre>

**Tip:** See [mobile-dotnet-samples](https://github.com/CartoDB/mobile-dotnet-samples/) for other common map options for the Xamarin mobile platform, such as:

- **Control map view** - set zoom, center, tilt etc.
- **Listen events** (MapListener.cs) of clicks to map and map objects
- **Add other objects**: Lines, Polygons, Points, Balloons (callouts). You can even add 3D objects and use customized Balloons
- **Download offline map packages** for a country, or smaller region

## Windows Phone Implementation

If using Windows Phone as the mobile platform, follow this implementation procedure.

_**Note:** The mobile development team is still testing Windows Phone implementation with the Mobile SDK._ Please [contact us](mailto:support@carto.com) if you run into any issues.

1) Install [Visual Studio Extension Package](https://msdn.microsoft.com/en-us/library/bb166441.aspxf) from Windows

  The following requirements are mandatory:

  - Windows Phone version 10
  - MS Visual Studio 2013 Community edition, or better
  - Windows Phone 10 SDK, should come with Visual Studio
  - Visual Studio extension (VSIX) for CARTO Maps SDK component. Download and start the package to install it

2) [Register your app](/docs/carto-engine/mobile-sdk/mobile-apps-and-api-keys/#mobile-apps-and-api-keys) and select _Windows Phone_ as the app type

  - Ensure you enter the same application ID as your *Package.appmanifest > Packaging > Package name*. For example, the [sample app](/docs/carto-engine/mobile-sdk/sample-apps/#xamarin-and-windows-phone-samples) ID is **c882d38a-5c09-4994-87f0-89875cdee539**

3) Create a cross-platform project for your Windows Phone app

  You can create one .Net project for Android, iOS, Windows Phone and share map-related code. Each platform still needs to be registered as its own app, since many app aspects (such as UI, file system, and so on) are platform-specific. However, when executing API requests with the Mobile SDK, you can create one project for adding layers and objects to map, handling interactions and click, and so on.

  **Tip:** .Net [sample app](/docs/carto-engine/mobile-sdk/sample-apps/#xamarin-and-windows-phone-samples) contains two solutions: one for Windows Phone and another for Xamarin, and they share one project _hellomap-shared_ with map-related code.

### Create a WP App

Follow these steps in order to create a Windows Phone (WP) mobile application.

1) Ensure you have the CARTO Visual Studio Extension installed, and your app project has Internet connection

  In the *Solution Explorer References* section, add *Carto Maps SDK for Windows Phone*. You will find it from the Windows Phone 8.1 extensions. We do not have NuGet package yet, please [let us know](mailto:support@carto.com) if this is something that interests you.

2) Copy vector style file (*osmbright.zip*) to your project *Assets* folder, available from the [Sample Apps](/docs/carto-engine/mobile-sdk/sample-apps/#xamarin-and-windows-phone-samples). This is needed for vector basemaps

3) Create MapView object, and add a base layer

You can create a MapView object with code. A definition of a base layer is enough for the minimal map configuration.

<pre class="brush: csharp">
using Carto.Core;
using Carto.Graphics;
using Carto.DataSources;
using Carto.Projections;
using Carto.Layers;
using Carto.Styles;
using Carto.VectorElements;

...

protected async override void OnLaunched(LaunchActivatedEventArgs e)
{
  if (mapView == null)
  {
        // 1. Register CARTO app license
        var licenseOk = Carto.Ui.MapView.RegisterLicense("YOUR_LICENSE_KEY");

        // 2. Create map view and initialize
        mapView = new Carto.Ui.MapView();

        // 3. Online vector base layer
        var baseLayer = new CartoOnlineVectorTileLayer("osmbright-v3.zip");

        // 4. Set online base layer.
        // Note: assuming here that Map is an outlet added to the controller.
        mapView.Layers.Add(baseLayer);
        
  }

  // Place the page in the current window and ensure that it is active.
  Windows.UI.Xaml.Window.Current.Content = mapView;
  Windows.UI.Xaml.Window.Current.Activate();
}

private Carto.Ui.MapView mapView; 

...

</pre>


### Add a Marker (WP)

To create a map marker at a defined coordinate on a Windows Phone mobile app, add following code (after creating a [mapView](/docs/carto-engine/mobile-sdk/basic-map-components/#basic-map-component)).

**Note:** You must have *Icon.png* that is a bitmap, located in the Assets folder of your project

<pre class="brush: csharp">
// 1. Create overlay layer for markers
var proj = new EPSG3857();
var dataSource = new LocalVectorDataSource(proj);
var overlayLayer = new VectorLayer(dataSource);
mapView.Layers.Add(overlayLayer);

// 2. Create Marker style
var markersStyleBuilder = new MarkerStyleBuilder();
markersStyleBuilder.Size = 20;
UnsignedCharVector iconBytes = AssetUtils.LoadBytes("Icon.png");
var bitmap = new Bitmap(iconBytes, true);
markersStyleBuilder.Bitmap = bitmap;

// Example Marker for London
var marker = new Marker(proj.FromWgs84(new MapPos(-0.8164, 51.2383)), markersStyleBuilder.BuildStyle());
dataSource.Add(marker);

</pre>

**Tip:** See [mobile-dotnet-samples](https://github.com/CartoDB/mobile-dotnet-samples/) sample project (solution: *hellomap-winphone.sln*) for other WP map actions, such as:

- **Control map view** - set zoom, center, tilt etc
- **Listen events** (MapListener.cs) of clicks to map and map objects
- **Add other objects**: Lines, Polygons, Points, Balloons (callouts). You can even add 3D objects and use customized Balloons.
- **Download offline map packages** for country or smaller region
