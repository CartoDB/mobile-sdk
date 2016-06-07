{% comment %}
The original resource for this is in:
https://developer.nutiteq.com/getting-started
{% endcomment %}

# Getting Started

This getting started procedure describes how to create simple map applications for your mobile platform. It is recommended to familiarise yourself with the setup for the platform that you are using. You can then add basic and advanced map features, as specified in this high-level workflow for using the Mobile SDK. 

1. Register for a [CartoDB account](https://cartodb.com/) and register **Mobile API Key** under Account.


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

 
2) Define INTERNET permission for your AndroidManifest.xml

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
### 1. Create new project with Nutiteq SDK framework

*Create new project in Xcode, add SDK framework (binary and headers) and
assets to it, and make sure that builder will find new framework .h
files. Also add dependencies: GLKit and libz.*

1.  Create a new Xcode project. ‘File’ ~~&gt; ’New’~~&gt; ‘Project’
    -&gt; ‘Single View application’.
2.  Suggested: Create **Podfile** with dependency
    [‘Nutiteq’](https://cocoapods.org/pods/Nutiteq) , install the Pods
    and open Workspace file.
3.  (Alternative: Copy Nuti.framework and nutibright.zip to the project
    root directory. You get these files from [Downloads](/downloads) .
    Then add dependencies: **GLkit.framework** and **libz** frameworks
    (Build Phases &gt; Link Binary with Libraries))

### 2. Modify Controller for Map View {#controller-for-map-view}

*Extend ViewController and add MapView manipulation code into it. Make
sure it is Objective C++, not plain Objective C class*

1.  **Rename** ‘ViewController.m’ (comes with template) to
    ‘ViewController.mm’ to avoid compilation issues
2.  Make **ViewController.h** to extend **GLKViewController** instead of
    UIViewController
3.  Replace **YOUR\_LICENSE\_KEY** with your license key in code below.
    You have to be [registered
    here](https://developer.nutiteq.com/signup?plan_ids[]=2357355792402)
    (free Lite plan is fine), and get the code from [My
    apps](https://developer.nutiteq.com/admin) page.

``` {.brush: .objc}
#import 

@interface ViewController : GLKViewController

@end
```

and implementation **ViewController.mm** :

``` {.brush: .objc}

#import "ViewController.h"
#import 

@implementation ViewController

- (void)loadView {
  // The initial step: register your license. 
  // This must be done before using MapView
  [NTMapView registerLicense:@"YOUR_LICENSE_KEY"];
  [super loadView];
}


- (void)viewDidLoad
{

 [super viewDidLoad];
  // minimal map definition code 

  // The storyboard has NTMapView connected as a view
  NTMapView* mapView = (NTMapView*) self.view;

  // Create online vector tile layer, use style asset embedded in the project
  NTVectorTileLayer* vectorTileLayer = [[NTNutiteqOnlineVectorTileLayer alloc] initWithSource: @"nutiteq.osm" styleAssetName:@"nutibright.zip"];

  // Add vector tile layer
  [[mapView getLayers] add:vectorTileLayer];


@end
```

### 2. Modify storyboard to enable Map View {#modify-storyboard-to-enable-view}

*Default storyboard template uses UIView class, we need NTMapView here
instead.*

1.  Repeat next step with both **Main\_iPhone.storyboard** and
    **Main\_iPad.storyboard** files, if you have both
2.  Open Main.Storyboard, find *View Controller Scene -&gt; View
    Controller* and *View*. From Navigator window find **Identity
    Inspector**, and change the first parameter (Custom Class) to
    **NTMapView** (from the default UIView).

![](/images/xcode_storyboard.png)

### 3. Run the app

Run the app - you should see the map with OpenStreetMap, as defined in
your ViewController. It is zoomable, rotatable and tilt-able out of the
box, with the default world view.

### 4. View manipulations

Manipulating the map view goes via *MapView* object methods:

``` {.brush: .objc}
  // Set the base projection, that will be used for most MapView, MapEventListener and Options methods
  NTEPSG3857* proj = [[NTEPSG3857 alloc] init];
  [[mapView getOptions] setBaseProjection:proj]; // EPSG3857 is actually the default base projection, so this is actually not needed

  // General options
  [[mapView getOptions] setRotatable:YES]; // make map rotatable (this is actually the default)
  [[mapView getOptions] setTileThreadPoolSize:2]; // use 2 threads to download tiles

  // Set initial location and other parameters, don't animate
  [mapView setFocusPos:[proj fromWgs84:[[NTMapPos alloc] initWithX:24.650415 y:59.428773]]  durationSeconds:0];
  [mapView setZoom:14 durationSeconds:0];
  [mapView setRotation:0 durationSeconds:0];
```

## Xamarin (Android and iOS)

### Get component from Xamarin Store

[Xamarin Component Store](https://components.xamarin.com/view/nutiteqmapssdk) has Nutiteq Maps SDK component, which can be added to your Android or iOS project directly from Xamarin Studio.


### Register license key

Sign up in [developer.nutiteq.com](http://developer.nutiteq.com) and add an application. Select **xamarin-ios** or **xamarin-android** as application type, and make sure you enter same application ID as you have in your app. For examole, sample app ID is **com.nutiteq.hellomap.xamarin** for both platforms. Finally you should get license code, which is a long string starting with *"XTU..."*. This is needed for your code

If you cover both platforms, register license codes for the two apps.

### Cross-platform apps

You can create one Xamarin project (solution) for Android and iOS and share code. These still need to be two apps, as many app aspects (UI, file system etc) are platform-specific. From Nutiteq SDK point of view the API is almost the same and your code can be shared, except some specific API calls which need Android *context* or file system references. For example these calls must be platform specific:

* Register license key: *MapView.RegisterLicense()*
* Create package manager: *new NutiteqPackageManager()*

Almost all of the map API related code: adding layers and objects to map, handling interactions/clicks etc can be shared for iOS and Android!

### Android app

1) **Uncompress Nutiteq Xamarin SDK Android package** to your project, so you have the .dll file under Assemblies folder

2) **Copy vector style file** (*osmbright.zip*) to your project *Assets* folder. You can take it from samples. This is needed for vector basemap.

3) **Add MapView to your application main layout**

<pre class="brush: xml" >
<?xml version="1.0" encoding="utf-8"?>
<LinearLayout xmlns:android="http://schemas.android.com/apk/res/android"
    android:layout_width="fill_parent"
    android:layout_height="fill_parent"
    android:orientation="vertical" >
   <nutiteq.ui.MapView
    android:id="@+id/mapView"
    android:layout_width="fill_parent" 
    android:layout_height="fill_parent" 
    />
</LinearLayout>
</pre>

4) **Create MapView object, add a base layer** 

You you can load layout from a xml and load the MapView from Layout, or create it with code. Definition of base layer is enough for minimal map configuration.

<pre class="brush: csharp">
using Nutiteq.Ui;
using Nutiteq.Layers;
using Nutiteq.DataSources;


[Activity (Label = "Nutiteq.HelloMap", MainLauncher = true)]
public class MainActivity : Activity
{

	protected override void OnCreate ( Bundle bundle )
	{
		base.OnCreate ( bundle );

		// Register license BEFORE creating MapView (done in SetContentView)
		MapView.registerLicense("YOUR_LICENSE_KEY", this);

		/// Set our view from the "main" layout resource
		SetContentView ( Resource.Layout.Main );
	
		/// Get our map from the layout resource. 
		var mapView = FindViewById<MapView> ( Resource.Id.mapView );

		/// Online vector base layer
		var baseLayer = new NutiteqOnlineVectorTileLayer("osmbright.zip");

		/// Set online base layer  
		mapView.Layers.Add(baseLayer);
	}
	
</pre>


### iOS app

1) **Uncompress Nutiteq Xamarin iOS SDK package** to your project, so you have the .dll file under Assemblies folder

2) **Copy vector style file** (*osmbright.zip*) to your project. You can take it from samples. This is needed for vector basemap.

