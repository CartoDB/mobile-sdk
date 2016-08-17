# Getting Started

The following workflow guides you through the process of getting started with the Mobile SDK.

[1. Download the SDK by managed libraries, or by individual packages](#downloading-the-sdk) | 
[2. Register your mobile app to get your API Key](#registering-your-mobile-app) | 
[3. Create your mobile project, based on the platform of your device](#creating-your-project) |
[4. Create your first mobile app with basic map features](#basic-map-features) |
[5. Use our sample mobile apps to test your project, and re-use prepackaged styles](#sample-apps) | 

## Downloading the SDK

To begin using the Mobile SDK, download the required SDK packages and libraries based on your mobile platform. _Some mobile platforms are only available for select account plans._ [Contact us](mailto:support@carto.com) if you have questions about Mobile SDK packages and what features are included.

There are multiple ways to download SDK packages:

- Use the Managed library package based on your platform (this is the suggested method for managing SDK features, and also contains some default style files)

- Download specific versions of SDK packages as zip files, and add them to you projects

### Managed Libraries

These SDK libraries are managed by CARTO and include all the required libraries to support the Mobile SDK. In some cases, managed libraries are not available and the direct download package url is provided.

<div class="js-TabPanes">
  <ul class="Tabs">
    <li class="Tab js-Tabpanes-navItem is-active">
      <a href="#/0" class="js-Tabpanes-navLink">Android SDK</a>
    </li>
    <li class="Tab js-Tabpanes-navItem">
      <a href="#/1" class="js-Tabpanes-navLink">iOS SDK</a>
    </li>
    <li class="Tab js-Tabpanes-navItem">
      <a href="#/2" class="js-Tabpanes-navLink">Xamarin SDK</a>
    </li>
    <li class="Tab js-Tabpanes-navItem">
      <a href="#/3" class="js-Tabpanes-navLink">Windows Phone SDK</a>
    </li>
  </ul>

  <div class="Carousel-item js-Tabpanes-item is-active">
  {% highlight groovy %}

// 1. Download "project build.gradle" from the Android SDK package

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

// 2. Add "module build.gradle" as part of your package

dependencies {
    compile 'com.carto:carto-mobile-sdk:snapshot@aar'
}

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item">
  {% highlight groovy %}

// Download and install the iOS SDK package

    url "https://nutifront.s3.amazonaws.com/sdk_snapshots/sdk4-ios-snapshot-latest.zip"

    // Note: There is currently no managed library for iOS, download the platform package instead. When the final Production version of Mobile SDK is released, the iOS managed library package will be available

{% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item">
  {% highlight groovy %}

// 1. Download nuget for the Xamarin SDK package

    url "https://www.nuget.org/packages/CartoMobileSDK/"
        
// 2. To locate the downloaded package, ensure you enable "Show preview" in Xamarin Studio

{% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item">
  {% highlight groovy %}

// Download and install the Visual Studio Extension Package to create a Windows Phone project

    url "https://msdn.microsoft.com/en-us/library/bb166441.aspxf"

    // Note: There is currently no managed library for Windows Phone (WP), download the platform package instead. The mobile development team is still testing Windows Phone implementation with the Mobile SDK

{% endhighlight %}

  </div>
</div>

### Download Packages 

The following links will download the platform specific package directly to your local Downloads folder, and includes Release Notes.

_**Note:** The current version, 4.0.0, is a beta release. These zip files are subject to frequent changes._

- Android: [sdk4-android-snapshot-latest.zip](https://nutifront.s3.amazonaws.com/sdk_snapshots/sdk4-android-snapshot-latest.zip)

- iOS: [sdk4-ios-snapshot-latest.zip](https://nutifront.s3.amazonaws.com/sdk_snapshots/sdk4-ios-snapshot-latest.zip)

- Xamarin: there is no individual download package for Xamarin, download [nuget](https://www.nuget.org/packages/CartoMobileSDK/) instead

- Windows Phone: Install the [Visual Studio Extension Package](https://msdn.microsoft.com/en-us/library/bb166441.aspxf) from Windows and see the [Windows Phone implementation](/docs/carto-engine/mobile-sdk/mobile-platforms-implementation/#windows-phone-implementation) details

## Registering your Mobile App

You must register your mobile applications under your CARTO account settings. Once an app is added, you can retrieve the mobile app API Key and manage its settings.

The following procedure describes how to register mobiles apps under your account settings. While the number of mobile apps is unlimited, application quota consumption rules are enforced based on your account plan. [Contact us](mailto:sales@carto.com) if you have questions about your mobile account options.

1. Access [Your API Keys](/docs/carto-editor/your-account/#api-key) from the CARTO Dashboard

    <span class="wrap-border"><img src="/docs/img/layout/mobile/access_mobile_api_key.jpg" alt="Access mobile apps from API keys" /></span>

    The API Key page opens, displaying options for _CARTO_ or _Mobile Apps_. 

2. Click _Mobile apps_ to add mobile applications to your account

    <span class="wrap-border"><img src="/docs/img/layout/mobile/no_registered_apps.jpg" alt="No registered mobile apps" /></span>

3. Click _NEW APPLICATION_

    The new application settings appear.

4. Enter options for the mobile application
 
    <span class="wrap-border"><img src="/docs/img/layout/mobile/new_mobile_app_settings.jpg" alt="New mobile app settings" /></span>

    The following mobile application settings are available: 

    Mobile Application Settings | Description
    --- | ---
    App icon | Select an icon to represent your mobile application in your account
    Name | An informative name of the mobile application in your account
    Description | A description about the mobile application in your account
    Platform | Identifies the type of mobile app platform. Some options may not be available, based on your account plan<br/><br/>**Note:** Once a mobile application is saved, you cannot edit the Platform setting. As an alternative, you can [delete](#delete-a-mobile-app) the application and recreate it with new settings.<br/><br/>**Tip:** When you select a _Platform_, the _Application ID_ displays more details about what identifiers, or packages, are required.<br/><br/><img src="/docs/img/layout/mobile/platform_appid.jpg" alt="Platform App ID hint" /> 
    Application ID | The identification of the mobile app in the App store that is required for development with the Mobile SDK.<br/><br/>The following application ids identify the Platform selected:<br/><br/>- [**Android**](#android-implementation): requires the AndroidManifest.xml package<br/><br/>- [**iOS**](#ios-implementation): requires a Bundle identifier. You can find it in the project properties of your iOS package<br/><br/>- [**Xamarin Android**](#xamarin-android-and-ios-implementation): requires the AndroidManifest.xml package<br/><br/>- [**Xamarin iOS**](#xamarin-ios-app): requires a Bundle identifier. You can find it in the project properties of your Xamarin iOS package<br/><br/>- [**Windows Phone**](#windows-phone-implementation): Requires the package name from the Windows package.appmanifest 
    App type | Identifies the type of mobile application, which are subject to different quota limitations<br/><br/>- **Open**: Adds and counts mobile apps users towards global usage quotas, ranging from 10K to 500k users, depending on your account plan. This app type is open to the public and available in public app stores<br/><br/>- **Dev**: Select this app type when you are developing and testing your mobile app. Dev is available for all account plans and allows each mobile app to have up to five users. Dev apps do not appear in public app stores<br/><br/>_**Note:** Once you are satisfied with your mobile development, you can [promote a Dev app](#promoting-a-dev-app) to another app type._<br/><br/>- **Private**: Available for Enterprise accounts only, allows organizations to publish their apps internally and share quota limitations, up to 750 users. Private apps are not published to public app stores
 
5. Click _SAVE CHANGES_

    The Mobile apps page refreshes, displaying the added mobile application and the features enabled. 

### Access your Mobile API Key

Once your mobile apps are registered for your account, you can retrieve the API Key for each application. This is useful when you need to copy and paste the API Key for mobile development.

1. From the _Mobile apps_ section of your API Keys, click a mobile application name

    <span class="wrap-border"><img src="/docs/img/layout/mobile/registered_mobile_app.jpg" alt="Registered mobile apps" /></span>

    The application settings appear, and include the license key and amount of quota for the application. 

2. Copy the API Key from the _License key_ section

    The following example displays the API Keys section for a selected mobile application.  Note, the API Key in the example image is blurred out for privacy.

    <span class="wrap-border"><img src="/docs/img/layout/mobile/example_settings.jpg" alt="Example of mobile app settings" /></span>

3. Click the back navigation arrow, located next to the app name, to return Mobile apps section of your API Keys

### License Key Rules

Mobile App API Keys cannot be regenerated manually, but are automatically regenerated in the following scenarios.

- If you [request a new API Key](/docs/carto-editor/your-account/#api-key) for your CARTO API's, the Mobile apps license key is also automatically regenerated

- If you [upgrade](/docs/carto-editor/your-account/#billing) your account plan that includes additional mobile features, the Mobile apps license key is automatically regenerated

- If you change the _App type_ from **Dev** to Open or Private, the Mobile apps license key is regenerated

You will receive a notification when mobile api keys are regenerated.

### Promoting a Dev App

A Dev mobile app enables you to develop and test your mobile application. Only a developer mobile app can be promoted to another app type through the Mobile apps settings.

**Note:** Only the Dev app type can be promoted. You cannot change the mobile app type if it is Open or Private. As an alternative, you can delete the application and [recreate it](#registering-your-mobile-app) with the new app type.

1. From the _Mobile apps_ section of your API Keys, click a mobile application name

    The application settings appear. 

2. Click _Delete this application_ from the bottom of the app settings

    <span class="wrap-border"><img src="/docs/img/layout/mobile/change_app_type.jpg" alt="Promote dev app to another app type" /></span>

3. Click _SAVE CHANGES_

    A confirmation dialog appears and notifies you that the mobile app license key will also change.

4. Click _CHANGE APPLICATION TYPE_

    The selected mobile app settings refresh, and the license key is regenerated.

5. Copy and paste the regenerated API Key, as needed for your mobile app

6. Click the back navigation arrow, located next to the app name, to return Mobile apps section of your API Keys

### Delete a Mobile App

Once a mobile application is saved, you cannot edit the Platform setting. As an alternative, you can delete the application and [recreate it](#registering-your-mobile-app) with new settings.

1. From the _Mobile apps_ section of your API Keys, click a mobile application name

    The application settings appear, and include the license key and amount of quota for the application. 

2. Click _Delete this application_ from the bottom of the app settings

    <span class="wrap-border"><img src="/docs/img/layout/mobile/delete_application.jpg" alt="Delete mobile app" /></span>

3. Click the back navigation arrow, located next to the app name, to return Mobile apps section of your API Keys

## Creating your Project

Once your mobile apps are registered and you have your API Keys, it is recommended to familiarize yourself with the setup for the platform that you are using. Some of these platforms contain unique map features that are only available based on the mobile platform. You can then use sample mobile apps and add basic and advanced map features.

### Android Implementation

If using Android as the mobile platform, follow this implementation procedure.

1) Add the **[Android Studio](#android-samples)** `build.gradle` SDK files to your project

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

  - Replace `YOUR_LICENSE_KEY` with your [Mobile App API Key](#registering-you-mobile-app)

  - Define the first layer of the map, which will be the basemap layer. This is a vector map layer, which requires that you load and define styles in the assets of this layer. You can also add other map layers once a basemap is configured

#### Example - Android Request

The following example shows the complete request for initializing your Android mobile map.

<pre class="brush: java">
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.main);

        // 1. Register and replace your Mobile API license key. This must be done before using MapView!
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

### iOS Implementation

If using iOS as the mobile platform, follow this implementation procedure.

1) Create new project in Xcode and add Mobile SDK framework using Podfile

  -  Create a new ‘Single View application’ in your Xcode project

  -  For the current beta version of the Mobile SDK: 
      - Get SDK package latest dev build: [sdk4-ios-snapshot-latest.zip](https://nutifront.s3.amazonaws.com/sdk_snapshots/sdk4-ios-snapshot-latest.zip)
      - Unzip it and copy *CartoMobileSDK.framework* to the Xcode project root folder

  - COMING SOON: Once the final version of Mobile SDK is released, you will have to create "Podfile" in your project folder with dependency [‘Carto-Mobile-SDK’](https://cocoapods.org/pods/Carto-Mobile-SDK). THIS IS NOT AVAILABLE YET

2)  Modify Controller for Map View

  - Extend ViewController and add MapView manipulation code into it. _Ensure it is Objective C++, not plain Objective C class_

  - Rename `ViewController.m` (comes with template) to `ViewController.mm`, to avoid compilation issues

  -  Implement **ViewController.h** to extend **GLKViewController**, instead of
    UIViewController

  - You must replace `YOUR_LICENSE_KEY` with your [Mobile Apps and API Key](#registering-your-mobile-app) in the code below

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

  <span class="wrap-border"><img src="/docs/img/layout/mobile/xcode_storyboard.jpg" alt="Xcode storyboard" /></span>

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

### Xamarin (Android and iOS) Implementation

If using Xamarin as the mobile platform, follow these implementation procedures for Xamarin (Android) and Xamarin (iOS).

1. Add library as nuget [CartoMobileSDK](https://www.nuget.org/packages/CartoMobileSDK/) from the main repo, and add to your mobile app

2. [Register your mobile app](#registering-your-mobile-app) to get the API Key

    **Note:** If you are using both Xamarin Android and iOS, register each platform as its own app.

3. Create a cross-platform project for your apps

    Each platform still needs to be registered as its own app, since many app aspects (such as UI, file system, and so on) are platform-specific. However, when executing API requests with the Mobile SDK, you can create one Xamarin project for Android and iOS and share the code. _Some exceptions apply in regards to API calls which need Android context, or file system references._ For example, the following API requests are platform specific:

    - Register license key: `MapView.RegisterLicense()`
    - Create package manager: `new CartoPackageManager()`

    Almost all of the map related API code (such as adding layers and objects to map, handling interactions and clicks, etc.) can be shared for iOS and Android through one project!

### Xarmin Forms Apps

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

  You can use the [sample app](#android-samples) resources for your assets. This is required for vector basemaps

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

2) **Copy vector style file** (*osmbright.zip*) to your project. You can get it from our Sample Apps. This is needed for vector basemap.

3) **Add Map object to app view**. When using Storyboards, use *OpenGL ES View Controller* (GLKit.GLKViewController) as a template for the map and replace *GLKView* with *MapView* as the underlying view class.
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

  Add the following code after creating the [MapView](#xamarin-android-and-ios-implementation).

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

<br/><br/>**Tip:** See [mobile-dotnet-samples](https://github.com/CartoDB/mobile-dotnet-samples/) for other common map options for the Xamarin mobile platform, such as:

- **Control map view** - set zoom, center, tilt etc.
- **Listen events** (MapListener.cs) of clicks to map and map objects
- **Add other objects**: Lines, Polygons, Points, Balloons (callouts). You can even add 3D objects and use customized Balloons
- **Download offline map packages** for a country, or smaller region

### Windows Phone Implementation

If using Windows Phone as the mobile platform, follow this implementation procedure.

_**Note:** The mobile development team is still testing Windows Phone implementation with the Mobile SDK._ Please [contact us](mailto:support@carto.com) if you run into any issues.

1. Install [Visual Studio Extension Package](https://msdn.microsoft.com/en-us/library/bb166441.aspxf) from Windows

    The following requirements are mandatory:

    - Windows Phone version 10
    - MS Visual Studio 2013 Community edition, or better
    - Windows Phone 10 SDK, should come with Visual Studio
    - Visual Studio extension (VSIX) for CARTO Maps SDK component. Download and start the package to install it

2. [Register your app](#registering-your-mobile-app) and select _Windows Phone_ as the app type

    - Ensure you enter the same application ID as your *Package.appmanifest > Packaging > Package name*. For example, the [sample app](#windows-phone-implementation) ID is **c882d38a-5c09-4994-87f0-89875cdee539**

3. Create a cross-platform project for your Windows Phone app

    You can create one .Net project for Android, iOS, Windows Phone and share map-related code. Each platform still needs to be registered as its own app, since many app aspects (such as UI, file system, and so on) are platform-specific. However, when executing API requests with the Mobile SDK, you can create one project for adding layers and objects to map, handling interactions and click, and so on.

    **Tip:** .Net [sample app](#xamarin-and-windows-phone-samples) contains two solutions: one for Windows Phone and another for Xamarin, and they share one project _hellomap-shared_ with map-related code.

#### Create a WP App

Follow these steps in order to create a Windows Phone (WP) mobile application.

1) Ensure you have the CARTO Visual Studio Extension installed, and your app project has Internet connection

  In the *Solution Explorer References* section, add *Carto Maps SDK for Windows Phone*. You will find it from the Windows Phone 8.1 extensions. We do not have NuGet package yet, please [let us know](mailto:support@carto.com) if this is something that interests you.

2) Copy vector style file (*osmbright.zip*) to your project *Assets* folder, available from the [Sample Apps](#xamarin-and-windows-phone-samples). This is needed for vector basemaps

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

#### Add a Marker (WP)

To create a map marker at a defined coordinate on a Windows Phone mobile app, add following code (after creating a [MapView](#basic-map-features).

**Note:** You must have an *Icon.png* bitmap, located in the Assets folder of your project

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
</div>

<br></br>**Tip:** See [mobile-dotnet-samples](https://github.com/CartoDB/mobile-dotnet-samples/) sample project (solution: *hellomap-winphone.sln*) for other WP map actions, such as:

- **Control map view** - set zoom, center, tilt etc.
- **Listen events** (MapListener.cs) of clicks to map and map objects
- **Add other objects**: Lines, Polygons, Points, Balloons (callouts). You can even add 3D objects and use customized Balloons.
- **Download offline map packages** for country or smaller region

## Basic Map Features

This section describes the basic map components that are required when creating mobile apps with the SDK. The following key objects are required for each mobile app:

-  **MapView** is the object which goes to your app view hierarchy. It provides methods for directly manipulating the map and its view. For example, you can set center location, zoom, and so on.  MapViews contain objects for the map layers and map options

    - **Layer** (member of MapView) is an object that provides methods for adding data to the map

      - **DataSource** (member of Layers) is an object created as a member of the Layer object, which defines where the data comes from. There are specific DataSource implementations which keep data in memory, load from a persistent file or on-line API

    -  **Objects** (member of MapView) is an object that provides methods for adjusting the map parameters

### Layers

Map data is organized by **Layers**, which are needed for rendering your visualization. The usual approach is to add one **TileLayer** as a general background then add one or more interactive **VectorLayers** on top of the first layer. You can then include _Markers_, _Texts_, _Popups_ and other map features on the top layer.

#### DataSource

For mobile maps, each map layer is required to have a **DataSource**, which defines where the layer retrieves data. Several common data source implementations are built directly into the Mobile SDK, but you can also define your own data sources using the following parameters. 

- `HTTPRasterTileDataSource` is used for retrieving map tiles as images over HTTP connection

- `LocalVectorDataSource` stores data in memory and is used for adding vector objects to the map, per each user session

**Tip:** For details about selecting different DataSources for your mobile map layers, see [Loading Mobile Data](/docs/carto-engine/mobile-sdk/loading-mobile-data/).

### Basemap

Basemaps apply the map background required for rendering data. Basemaps are required as the bottom layer. You can add a vector layer that contains the background of your mobile map by using the following code: 

<div class="js-TabPanes">
  <ul class="Tabs">
    <li class="Tab js-Tabpanes-navItem--lang is-active">
      <a href="#/0" class="js-Tabpanes-navLink--lang js-Tabpanes-navLink--lang--java">Java</a>
    </li>
    <li class="Tab js-Tabpanes-navItem--lang">
      <a href="#/1" class="js-Tabpanes-navLink--lang js-Tabpanes-navLink--lang--csharp">C#</a>
    </li>
    <li class="Tab js-Tabpanes-navItem--lang">
      <a href="#/2" class="js-Tabpanes-navLink--lang js-Tabpanes-navLink--lang--objective-c">Objective-C</a>
    </li>
    <li class="Tab js-Tabpanes-navItem--lang">
      <a href="#/3" class="js-Tabpanes-navLink--lang js-Tabpanes-navLink--lang--swift">Swift</a>
    </li>
  </ul>

  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--java is-active">
  {% highlight java %}Projection proj = mapView.getOptions().getBaseProjection();

// 1. Initialize an local vector data source
      LocalVectorDataSource vectorDataSource1 = new LocalVectorDataSource(proj);

// 2. Initialize a vector layer with the previous data source
      VectorLayer vectorLayer1 = new VectorLayer(vectorDataSource1);

// 3. Add the previous vector layer to the map
      mapView.getLayers().add(vectorLayer1);

// 4. Set limited visible zoom range for the vector layer
      vectorLayer1.setVisibleZoomRange(new MapRange(10, 24));

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--csharp">
  {% highlight csharp %}Projection proj = MapView.Options.BaseProjection;

// 1. Initialize an local vector data source
      LocalVectorDataSource vectorDataSource1 = new LocalVectorDataSource(proj);

// 2. Initialize a vector layer with the previous data source
      VectorLayer vectorLayer1 = new VectorLayer(vectorDataSource1);

// 3. Add the previous vector layer to the map
      MapView.Layers.Add(vectorLayer1);

// 4. Set limited visible zoom range for the vector layer
      vectorLayer1.VisibleZoomRange = new MapRange(10, 24);

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--objective-c">
  {% highlight objc %}

// this was already initialized before
  NTEPSG3857* proj = [[NTEPSG3857 alloc] init];

// 1. Initialize a local vector data source
      NTLocalVectorDataSource* vectorDataSource1 = [[NTLocalVectorDataSource alloc] initWithProjection:proj];
  
// 2. Initialize a vector layer with the previous data source
      NTVectorLayer* vectorLayer1 = [[NTVectorLayer alloc] initWithDataSource:vectorDataSource1];
  
// 3. Add the previous vector layer to the map
      [[self getLayers] add:vectorLayer1];
  
// 4. Set visible zoom range for the vector layer
      [vectorLayer1 setVisibleZoomRange:[[NTMapRange alloc] initWithMin:10 max:24]];

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--swift">
  {% highlight swift %}COMING SOON...

  {% endhighlight %}
  </div>
    
</div>

### MapView Objects

In following examples, **vector elements** (Markers, Points, Lines, Polygons, Texts and BalloonPopups) are added to a mobile map application. For each object, the styling is defined and objects are created based on given coordinates. These coordinates are store in the memory-based vector data source parameter, `LocalVectorDataSource`.

- Before adding any MapView objects, you must create a [VectorLayer](#layers) as the data source and add the layer to the map

**Note:** A popup (callout, bubble) which appears when you click on map is a map object of its own, and should be added using object click listener. For details, see [Add a BalloonPopup](#add-a-balloonpopup).

### Add a Marker

Add a marker and apply marker styling using the following code:

<div class="js-TabPanes">
  <ul class="Tabs">
    <li class="Tab js-Tabpanes-navItem--lang is-active">
      <a href="#/0" class="js-Tabpanes-navLink--lang js-Tabpanes-navLink--lang--java">Java</a>
    </li>
    <li class="Tab js-Tabpanes-navItem--lang">
      <a href="#/1" class="js-Tabpanes-navLink--lang js-Tabpanes-navLink--lang--csharp">C#</a>
    </li>
    <li class="Tab js-Tabpanes-navItem--lang">
      <a href="#/2" class="js-Tabpanes-navLink--lang js-Tabpanes-navLink--lang--objective-c">Objective-C</a>
    </li>
    <li class="Tab js-Tabpanes-navItem--lang">
      <a href="#/3" class="js-Tabpanes-navLink--lang js-Tabpanes-navLink--lang--swift">Swift</a>
    </li>
  </ul>

  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--java is-active">
  {% highlight java %}

// 1. Create marker style
      MarkerStyleBuilder markerStyleBuilder = new MarkerStyleBuilder();
      markerStyleBuilder.setSize(30);
      // Green colour as ARGB
      markerStyleBuilder.setColor(new Color(0xFF00FF00));

      MarkerStyle sharedMarkerStyle = markerStyleBuilder.buildStyle();

// 2. Add marker
      Marker marker1 = new Marker(proj.fromWgs84(new MapPos(24.646469, 59.426939)), sharedMarkerStyle);
      marker1.setMetaDataElement("ClickText", "Marker nr 1");
      vectorDataSource1.add(marker1);

// 3. Animate map to the marker
      mapView.setFocusPos(tallinn, 1);
      mapView.setZoom(12, 1);

{% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--csharp">
  {% highlight csharp %}

// 1. Create marker style
      MarkerStyleBuilder markerStyleBuilder = new MarkerStyleBuilder();
      markerStyleBuilder.Size = 30;

// 2. Build style
      MarkerStyle sharedMarkerStyle = markerStyleBuilder.BuildStyle();

// 3. Set marker position
      MapPos tallinn = proj.FromWgs84(new MapPos(24.646469, 59.426939));

// 4. Add marker
      Marker marker1 = new Marker(tallinn, sharedMarkerStyle);
      marker1.SetMetaDataElement("ClickText", new Variant("Marker nr 1"));

// 5. Animate map to the marker
      MapView.SetFocusPos(tallinn, 1);
      MapView.SetZoom(12, 1);

      vectorDataSource1.Add(marker1);
    
  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--objective-c">
  {% highlight objc %}

// 1. Create a marker style, we use default marker bitmap here
      NTMarkerStyleBuilder* markerStyleBuilder = [[NTMarkerStyleBuilder alloc] init];
      // Styles use dpi-independent units, no need to adjust it for retina
     [markerStyleBuilder setSize:30];
      // Green colour using ARGB format
      [markerStyleBuilder setColor:[[NTColor alloc] initWithColor:0xFF00FF00]]; 
      NTMarkerStyle* sharedMarkerStyle = [markerStyleBuilder buildStyle];

// 2. Define position and metadata for marker. Two important notes:
      // (1) Position in latitude/longitude has to be converted using projection
      // (2) X is longitude, Y is latitude !
      NTMapPos* pos = [proj fromWgs84:[[NTMapPos alloc] initWithX:24.651488 y:59.423581]];
      NTMarker* marker1 = [[NTMarker alloc] initWithPos:pos style:sharedMarkerStyle];
  
// 3. The defined metadata will be used later for Popups
      [marker1 setMetaDataElement:@"ClickText" element:[[NTVariant alloc] initWithString:@"Marker 1"]];
  
      [vectorDataSource1 add:marker1];

      [self.mapView setFocusPos:pos  durationSeconds:0]
      [self.mapView setZoom:12 durationSeconds:1];

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--swift">
  {% highlight swift %}COMING SOON...

  {% endhighlight %}
  </div>
</div>

#### Example Marker on a Mobile Map

![pin](https://dl.dropboxusercontent.com/u/3573333/public_web/developersite/pin.png)

### Add a Point

Points are used to indicating specific location points on a map, similar to Markers. However, Points do not have overlapping controls and cannot be use with billboard style version 2.5D. If you have a lot of data (thousands of points) and are not using 2.5D views, use Points as an alternative to Markers. Your rendering time will be significantly faster.

You can add any type of vector objects to the same Layer and `UnculledVectorDataSource`. This enables you to reuse settings for a defined Marker. It is recommended to define different Layers and DataSources for managing your objects, as it allows you to:

-  Select and delete all objects of DataSource

-  Specify the drawing order of layers. The drawing order within a single Layer is undefined. For multiple layers, layers that are added are drawn underneath the initial map layer

Add a point and apply point styling using the following code:

<div class="js-TabPanes">
  <ul class="Tabs">
    <li class="Tab js-Tabpanes-navItem--lang is-active">
      <a href="#/0" class="js-Tabpanes-navLink--lang js-Tabpanes-navLink--lang--java">Java</a>
    </li>
    <li class="Tab js-Tabpanes-navItem--lang">
      <a href="#/1" class="js-Tabpanes-navLink--lang js-Tabpanes-navLink--lang--csharp">C#</a>
    </li>
    <li class="Tab js-Tabpanes-navItem--lang">
      <a href="#/2" class="js-Tabpanes-navLink--lang js-Tabpanes-navLink--lang--objective-c">Objective-C</a>
    </li>
    <li class="Tab js-Tabpanes-navItem--lang">
      <a href="#/3" class="js-Tabpanes-navLink--lang js-Tabpanes-navLink--lang--swift">Swift</a>
    </li>
  </ul>

  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--java is-active">
  {% highlight java %}

// 1. Set marker position
      MapPos tallinn = proj.FromWgs84(new MapPos(24.646469, 59.426939));

// 2. Create style and position for the Point
      PointStyleBuilder pointStyleBuilder = new PointStyleBuilder();
      pointStyleBuilder.setColor(new Color(0xFF00FF00));
      pointStyleBuilder.setSize(16);

// 3. Create Point, add to datasource with metadata
      Point point1 = new Point(tallinn, pointStyleBuilder.buildStyle());
      point1.setMetaDataElement("ClickText", "Point nr 1");

      vectorDataSource1.add(point1);
  
// 4. Animate map to the point
      mapView.setFocusPos(tallinn, 1);
      mapView.setZoom(12, 1);
  
  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--csharp">
  {% highlight csharp %}

// 1. Set point position
      MapPos tallinn = proj.FromWgs84(new MapPos(24.646469, 59.426939));

// 2. Create style and position for the Point
      var pointStyleBuilder = new PointStyleBuilder();
      pointStyleBuilder.Color = new Color(0, 255, 0, 255);
      pointStyleBuilder.Size = 16;

// 3. Create Point, add to datasource with metadata
      Point point1 = new Point(tallinn, pointStyleBuilder.BuildStyle());
      point1.SetMetaDataElement("ClickText", new Variant("Point nr 1"));

      vectorDataSource1.Add(point1);

// 4. Animate map to the point
      MapView.SetFocusPos(tallinn, 1);
      MapView.SetZoom(12, 1);

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--objective-c">
  {% highlight objc %}

// 1. Create style and position for the Point
      NTPointStyleBuilder* pointStyleBuilder = [[NTPointStyleBuilder alloc] init];

      //color is defined as ARGB integer, i.e. following is opaque green
      //you can not use UIColor (or any other ObjectiveC specific class) in Carto mobile SDK
     
      [pointStyleBuilder setColor:[[NTColor alloc] initWithColor:0xFF00FF00]];
      [pointStyleBuilder setSize:16];
      NTMapPos* pos = [proj fromWgs84:[[NTMapPos alloc] initWithX:24.651488 y:59.423581]];

// 2. Create Point, add to datasource with metadata
      NTPoint* point1 = [[NTPoint alloc] initWithPos:pos style:[pointStyleBuilder buildStyle]];
      [point1 setMetaDataElement:@"ClickText" element:[[NTVariant alloc] initWithString:@"Point 1"]];
      [vectorDataSource1 add:point1];

      [self.mapView setFocusPos:pos  durationSeconds:0]
      [self.mapView setZoom:12 durationSeconds:1];

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--swift">
  {% highlight swift %}COMING SOON...

  {% endhighlight %}
  </div>

</div>

### Add a Line

Lines can be added to the same `vectorDataSource1`. Add a line and apply line styling using the following code:

<div class="js-TabPanes">
  <ul class="Tabs">
    <li class="Tab js-Tabpanes-navItem--lang is-active">
      <a href="#/0" class="js-Tabpanes-navLink--lang js-Tabpanes-navLink--lang--java">Java</a>
    </li>
    <li class="Tab js-Tabpanes-navItem--lang">
      <a href="#/1" class="js-Tabpanes-navLink--lang js-Tabpanes-navLink--lang--csharp">C#</a>
    </li>
    <li class="Tab js-Tabpanes-navItem--lang">
      <a href="#/2" class="js-Tabpanes-navLink--lang js-Tabpanes-navLink--lang--objective-c">Objective-C</a>
    </li>
    <li class="Tab js-Tabpanes-navItem--lang">
      <a href="#/3" class="js-Tabpanes-navLink--lang js-Tabpanes-navLink--lang--swift">Swift</a>
    </li>
  </ul>

  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--java is-active">
  {% highlight java %}

// 1. Create line style, and line poses
      LineStyleBuilder lineStyleBuilder = new LineStyleBuilder();
      lineStyleBuilder.setColor(new Color(0xFFFFFFFF));
      lineStyleBuilder.setLineJointType(LineJointType.LINE_JOINT_TYPE_ROUND);
      lineStyleBuilder.setWidth(8);

// 2. Special MapPosVector must be used for coordinates
      MapPosVector linePoses = new MapPosVector();
      MapPos initial = proj.fromWgs84(new MapPos(24.645565, 59.422074));
  
// 3. Add positions
      linePoses.add(initial);
      linePoses.add(proj.fromWgs84(new MapPos(24.643076, 59.420502)));
      linePoses.add(proj.fromWgs84(new MapPos(24.645351, 59.419149)));
      linePoses.add(proj.fromWgs84(new MapPos(24.648956, 59.420393)));
      linePoses.add(proj.fromWgs84(new MapPos(24.650887, 59.422707)));

// 4. Add a line
      Line line1 = new Line(linePoses, lineStyleBuilder.buildStyle());
      line1.setMetaDataElement("ClickText", "Line nr 1");
      vectorDataSource1.add(line1);
  
// 5. Animate map to the line
      mapView.setFocusPos(tallinn, 1);
      mapView.setZoom(12, 1);

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--csharp">
  {% highlight csharp %}

// 1. Create line style, and line poses
      var lineStyleBuilder = new LineStyleBuilder();
      lineStyleBuilder.LineJoinType = LineJoinType.LineJoinTypeRound;
      lineStyleBuilder.Width = 8;
      lineStyleBuilder.Color = new Color(255, 0, 0, 255); // Red

      var positions = new MapPosVector();
      MapPos initial = proj.FromWgs84(new MapPos(24.645565, 59.422074));

// 2. Add positions
      positions.Add(initial);
      positions.Add(proj.FromWgs84(new MapPos(24.643076, 59.420502)));
      positions.Add(proj.FromWgs84(new MapPos(24.645351, 59.419149)));
      positions.Add(proj.FromWgs84(new MapPos(24.648956, 59.420393)));
      positions.Add(proj.FromWgs84(new MapPos(24.650887, 59.422707)));

// 3. Add line to source
      var line = new Line(positions, lineStyleBuilder.BuildStyle());
      vectorDataSource1.Add(line);

// 4. Animate zoom to the line
      MapView.SetFocusPos(initial, 1);
      MapView.SetZoom(15, 1);

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--objective-c">
  {% highlight objc %}

// 1. Define line style
      NTLineStyleBuilder* lineStyleBuilder = [[NTLineStyleBuilder alloc] init];
        // White color, opaque
      [lineStyleBuilder setColor:[[NTColor alloc] initWithColor:0xFFFFFFFF]];
      [lineStyleBuilder setLineJointType:NT_LINE_JOINT_TYPE_ROUND];
      [lineStyleBuilder setWidth:8];

// 2. Define line positions, here as fixed locations
      MapPosVector* linePoses = [[MapPosVector alloc] init];
      NTMapPos* initial = [proj fromWgs84:[[NTMapPos alloc] initWithX:24.645565 y:59.422074]]

      [linePoses add:initial];
      [linePoses add:[proj fromWgs84:[[NTMapPos alloc] initWithX:24.643076 y:59.420502]]];
      [linePoses add:[proj fromWgs84:[[NTMapPos alloc] initWithX:24.645351 y:59.419149]]];
      [linePoses add:[proj fromWgs84:[[NTMapPos alloc] initWithX:24.648956 y:59.420393]]];
      [linePoses add:[proj fromWgs84:[[NTMapPos alloc] initWithX:24.650887 y:59.422707]]];

// 3. Create line, add metadata and add to the datasource
      NTLine* line1 = [[NTLine alloc] initWithGeometry:[[NTLineGeometry alloc] initWithPoses:linePoses] style:[lineStyleBuilder buildStyle]];
      [line1 setMetaDataElement:@"ClickText" element:[[NTVariant alloc] initWithString:@"Line 1"]];
      [vectorDataSource1 add:line1];

      [self.mapView setFocusPos:initial  durationSeconds:0]
      [self.mapView setZoom:15 durationSeconds:1];
  
  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--swift">
  {% highlight swift %}COMING SOON...

  {% endhighlight %}
  </div>
  
</div>

#### Example Line on a Mobile Map

![line](https://dl.dropboxusercontent.com/u/3573333/public_web/developersite/line.png)

### Add a Polygon

Add a polygon and apply polygon styling using the following code. The following examples add a polygon with polygon holes:

<div class="js-TabPanes">
  <ul class="Tabs">
    <li class="Tab js-Tabpanes-navItem--lang is-active">
      <a href="#/0" class="js-Tabpanes-navLink--lang js-Tabpanes-navLink--lang--java">Java</a>
    </li>
    <li class="Tab js-Tabpanes-navItem--lang">
      <a href="#/1" class="js-Tabpanes-navLink--lang js-Tabpanes-navLink--lang--csharp">C#</a>
    </li>
    <li class="Tab js-Tabpanes-navItem--lang">
      <a href="#/2" class="js-Tabpanes-navLink--lang js-Tabpanes-navLink--lang--objective-c">Objective-C</a>
    </li>
    <li class="Tab js-Tabpanes-navItem--lang">
      <a href="#/3" class="js-Tabpanes-navLink--lang js-Tabpanes-navLink--lang--swift">Swift</a>
    </li>
  </ul>

  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--java is-active">
  {% highlight java %}

// 1. Create polygon style and poses
      PolygonStyleBuilder polygonStyleBuilder = new PolygonStyleBuilder();
      polygonStyleBuilder.setColor(new Color(0xFFFF0000));
      lineStyleBuilder = new LineStyleBuilder();
      lineStyleBuilder.setColor(new Color(0xFF000000));
      lineStyleBuilder.setWidth(1.0f);
      polygonStyleBuilder.setLineStyle(lineStyleBuilder.buildStyle());

      MapPosVector polygonPoses = new MapPosVector();
      MapPos initial = proj.fromWgs84(new MapPos(24.650930, 59.421659));
      polygonPoses.add(initial);
      polygonPoses.add(proj.fromWgs84(new MapPos(24.657453, 59.416354)));
      polygonPoses.add(proj.fromWgs84(new MapPos(24.661187, 59.414607)));
      polygonPoses.add(proj.fromWgs84(new MapPos(24.667667, 59.418123)));
      polygonPoses.add(proj.fromWgs84(new MapPos(24.665736, 59.421703)));
      polygonPoses.add(proj.fromWgs84(new MapPos(24.661444, 59.421245)));
      polygonPoses.add(proj.fromWgs84(new MapPos(24.660199, 59.420677)));
      polygonPoses.add(proj.fromWgs84(new MapPos(24.656552, 59.420175)));
      polygonPoses.add(proj.fromWgs84(new MapPos(24.654010, 59.421472)));
        
// 2. Create 2 polygon holes
      MapPosVector holePoses1 = new MapPosVector();
      holePoses1.add(proj.fromWgs84(new MapPos(24.658409, 59.420522)));
      holePoses1.add(proj.fromWgs84(new MapPos(24.662207, 59.418896)));
      holePoses1.add(proj.fromWgs84(new MapPos(24.662207, 59.417411)));
      holePoses1.add(proj.fromWgs84(new MapPos(24.659524, 59.417171)));
      holePoses1.add(proj.fromWgs84(new MapPos(24.657615, 59.419834)));
      MapPosVector holePoses2 = new MapPosVector();
      holePoses2.add(proj.fromWgs84(new MapPos(24.665640, 59.421243)));
      holePoses2.add(proj.fromWgs84(new MapPos(24.668923, 59.419463)));
      holePoses2.add(proj.fromWgs84(new MapPos(24.662893, 59.419365)));
      MapPosVectorVector polygonHoles = new MapPosVectorVector();
          
      polygonHoles.add(holePoses1);
      polygonHoles.add(holePoses2);

// 3. Add polygon
      Polygon polygon = new Polygon(polygonPoses, polygonHoles, polygonStyleBuilder.buildStyle());
        polygon.setMetaDataElement("ClickText", new Variant("Polygon"));
        vectorDataSource1.add(polygon);

// 4. Animate zoom to position
      mapView.setFocusPos(initial, 1);
      mapView.setZoom(13, 1);

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--csharp">
  {% highlight csharp %}

// 1. Create polygon style and poses
      PolygonStyleBuilder polygonStyleBuilder = new PolygonStyleBuilder();
      polygonStyleBuilder.Color = new Color(255, 0, 0, 255); // red
        var lineStyleBuilder = new LineStyleBuilder();
        lineStyleBuilder.Color = new Color(0, 0, 0, 255); // black
        lineStyleBuilder.Width = 1.0f;
        polygonStyleBuilder.LineStyle = lineStyleBuilder.BuildStyle();

// 2. Define coordinates of outer ring
      MapPosVector polygonPoses = new MapPosVector();
      MapPos initial = proj.FromWgs84(new MapPos(24.650930, 59.421659));
      polygonPoses.Add(initial);
      polygonPoses.Add(proj.FromWgs84(new MapPos(24.657453, 59.416354)));
      polygonPoses.Add(proj.FromWgs84(new MapPos(24.661187, 59.414607)));
      polygonPoses.Add(proj.FromWgs84(new MapPos(24.667667, 59.418123)));
      polygonPoses.Add(proj.FromWgs84(new MapPos(24.665736, 59.421703)));
      polygonPoses.Add(proj.FromWgs84(new MapPos(24.661444, 59.421245)));
      polygonPoses.Add(proj.FromWgs84(new MapPos(24.660199, 59.420677)));
      polygonPoses.Add(proj.FromWgs84(new MapPos(24.656552, 59.420175)));
      polygonPoses.Add(proj.FromWgs84(new MapPos(24.654010, 59.421472)));

// 3. Create polygon holes poses, note that special MapPosVectorVector must be used
      MapPosVectorVector polygonHoles = new MapPosVectorVector();

      MapPosVector hole1 = new MapPosVector();
      hole1.Add(proj.FromWgs84(new MapPos(24.658409, 59.420522)));
      hole1.Add(proj.FromWgs84(new MapPos(24.658409, 59.420522)));
      hole1.Add(proj.FromWgs84(new MapPos(24.662207, 59.418896)));
      hole1.Add(proj.FromWgs84(new MapPos(24.662207, 59.417411)));
      hole1.Add(proj.FromWgs84(new MapPos(24.659524, 59.417171)));
      hole1.Add(proj.FromWgs84(new MapPos(24.657615, 59.419834)));

      MapPosVector hole2 = new MapPosVector();
      hole2.Add(proj.FromWgs84(new MapPos(24.665640, 59.421243)));
      hole2.Add(proj.FromWgs84(new MapPos(24.668923, 59.419463)));
      hole2.Add(proj.FromWgs84(new MapPos(24.662893, 59.419365)));

      polygonHoles.Add(hole1);
      polygonHoles.Add(hole2);

// 4. Add polygon
      Polygon polygon = new Polygon(polygonPoses, polygonHoles, polygonStyleBuilder.BuildStyle());
        polygon.SetMetaDataElement("ClickText", new Variant("Polygon"));
        vectorDataSource1.Add(polygon);

// 5. Animate zoom to position
      MapView.SetFocusPos(initial, 1);
      MapView.SetZoom(13, 1);

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--objective-c">
  {% highlight objc %}

// 1. Create polygon style
      NTPolygonStyleBuilder* polygonStyleBuilder = [[NTPolygonStyleBuilder alloc] init];
         // polygon fill color: opaque red
      [polygonStyleBuilder setColor:[[NTColor alloc] initWithColor:0xFFFF0000]];
         // define polygon outline style as line style
      lineStyleBuilder = [[NTLineStyleBuilder alloc] init];
         // opaque black color
      [lineStyleBuilder setColor:[[NTColor alloc] initWithColor:0xFF000000]];
      [lineStyleBuilder setWidth:1.0f];
      [polygonStyleBuilder setLineStyle:[lineStyleBuilder buildStyle]];

// 2. Define polygon coordinates
      // First define outline as MapPosVector, which is array of MapPos
      // We cannot use Objective C objects, like NSArray in CARTO mobile SDK, 
      // so there are special objects for collections
      MapPosVector* polygonPoses = [[MapPosVector alloc] init];
      NTMapPos* initial = [proj fromWgs84:[[NTMapPos alloc] initWithX:24.650930 y:59.421659]];

      [polygonPoses add:initial];
      [polygonPoses add:[proj fromWgs84:[[NTMapPos alloc] initWithX:24.657453 y:59.416354]]];
      [polygonPoses add:[proj fromWgs84:[[NTMapPos alloc] initWithX:24.661187 y:59.414607]]];
      [polygonPoses add:[proj fromWgs84:[[NTMapPos alloc] initWithX:24.667667 y:59.418123]]];
      [polygonPoses add:[proj fromWgs84:[[NTMapPos alloc] initWithX:24.665736 y:59.421703]]];
      [polygonPoses add:[proj fromWgs84:[[NTMapPos alloc] initWithX:24.661444 y:59.421245]]];
      [polygonPoses add:[proj fromWgs84:[[NTMapPos alloc] initWithX:24.660199 y:59.420677]]];
      [polygonPoses add:[proj fromWgs84:[[NTMapPos alloc] initWithX:24.656552 y:59.420175]]];
      [polygonPoses add:[proj fromWgs84:[[NTMapPos alloc] initWithX:24.654010 y:59.421472]]];

      // Define polygon holes. This is two-dimensional array (MapPosVectorVector)
      // because Polygon can have several holes. In this sample there are two
      MapPosVectorVector* holes = [[MapPosVectorVector alloc] init];
      [holes add:[[MapPosVector alloc] init]];
      [[holes get:0] add:[proj fromWgs84:[[NTMapPos alloc] initWithX:24.658409 y:59.420522]]];
      [[holes get:0] add:[proj fromWgs84:[[NTMapPos alloc] initWithX:24.662207 y:59.418896]]];
      [[holes get:0] add:[proj fromWgs84:[[NTMapPos alloc] initWithX:24.662207 y:59.417411]]];
      [[holes get:0] add:[proj fromWgs84:[[NTMapPos alloc] initWithX:24.659524 y:59.417171]]];
      [[holes get:0] add:[proj fromWgs84:[[NTMapPos alloc] initWithX:24.657615 y:59.419834]]];
      [holes add:[[MapPosVector alloc] init]];
      [[holes get:1] add:[proj fromWgs84:[[NTMapPos alloc] initWithX:24.665640 y:59.421243]]];
      [[holes get:1] add:[proj fromWgs84:[[NTMapPos alloc] initWithX:24.668923 y:59.419463]]];
      [[holes get:1] add:[proj fromWgs84:[[NTMapPos alloc] initWithX:24.662893 y:59.419365]]];

// 3. Create polygon, define metadata and add to datasource
      NTPolygon* polygon = [[NTPolygon alloc] initWithGeometry:[[NTPolygonGeometry alloc] initWithPoses:polygonPoses holes:holes] style:[polygonStyleBuilder buildStyle]];
      [polygon setMetaDataElement:@"ClickText" element:[[NTVariant alloc] initWithString:@"Polygon"]];
      [vectorDataSource1 add:polygon];

      [self.mapView setFocusPos:initial  durationSeconds:0]
      [self.mapView setZoom:13 durationSeconds:1];

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--swift">
  {% highlight swift %}COMING SOON...

  {% endhighlight %}
  </div>
  
</div>

#### Example Polygon on a Mobile Map

![polygon](https://dl.dropboxusercontent.com/u/3573333/public_web/developersite/polygon.png)

### Add Text

Text style parameters are similar to Markers, as both are Billboards - which are MapView objects that contain the following features:

- Control and prohibit text overlapping, based on priority and the location of where the text appears

- Display text as billboards in 2.5D (tilted) view by defining the `OrientationMode` parameter. There are 3 options: show on ground, and rotate with map (like street names), show on ground (do not rotate with map), or show as billboard (no rotation).

Add text and apply text styling using the following code.

<div class="js-TabPanes">
  <ul class="Tabs">
    <li class="Tab js-Tabpanes-navItem--lang is-active">
      <a href="#/0" class="js-Tabpanes-navLink--lang js-Tabpanes-navLink--lang--java">Java</a>
    </li>
    <li class="Tab js-Tabpanes-navItem--lang">
      <a href="#/1" class="js-Tabpanes-navLink--lang js-Tabpanes-navLink--lang--csharp">C#</a>
    </li>
    <li class="Tab js-Tabpanes-navItem--lang">
      <a href="#/2" class="js-Tabpanes-navLink--lang js-Tabpanes-navLink--lang--objective-c">Objective-C</a>
    </li>
    <li class="Tab js-Tabpanes-navItem--lang">
      <a href="#/3" class="js-Tabpanes-navLink--lang js-Tabpanes-navLink--lang--swift">Swift</a>
    </li>
  </ul>

  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--java is-active">
  {% highlight java %}

// 1. Create text style
      TextStyleBuilder textStyleBuilder = new TextStyleBuilder();
      textStyleBuilder.setColor(new Color(0xFFFF0000));
      textStyleBuilder.setOrientationMode(BillboardOrientation.BILLBOARD_ORIENTATION_FACE_CAMERA);

      // This enables higher resolution texts for retina devices, but consumes more memory and is slower
      textStyleBuilder.setScaleWithDPI(false);

// 2. Add text
      MapPos position = proj.fromWgs84(new MapPos(24.653302, 59.422269));
      Text textpopup1 = new Text(position, textStyleBuilder.buildStyle(), "Face camera text");
      textpopup1.setMetaDataElement("ClickText", "Text nr 1");
      vectorDataSource1.add(textpopup1);
  
// 3. Animate zoom to position
      mapView.setFocusPos(position, 1);
      mapView.setZoom(13, 1);

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--csharp">
  {% highlight csharp %}

// 1. Create text style
      TextStyleBuilder textStyleBuilder = new TextStyleBuilder();
      textStyleBuilder.setColor(new Color(0xFFFF0000));
      textStyleBuilder.setOrientationMode(BillboardOrientation.BILLBOARD_ORIENTATION_FACE_CAMERA);

      // This enables higher resolution texts for retina devices, but consumes more memory and is slower
      textStyleBuilder.setScaleWithDPI(false);

// 2. Add text
      MapPos position = proj.fromWgs84(new MapPos(24.653302, 59.422269));
      Text textpopup1 = new Text(position, textStyleBuilder.buildStyle(), "Face camera text");
      textpopup1.setMetaDataElement("ClickText", new Variant("Text nr 1"));
  
      vectorDataSource1.add(textpopup1);

// 3. Animate zoom to position
      MapView.SetFocusPos(position, 1);
      MapView.SetZoom(13, 1);

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--objective-c">
  {% highlight objc %}

// 4. Create text style
      NTTextStyleBuilder* textStyleBuilder = [[NTTextStyleBuilder alloc] init];
      [textStyleBuilder setColor:[[NTColor alloc] initWithColor:0xFFFF0000]];
      [textStyleBuilder setOrientationMode:NT_BILLBOARD_ORIENTATION_FACE_CAMERA];
  
      // setScaleWithDPI enables higher resolution texts for retina devices,
      // but consumes more memory and is slower if you have many texts on map
      [textStyleBuilder setScaleWithDPI:false];

// 5. Define text location and add to datasource
      NTMapPos* pos = [proj fromWgs84:[[NTMapPos alloc] initWithX:24.653302 y:59.422269]];
      NTText* text1 = [[NTText alloc] initWithPos:pos style:[textStyleBuilder buildStyle] text:@"Face camera text"];
  
      [text1 setMetaDataElement:@"ClickText" element:[[NTVariant alloc] initWithString:@"Text 1"]];

      [vectorDataSource1 add:text1];

      [self.mapView setFocusPos:pos  durationSeconds:0]
      [self.mapView setZoom:13 durationSeconds:1];

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--swift">
  {% highlight swift %}COMING SOON...

  {% endhighlight %}
  </div>
  
</div>

#### Example Text on a Mobile Map

![text](https://dl.dropboxusercontent.com/u/3573333/public_web/developersite/text.png)

### Add a BalloonPopup

A BalloonPopup appears based on click event of an object. You can also add a defined BalloonPopup. Add BalloonPops using the following code. Note that these examples contain several special styling elements, such as:

- Image (_info.png_) as the "Left Image"
- Arrow (_arrow.png_)  as the "Right Image"
- Styled appearance of tuned radius values

**Tip:** To use these styling elements as part of your own project, copy the above .png files, (available from the [Sample Apps](#sample-apps)), to your local map application project.

<div class="js-TabPanes">
  <ul class="Tabs">
    <li class="Tab js-Tabpanes-navItem--lang is-active">
      <a href="#/0" class="js-Tabpanes-navLink--lang js-Tabpanes-navLink--lang--java">Java</a>
    </li>
    <li class="Tab js-Tabpanes-navItem--lang">
      <a href="#/1" class="js-Tabpanes-navLink--lang js-Tabpanes-navLink--lang--csharp">C#</a>
    </li>
    <li class="Tab js-Tabpanes-navItem--lang">
      <a href="#/2" class="js-Tabpanes-navLink--lang js-Tabpanes-navLink--lang--objective-c">Objective-C</a>
    </li>
    <li class="Tab js-Tabpanes-navItem--lang">
      <a href="#/3" class="js-Tabpanes-navLink--lang js-Tabpanes-navLink--lang--swift">Swift</a>
    </li>
  </ul>

  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--java is-active">
  {% highlight java %}

// 1. Load bitmaps to show on the label
      Bitmap infoImage = BitmapFactory.decodeResource(getResources(), R.drawable.info);
      Bitmap arrowImage = BitmapFactory.decodeResource(getResources(), R.drawable.arrow);

// 2. Add popup
      BalloonPopupStyleBuilder builder = new BalloonPopupStyleBuilder();
      builder.setCornerRadius(20);
      builder.setLeftMargins(new BalloonPopupMargins(6, 6, 6, 6));
      builder.setLeftImage(BitmapUtils.createBitmapFromAndroidBitmap(infoImage));
      builder.setRightImage(BitmapUtils.createBitmapFromAndroidBitmap(arrowImage));
      builder.setRightMargins(new BalloonPopupMargins(2, 6, 12, 6));
      builder.setPlacementPriority(1);

      MapPos position = proj.fromWgs84(new MapPos(24.655662, 59.425521));
      BalloonPopup popup = new BalloonPopup(position, builder.buildStyle(), "Popup with pos", "Images, round");
        popup.setMetaDataElement("ClickText", new Variant("Popup caption nr 1"));

        vectorDataSource1.add(popup);

        mapView.setFocusPos(position, 1);
        mapView.setZoom(13, 1);

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--csharp">
  {% highlight csharp %}

// Android
      Bitmap androidInfoBitmap = BitmapFactory.DecodeResource(Resources, HelloMap.Resource.Drawable.info);
      Carto.Graphics.Bitmap infoBitmap = BitmapUtils.CreateBitmapFromAndroidBitmap(androidInfoBitmap);

      Bitmap androidArrowBitmap = BitmapFactory.DecodeResource(Resources, HelloMap.Resource.Drawable.arrow);
      Carto.Graphics.Bitmap arrowBitmap = BitmapUtils.CreateBitmapFromAndroidBitmap(androidArrowBitmap);

// iOS
      var infoBitmap = BitmapUtils.CreateBitmapFromUIImage(UIImage.FromFile("info.png"));
      var arrowBitmap = BitmapUtils.CreateBitmapFromUIImage(UIImage.FromFile("arrow.png"));

      var builder = new BalloonPopupStyleBuilder();
      builder.LeftImage = infoBitmap;
      builder.RightImage = arrowBitmap;
      builder.LeftMargins = new BalloonPopupMargins(6, 6, 6, 6);
      builder.RightMargins = new BalloonPopupMargins(2, 6, 12, 6);
      builder.CornerRadius = 20;
      builder.TitleFontName = "Helvetica";
      builder.TitleFontSize = 16;
      builder.TitleColor = new Color(200, 0, 0, 255);
      builder.StrokeColor = new Color(200, 120, 0, 255);
      builder.StrokeWidth = 1;
      builder.PlacementPriority = 1;

      MapPos position = proj.FromWgs84(new MapPos(0, 20));
      var popup = new BalloonPopup(position, builder.BuildStyle(), "Popup Title", "Description");

      vectorDataSource1.Add(popup);

      // Animate zoom to position
      MapView.SetFocusPos(position, 1);
      MapView.SetZoom(13, 1);

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--objective-c">
  {% highlight objc %}

// 1. Load styling bitmaps to show on the popups
      UIImage* infoImage = [UIImage imageNamed:@"info.png"];
      UIImage* arrowImage = [UIImage imageNamed:@"arrow.png"];
  
// 2. Create style for the BalloonPopup
      NTBalloonPopupStyleBuilder* balloonPopupStyleBuilder = [[NTBalloonPopupStyleBuilder alloc] init];
      [balloonPopupStyleBuilder setCornerRadius:20];
      [balloonPopupStyleBuilder setLeftMargins:[[NTBalloonPopupMargins alloc] initWithLeft:6 top:6 right:6 bottom:6]];
      [balloonPopupStyleBuilder setLeftImage:infoImage];
      [balloonPopupStyleBuilder setRightImage:arrowImage];
      [balloonPopupStyleBuilder setRightMargins:[[NTBalloonPopupMargins alloc] initWithLeft:2 top:6 right:12 bottom:6]];
    
    // Higher priority ensures that baloon is visible when overlaps with other billboards
  [balloonPopupStyleBuilder setPlacementPriority:1];
 
// 3. Sefine location
      NTMapPos* pos = [proj fromWgs84:[[NTMapPos alloc] initWithX:24.655662 y:59.425521]];

// 4. Create BalloonPopup and add to datasource
      NTBalloonPopup* popup1 = [[NTBalloonPopup alloc] initWithPos:pos
                               style:[balloonPopupStyleBuilder buildStyle]
                               title:@"Popup with pos"
                              desc:@"Images, round"];
  
      [popup1 setMetaDataElement:@"ClickText" element:[[NTVariant alloc] initWithString:@"Popup caption 1"]];
      
      [vectorDataSource1 add:popup1];

      [self.mapView setFocusPos:pos  durationSeconds:0]
      [self.mapView setZoom:13 durationSeconds:1];

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--swift">
  {% highlight swift %}COMING SOON...

  {% endhighlight %}
  </div>
  
</div>

#### Example BalloonPopup on a Mobile Map

![popup](https://dl.dropboxusercontent.com/u/3573333/public_web/developersite/popup.png)

### Add 3D Model Objects

One special feature of the Mobile SDK is that you can add 3D objects (models) to a mobile map. For example, you can add small moving car or other decorative or informative elements.

**Note:** 3D objects are added to the same `LocalVectorDataSource` as 2D objects. However, 3D objects are only supported using the *Nutiteq Markup Language* (**NML**) format, as it was created by our [mobile partner, Nutiteq](https://carto.com/engine/mobile/). This custom format is optimized for the multi-resolution of 3D files on mobile apps. _For details about tuning the performance of 3D models, see [`LocalVectorDataSource` Performance](#localvectordatasource-performance)._

The following procedure describes how to setup and add a 3D object to your mobile MapView:

1. Select a _NML file_

    **Tip:** You can retrieve some free samples from [Nutiteq's NHL sample page](https://github.com/nutiteq/hellomap3d/wiki/NML-model-samples).

    If you have own model as Collada DAE (or KMZ) format, then you would need **Carto Mobile 3D converter tools** to convert it to NML, so it can be used in mobile. Please [contact us](mailto:sales@carto.com) for more information.

2. Adjust the file size of the 3D object for rendering

    **Note:** Typically, NML files are smaller than 100K. Anything larger than that takes too long to render. Consider applying lower detail models for your 3D objects. You can then add these models to the **res/raw** folder of your Android app, or as a **resource file** in your iOS project.

3. Load the model file in the code to add it to your map application by using the following code:

<div class="js-TabPanes">
  <ul class="Tabs">
    <li class="Tab js-Tabpanes-navItem--lang is-active">
      <a href="#/0" class="js-Tabpanes-navLink--lang js-Tabpanes-navLink--lang--java">Java</a>
    </li>
    <li class="Tab js-Tabpanes-navItem--lang">
      <a href="#/1" class="js-Tabpanes-navLink--lang js-Tabpanes-navLink--lang--csharp">C#</a>
    </li>
    <li class="Tab js-Tabpanes-navItem--lang">
      <a href="#/2" class="js-Tabpanes-navLink--lang js-Tabpanes-navLink--lang--objective-c">Objective-C</a>
    </li>
    <li class="Tab js-Tabpanes-navItem--lang">
      <a href="#/3" class="js-Tabpanes-navLink--lang js-Tabpanes-navLink--lang--swift">Swift</a>
    </li>
  </ul>

  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--java is-active">
  {% highlight java %}

// 1. Load NML model from a file
      UnsignedCharVector modelFile = AssetUtils.loadBytes("fcd_auto.nml");

// 2. Set location for model, and create NMLModel object with this
      MapPos modelPos = baseProjection.fromWgs84(new MapPos(24.646469, 59.423939));
      NMLModel model = new NMLModel(modelPos, modelFile);

// 3. Adjust the size- oversize it by 20*, just to make it more visible (optional)
      model.setScale(20);

// 4. Add metadata for click handling (optional)
      model.setMetaDataElement("ClickText", new Variant("Single model"));

// 5. Add it to normal datasource
      vectorDataSource1.add(model);

      mapView.setFocusPos(position, 1);
      mapView.setZoom(15, 1);

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--csharp">
  {% highlight csharp %}

      var file = AssetUtils.LoadAsset("fcd_auto.nml");

// 1. Set location for model, and create NMLModel object with this
      var position = proj.FromWgs84(new MapPos(24.646469, 59.423939));
      var model = new NMLModel(position, file);

// 2. Adjust the size- oversize it by 20x, just to make it more visible (optional)
      model.Scale = 20;

// 3. Add metadata for click handling (optional)
      model.SetMetaDataElement("ClickText", new Variant("Single model"));

// 4. Add it to normal datasource
      vectorDataSource1.Add(model);

// 5. Animate zoom to position
      MapView.SetFocusPos(position, 1);
      MapView.SetZoom(15, 1);

  {% endhighlight %}
  </div>

<div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--objective-c">
  {% highlight objc %}

// 1. Load NML model from a file
      NTUnsignedCharVector* modelData = [NTAssetUtils loadBytes:@"fcd_auto.nml"];
    
// 2. Set location for model, and create NMLModel object with this
      NTMapPos* pos = [proj fromWgs84:[[NTMapPos alloc] initWithX:24.646469 y:59.424939]];
      NTNMLModel* model = [[NTNMLModel alloc] initWithPos:pos sourceModelData:modelData];
    
// 3. Add metadata for click handling (optional)
      [model setMetaDataElement:@"ClickText" element:[[NTVariant alloc] initWithString:@"My nice car"]];    
  
// 4. Adjust the size- oversize it by 20*, just to make it more visible (optional)
      [model setScale:20];

      [vectorDataSource1 add:model];

      [self.mapView setFocusPos:pos  durationSeconds:0]
      [self.mapView setZoom:15 durationSeconds:1];

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--swift">
  {% highlight swift %}COMING SOON...

  {% endhighlight %}
  </div>
  
</div>

#### Example 3D Model Object on a Mobile Map

![3d animated](https://developer.nutiteq.com/images/animated3d.gif)

#### `LocalVectorDataSource` Performance

You can define the `spatialIndexType` of the `LocalVectorDataSource` to improve the performance of how a 3D model loads, by defining a spatial index to the DataSource. By default, no spatial index is defined. The following rules apply:

-  Do not define a spatial index if the number of elements is small, below ~1000 points or markers (or even less if you have complex lines and polygons). _This is the default option_

-  Apply `NT_LOCAL_SPATIAL_INDEX_TYPE_KDTREE` as the index type if there are a larger number of elements 

The advantage of defining a spatial index is that CPU usage decreases for large number of objects, improving the map performance of panning and zooming. However, displaying overlays may slightly delay the map response, as the spatial index is not loaded immediately when your move the map, it only moves after some hundred milliseconds. 

The overall maximum number of objects on map is limited to the RAM available for the app. Systems define several hundred MB for iOS apps, and closer to tens of MB for Android apps, but it depends on the device and app settings (as well as the density of the data). It is recommended to test your app with the targeted mobile platform and full dataset for the actual performance. 

The following code describes how to adjust the `LocalVectorDataSource` performance by defining a spatial index:

<div class="js-TabPanes">
  <ul class="Tabs">
    <li class="Tab js-Tabpanes-navItem--lang is-active">
      <a href="#/0" class="js-Tabpanes-navLink--lang js-Tabpanes-navLink--lang--java">Java</a>
    </li>
    <li class="Tab js-Tabpanes-navItem--lang">
      <a href="#/1" class="js-Tabpanes-navLink--lang js-Tabpanes-navLink--lang--csharp">C#</a>
    </li>
    <li class="Tab js-Tabpanes-navItem--lang">
      <a href="#/2" class="js-Tabpanes-navLink--lang js-Tabpanes-navLink--lang--objective-c">Objective-C</a>
    </li>
    <li class="Tab js-Tabpanes-navItem--lang">
      <a href="#/3" class="js-Tabpanes-navLink--lang js-Tabpanes-navLink--lang--swift">Swift</a>
    </li>
  </ul>

  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--java is-active">
  {% highlight java %}LocalVectorDataSource vectorDataSource2 = new LocalVectorDataSource(proj, LocalSpatialIndexType.LOCAL_SPATIAL_INDEX_TYPE_KDTREE);

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--csharp">
  {% highlight csharp %}

var vectorDataSource2 = new LocalVectorDataSource(proj, LocalSpatialIndexType.LocalSpatialIndexTypeKdtree);

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--objective-c">
  {% highlight objc %}NTLocalVectorDataSource* vectorDataSource2 =
    [[NTLocalVectorDataSource alloc] initWithProjection:proj
                                       spatialIndexType: NTLocalSpatialIndexType::NT_LOCAL_SPATIAL_INDEX_TYPE_KDTREE];

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--swift">
  {% highlight swift %}COMING SOON...

  {% endhighlight %}
  </div>
</div>

<br/><br/>**Note:** If you have **very complex lines or polygons**, this creates objects with high numbers (more than hundreds of points per object) of vertexes. For example, the GPS traces for long periods of time, recording a point every second or so. 

Spatial indexing will not help in this case, as you need to show the whole track on the screen. In this case, apply the **automatic line simplification** parameter for the `LocalVectorDataSource`. This reduces the number of polygon and line points, while maintaining the original object shape. 

**Tip:** Automatic simplification is zoom-dependent. When a map is zoomed out, more aggressive simplification is used. When you zoom in, less simplification is applied until you see the original details.

There are several ways to simplify geometries for automatic simplification by using the following code. Note that simplification is applied in two passes:

- First pass applies fast Radial Distance vertex rejection
- Second pass applies Ramer-Douglas-Peuckerworst algorithm (with the worst case quadratic complexity)

All this is can be applied with just one line of code - as shown in the following example (under iOS) for minimal 1-pixel simplification, the result is invisible but effects are significant, depending on your DataSource:

<div class="js-TabPanes">
  <ul class="Tabs">
    <li class="Tab js-Tabpanes-navItem--lang is-active">
      <a href="#/0" class="js-Tabpanes-navLink--lang js-Tabpanes-navLink--lang--java">Java</a>
    </li>
    <li class="Tab js-Tabpanes-navItem--lang">
      <a href="#/1" class="js-Tabpanes-navLink--lang js-Tabpanes-navLink--lang--csharp">C#</a>
    </li>
    <li class="Tab js-Tabpanes-navItem--lang">
      <a href="#/2" class="js-Tabpanes-navLink--lang js-Tabpanes-navLink--lang--objective-c">Objective-C</a>
    </li>
    <li class="Tab js-Tabpanes-navItem--lang">
      <a href="#/3" class="js-Tabpanes-navLink--lang js-Tabpanes-navLink--lang--swift">Swift</a>
    </li>
  </ul>

  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--java is-active">
  {% highlight java %}vectorDataSource2.setGeometrySimplifier(new DouglasPeuckerGeometrySimplifier(1.0f / 320.0f));

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--csharp">
  {% highlight csharp %}vectorDataSource2.GeometrySimplifier = new DouglasPeuckerGeometrySimplifier(1.0f / 320.0f);

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--objective-c">
  {% highlight objc %}[vectorDataSource2 setGeometrySimplifier:
  [[NTDouglasPeuckerGeometrySimplifier alloc] initWithTolerance: 1.0f / 320.0f]];

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--swift">
  {% highlight swift %}COMING SOON...

  {% endhighlight %}
  </div>
  
</div>

The automatic simplification renders a GPU tasks faster with some additional computation (CPU). For aggressive simplification, you will notice a decrease in the of line quality, so use is sparingly.

## Sample Apps

Once you have installed the mobile dependencies and have installed the required libraries, the following sample mobile applications can be used to test apps on multiple mobile platforms. Some of these samples include some useful style files that you can use with your Mobile SDK projects.

### Android Samples

* [https://github.com/CartoDB/mobile-android-samples](https://github.com/CartoDB/mobile-android-samples) - samples as sources

### iOS Samples

* [https://github.com/CartoDB/mobile-ios-samples](https://github.com/CartoDB/mobile-ios-samples), see sample screenshot of what should appear:

### Xamarin and Windows Phone Samples

* [https://github.com/CartoDB/mobile-dotnet-samples](https://github.com/CartoDB/mobile-dotnet-samples)
