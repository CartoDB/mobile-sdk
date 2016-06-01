# Register license key #

You need following steps before using Nutiteq Maps SDK:

* Sign up in [developer.nutiteq.com](http://developer.nutiteq.com), choose the free **Developer plan**.
* Add an application. Select **Xamarin iOS** or **Xamarin Android** as application type, and make sure you enter same application ID as you have in your app.
* Finally you will get license code, which is a long string. This is needed for your code.

If you cover several platforms, you should register separate apps for each.

# Cross-platform apps #

You can create one Xamarin project (solution) for Android and iOS and share most of the code. These still need to be two apps, as many app aspects (UI, file system etc) are platform-specific. From Nutiteq SDK API is the same and your code can be shared, except some specific API calls which need Android *context* or file system references. 

# Android app#

1) **Add Nutiteq SDK component** to your project

2) **Copy vector style file** (*nutibright-v3.zip*) to your project *Assets* folder. You will find it from http://developer.nutiteq.com/download. This is needed for vector basemap.

3) **Add MapView to your application main layout**

```xml
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
```

4) **Create MapView object, add a base layer** 

You you can load layout from axml and load the MapView from Layout, or create it with code. Definition of base layer is enough for minimal map configuration.

```csharp
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
		var baseLayer = new NutiteqOnlineVectorTileLayer("nutibright-v3.zip");

		/// Set online base layer  
		mapView.Layers.Add(baseLayer);
	}
	
```


# iOS app#


1) **Add Nutiteq SDK component** to your project

2) **Copy vector style file** (*osmbright-v3.zip*) to your project. You can take it from samples. This is needed for vector basemap.

3) **Add Map object to app view**. When using Storyboards or XIB, use *GL View* as a view for the map, and *GLK View Controller* with custom Class *MapView*. Here we name it as *Map* . You can also create the MapView in the code.

4) **Initiate map, set base layer**

Add into MainViewController.cs:

```csharp
using Nutiteq.Ui;
using Nutiteq.Layers;
using Nutiteq.DataSources;

partial class MainViewController : GLKit.GLKViewController
{

	public override void ViewDidLoad ()
	{
		base.ViewDidLoad ();

		// Register license BEFORE creating MapView 
		MapView.RegisterLicense("YOUR_LICENSE_KEY");

		/// Online vector base layer
		var baseLayer = new NutiteqOnlineVectorTileLayer("nutibright-v3.zip");

		/// Set online base layer
		Map.Layers.Add(baseLayer);

	}
```



# Android and iOS #

3) **Add a marker** to map, to given coordinates. Add following after creating mapView.

You must have *Icon.png* in your Assets folder to set bitmap

```csharp

		// Create overlay layer for markers
		var proj = new EPSG3857();
		var dataSource = new LocalVectorDataSource (proj);
		var overlayLayer = new VectorLayer (dataSource);
		mapView.Layers.Add (overlayLayer);

		// create Marker style
		var markersStyleBuilder = new MarkerStyleBuilder ();
		markersStyleBuilder.SetSize (20);
		UnsignedCharVector iconBytes = AssetUtils.LoadBytes("Icon.png");
		var bitmap = new Bitmap (iconBytes, true);
		markersStyleBuilder.SetBitmap (bitmap);


		// Marker for London
		var marker = new Marker (proj.FromWgs84(new MapPos(-0.8164,51.2383)), markersStyleBuilder.BuildStyle ());
		dataSource.Add (marker);

```

4) **Other map actions**

See sample code how to:

* Control map view - set zoom, center, tilt etc
* Listen events (see MapListener.cs) of clicks to map and map objects
* Add other objects: Lines, Polygons, Points, Balloons (callouts). You can even add 3D objects and use customized Balloons.
* Download offline map packages for country or smaller region (e.g. a city)