3) **Add Map object to app view**. When using Storyboards, use *OpenGL ES View Controller* (GLKit.GLKViewController)
as a template for the map and replace *GLKView* with *MapView* as the underlying view class.
In the example below, it is assumed that the outlet name of the map view is *Map*.

4) **Initiate map, set base layer**

Add into MainViewController.cs:

<pre class="brush: csharp">
using Nutiteq.Ui;
using Nutiteq.Layers;
using Nutiteq.DataSources;

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
		var baseLayer = new NutiteqOnlineVectorTileLayer("osmbright.zip");

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



### Android and iOS common map code

1) **Add a marker** to map, to given coordinates. Add following after creating mapView.

You must have *Icon.png* in your Assets folder to set bitmap

<pre class="brush: csharp">
	// Create overlay layer for markers
	var proj = new EPSG3857();
	var dataSource = new LocalVectorDataSource (proj);
	var overlayLayer = new VectorLayer (dataSource);
	mapView.Layers.Add (overlayLayer);

	// create Marker style
	var markersStyleBuilder = new MarkerStyleBuilder ();
	markersStyleBuilder.Size = 20;
	UnsignedCharVector iconBytes = AssetUtils.LoadBytes("Icon.png");
	var bitmap = new Bitmap (iconBytes, true);
	markersStyleBuilder.Bitmap = bitmap;

	// Marker for London
	var marker = new Marker (proj.FromWgs84(new MapPos(-0.8164,51.2383)), markersStyleBuilder.BuildStyle ());
	dataSource.Add (marker);

</pre>

### Other map actions

See [hellomap3d-dotnet](https://github.com/nutiteq/hellomap3d-dotnet/) sample code how to:

* **Control map view** - set zoom, center, tilt etc
* **Listen events** (MapListener.cs) of clicks to map and map objects
* **Add other objects**: Lines, Polygons, Points, Balloons (callouts). You can even add 3D objects and use customized Balloons.
* **Download offline map packages** for country or smaller region

## Windows Phone 10

### Get Visual Studio extension package

Requirements:

* Windows 10
* MS Visual Studio 2013 Community edition, or better
* Windows Phone 10 SDK, should come with Visual Studio
* Visual Studio extension (VSIX) for Nutiteq Maps SDK component from [Nutiteq SDK Downloads](/downloads). Download and just start the package to install it.


### Register license key

Sign up in [developer.nutiteq.com](http://developer.nutiteq.com) and add an application. Select **Windows Phone** as application type, and make sure you enter same application ID as you have in your *Package.appmanifest > Packaging > Package name*. For example, sample app ID is **c882d38a-5c09-4994-87f0-89875cdee539**. Finally you should get license code, which is a long string starting with *"XTU..."*. This is needed for your code.

### Cross-platform apps

You can create one .Net project (solution) for Android, iOS, Windows Phone and share map-related code. These still need to be separate apps, as many app aspects (UI, file system etc) are platform-specific. From Nutiteq SDK point of view the API is almost the same and your code can be shared, except some specific API calls which need e.g. file system references or app resources. 

Almost all of the map API related code: adding layers and objects to map, handling interactions/clicks etc can be shared for iOS and Android!

.Net sample project has two solutions: one for Windows Phone and another for Xamarin, and they share one project (hellomap-shared) with map-related code.

### Creating WP app 

1) Make sure you have Nutiteq VS extension installed, and your app project has Internet Capability as minimum. In *Solution Explorer References* section, add *Nutiteq Maps SDK for Windows Phone*. You should find it from Windows Phone 8.1 extensions. We don't have NuGet package yet, but it will come if you show us interest.

2) **Copy vector style file** (*osmbright.zip*) to your project *Assets* folder. You can take it from samples. This is needed for vector basemap.

3) **Create MapView object, add a base layer** 

You can create MapView object with code. Definition of base layer is enough for minimal map configuration.

<pre class="brush: csharp">
using Nutiteq.Core;
using Nutiteq.Graphics;
using Nutiteq.DataSources;
using Nutiteq.Projections;
using Nutiteq.Layers;
using Nutiteq.Styles;
using Nutiteq.VectorElements;

...

protected async override void OnLaunched(LaunchActivatedEventArgs e)
{
	if (mapView == null)
	{
        // Register Nutiteq app license
        var licenseOk = Nutiteq.Ui.MapView.RegisterLicense("YOUR_LICENSE_KEY");

        // Create map view and initialize
        mapView = new Nutiteq.Ui.MapView();

        // Online vector base layer
        var baseLayer = new NutiteqOnlineVectorTileLayer("osmbright.zip");

        // Set online base layer.
        // Note: assuming here that Map is an outlet added to the controller.
        mapView.Layers.Add(baseLayer);
        
	}

	// Place the page in the current window and ensure that it is active.
	Windows.UI.Xaml.Window.Current.Content = mapView;
	Windows.UI.Xaml.Window.Current.Activate();
}

private Nutiteq.Ui.MapView mapView;	

...

</pre>


### Add a marker

To create a map Marker to given coordinates add following after creating mapView.

Note: You must have *Icon.png* in your Assets folder to set bitmap

<pre class="brush: csharp">
// Create overlay layer for markers
var proj = new EPSG3857();
var dataSource = new LocalVectorDataSource(proj);
var overlayLayer = new VectorLayer(dataSource);
mapView.Layers.Add(overlayLayer);

// create Marker style
var markersStyleBuilder = new MarkerStyleBuilder();
markersStyleBuilder.Size = 20;
UnsignedCharVector iconBytes = AssetUtils.LoadBytes("Icon.png");
var bitmap = new Bitmap(iconBytes, true);
markersStyleBuilder.Bitmap = bitmap;

// Marker for London
var marker = new Marker(proj.FromWgs84(new MapPos(-0.8164, 51.2383)), markersStyleBuilder.BuildStyle());
dataSource.Add(marker);

</pre>

### Other map actions

See [hellomap3d-dotnet](https://github.com/nutiteq/hellomap3d-dotnet/) sample project (solution: *hellomap-winphone.sln*) how to:

* **Control map view** - set zoom, center, tilt etc
* **Listen events** (MapListener.cs) of clicks to map and map objects
* **Add other objects**: Lines, Polygons, Points, Balloons (callouts). You can even add 3D objects and use customized Balloons.
* **Download offline map packages** for country or smaller region
* See [Guides](/guides) pages with dotnet samples tab for other actions
 
