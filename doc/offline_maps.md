# Offline map service

## Introduction
Carto provides service to download base map packages in special vector format for offline use. There is no direct URL to get these packages, instead we provide cross-platform API from within SDK: **Package Manager API**. Application should use this API to download packages. The API provides following features:

* Listing of available packages (countries), get IDs and metadata of specific packages
* Start package download
* Pause and resume download
* Delete downloaded package
* Get status report of package state and download status (% downloaded)

Usage of API requires registration of license code.

### Typical scenarios
<img src = "http://share.gifyoutube.com/yaNw0r.gif" alt="Offline browser" align="right">

Currently we support following scenarios:

1. Provide **interactive list of all packages** to the user, who selects a country and downloads predefined map package. From app development point of view this is most complex, as you need to create nice UX. See advancedmap sample for it
2. You define **one specific country** what user wants to download. Your application provides UI button to start download; or even starts download automatically when user first starts your app. This is much simpler to implement, as there is no need for list and status handling. See below for example code for it.
3. Download **bounding box** map area. This is suitable for e.g. city guide apps where whole country is not needed. You can even let user to define own area of interest. See below example code for it.

### Available packages
* [List of available packages](/guides/packages) - see available package ID-s for country package downloads

## Usage

### Example code - complex scenario
We provide ready-made *advancedmap* examples: [**PackageManagerActivity**](https://github.com/nutiteq/hellomap3d-android/blob/master/com.nutiteq.advancedmap3/src/com/nutiteq/advancedmap3/PackageManagerActivity.java) for Android, and [**PackageMapController**](https://github.com/nutiteq/hellomap3d-ios/blob/master/advancedmap3/advancedmap3/PackageManagerController.mm) for iOS. These cover the more complex case - listing of all packages, and user can select one or many of them, and view the map.

### Event diagram
Following flow diagram displays key events and messages between app, Package Manager and online service.

![packagemanager flow](/images/pm_flow.png)


### Additional events
Application must handle properly following possible additional asynchronous events:

* *onPackageFailed* - package download failed, e.g. network connection issue
* *onPackageCancelled* - download canceled by application
* *onPackageListFailed* - package list download failed, probably network issue


### Steps for single package download
Following are steps to implement **single map package** downloading

* **Initialize CartoPackageManager**

<div id="tabs1">
  <ul>
    <li><a href="#i1"><span>iOS ObjC</span></a></li>
    <li><a href="#a1"><span>Android Java</span></a></li>
    <li><a href="#n1"><span>.NET C#</span></a></li>
  </ul>
<div id="i1">
<pre class="brush: objc">
    // Register license, this must be done before PackageManager can be used!
    [NTMapView registerLicense:"YOUR_LICENSE_HERE"];

    // Create folder for package manager files. Package manager needs persistent writable folder.
	NSArray* paths = NSSearchPathForDirectoriesInDomains(NSApplicationSupportDirectory, NSUserDomainMask,YES);
	NSString* appSupportDir = [paths objectAtIndex: 0];
	NSString* packagesDir = [appSupportDir stringByAppendingString:@"packages"];
	NSError *error;
	[[NSFileManager defaultManager] createDirectoryAtPath:packagesDir withIntermediateDirectories:NO attributes:nil error:&error];

	// Create package manager and package manager listener
	// we had defined packageManager and packageManagerListener already, as properties
	// currently the only package data source is nutiteq.mbstreets, it has OpenStreetMap global data
	_packageManager = [[NTCartoPackageManager alloc] initWithSource:@"nutiteq.mbstreets" dataFolder:packagesDir];
    [_packageManager start];
</pre>
</div>
<div id="a1">
<pre class="brush: java">
 // Register license, this must be done before PackageManager can be used!
 MapView.registerLicense("YOUR_LICENSE_HERE", getApplicationContext());
		
 // Create package manager
 File packageFolder = new File(getApplicationContext().getExternalFilesDir(null), "mappackages");
 if (!(packageFolder.mkdirs() || packageFolder.isDirectory())) {
    Log.e(Const.LOG_TAG, "Could not create package folder!");
 }
 packageManager = new CartoPackageManager("nutiteq.mbstreets", packageFolder.getAbsolutePath());
 packageManager.start();

</pre>
</div>
<div id="n1">
<pre class="brush: csharp">
 // 1) Android ONLY: Register license, this must be done before PackageManager can be used!
 MapView.RegisterLicense("YOUR-NUTITEQ-LICENSE", ApplicationContext);
 
 // Create/find folder for packages
 var packageFolder = new File (GetExternalFilesDir(null), "packages");
 if (!(packageFolder.Mkdirs() || packageFolder.IsDirectory)) {
   Log.Fatal("Could not create package folder!");
 }


 // 2) iOS ONLY: Register license, this must be done before PackageManager can be used!
 MapView.RegisterLicense("YOUR-NUTITEQ-LICENSE");

 // Find folder for packages
 var paths = NSSearchPath.GetDirectories (NSSearchPathDirectory.ApplicationSupportDirectory, NSSearchPathDomain.User);
 var packageFolder = paths [0] + "packages";
 NSFileManager.DefaultManager.CreateDirectory (packageFolder, true, null);

// Following code is identical for all platforms        

// Create package manager

// define PackageManager listener, definition is in same class above
var packageManager = new CartoPackageManager("nutiteq.mbstreets", packageFolder);
packageManager.PackageManagerListener = new PackageListener(packageManager);

// Download new package list only if it is older than 24h
// Note: this is only needed if pre-made packages are used
if (packageManager.ServerPackageListAge > 24 * 60 * 60) {
  packageManager.StartPackageListDownload ();
}

// Start manager - mandatory
packageManager.Start ();

</pre>
</div>
</div>

* **Implement and set PackageManagerListener** 

<div id="tabs2">
  <ul>
    <li><a href="#i2"><span>iOS ObjC</span></a></li>
    <li><a href="#a2"><span>Android Java</span></a></li>
    <li><a href="#n2"><span>.NET C#</span></a></li>
  </ul>
<div id="i2">
<pre class="brush: objc">

    // 1. define own listener object
    
/*
 * Package manager listener. Listener is notified about asynchronous events
 * about packages.
 */
@interface PackageManagerListener : NTPackageManagerListener

- (void)onPackageListUpdated;
- (void)onPackageListFailed;
- (void)onPackageUpdated:(NSString*)packageId version:(int)version;
- (void)onPackageCancelled:(NSString*)packageId version:(int)version;
- (void)onPackageFailed:(NSString*)packageId version:(int)version errorType:(NTPackageErrorType)errorType;
- (void)onPackageStatusChanged:(NSString*)packageId version:(int)version status:(NTPackageStatus*)status;

@property(readonly, atomic) NSHashTable* packageManagerControllers;

@end


    // 2. create listener implementation
    
    
@implementation PackageManagerListener


- (void)onPackageListUpdated
{
  // called when package list is downloaded
  // now you can start downloading packages
 
}

- (void)onPackageListFailed
{
  // called when package list download is failed (network connection)
}

- (void)onPackageUpdated:(NSString*)packageId version:(int)version
{
  // called when package is updated
}

- (void)onPackageCancelled:(NSString*)packageId version:(int)version
{
  // called when you called cancel package download
}

- (void)onPackageFailed:(NSString*)packageId version:(int)version errorType:(NTPackageErrorType)errorType
{
 // error in package download
}

- (void)onPackageStatusChanged:(NSString*)packageId version:(int)version status:(NTPackageStatus*)status
{
  // a portion of package is downloaded. Update your progress bar here.
}

@end

    // following to your Controller class:

    // 3. create listener object instance
	_packageManagerListener = [[PackageManagerListener alloc] init];

	// 4. Register this controller with listener to receive notifications about events
	[_packageManagerListener addPackageManagerController:self];

	// 5. Attach package manager listener
	[_packageManager setPackageManagerListener:_packageManagerListener];
</pre>
</div>
<div id="a2">
<pre class="brush: java">
   packageManager.setPackageManagerListener(new PackageListener());
   packageManager.startPackageListDownload();
   
   
   // PackageListener.java :
	/**
	 * Listener for package manager events.
	 */
	class PackageListener extends PackageManagerListener {
		@Override
		public void onPackageListUpdated() {
		  // called when package list is downloaded
		  // now you can start downloading packages
		}

		@Override
		public void onPackageListFailed() {
		  // Failed to download package list
		}

		@Override
		public void onPackageStatusChanged(String id, int version, PackageStatus status) {
		  // a portion of package is downloaded. Update your progress bar here.
		}

		@Override
		public void onPackageCancelled(String id, int version) {
		  // called when you called cancel package download
		}

		@Override
		public void onPackageUpdated(String id, int version) {
		  // called when package is updated
		}

		@Override
		public void onPackageFailed(String id, int version, PackageErrorType errorType) {
		  // Failed to download package " + id + "/" + version
		}
	}
</pre>
</div>
<div id="n2">
<pre class="brush: csharp">

// to your main class:
packageManager.PackageManagerListener = new PackageListener(packageManager);
if (packageManager.ServerPackageListAge > 24 * 60 * 60) {
		packageManager.StartPackageListDownload ();
}

// PackageListener.cs:

using Carto.PackageManager;
using Carto.Utils;

namespace HelloMap
{

	public class PackageListener : PackageManagerListener
	{
		private PackageManager _packageManager;

		public PackageListener (PackageManager packageManager)
		{
			_packageManager = packageManager;
		}

		public override void OnPackageListUpdated ()
		{
			// called when package list is downloaded
			// now you can start downloading packages
			Log.Debug ("OnPackageListUpdated");

			// to make sure that package list is updated, full package download is called here
			if (_packageManager.GetLocalPackage ("EE") == null) {
				_packageManager.StartPackageDownload ("EE");
			}
			 
		}

		public override void OnPackageListFailed ()
		{
			Log.Debug ("OnPackageListFailed");
			// Failed to download package list
		}

		public override void OnPackageStatusChanged (string id, int version, PackageStatus status)
		{
			// a portion of package is downloaded. Update your progress bar here.
			// Notice that the view and SDK are in different threads, so data copy id needed
			Log.Debug ("OnPackageStatusChanged " + id + " ver " + version + " progress " + status.Progress);
		}

		public override void OnPackageCancelled (string id, int version)
		{
			// called when you called cancel package download
			Log.Debug ("OnPackageCancelled");
		}

		public override void OnPackageUpdated (string id, int version)
		{
			// called when package is updated
			Log.Debug ("OnPackageUpdated");
		}

		public override void OnPackageFailed (string id, int version, PackageErrorType errorType)
		{
			// Failed to download package " + id + "/" + version
			Log.Debug ("OnPackageFailed: " + errorType);
		}
	}
}

</pre>
</div>
</div>


*  **Download of country package**

Following starts download of map of Estonia. See [list of available packages](/guides/packages) for available countries. Generally packages are for ISO 2-letter administrative country codes, with two exceptions:

* Some bigger countries: USA, Canada, Russia, Germany and France have one level  subdivision 
* Some countries (UK, Spain) have are both options: whole country, and subdivisions


<div id="tabs3">
  <ul>
    <li><a href="#i3"><span>iOS ObjC</span></a></li>
    <li><a href="#a3"><span>Android Java</span></a></li>
    <li><a href="#n3"><span>.NET C#</span></a></li>
  </ul>
<div id="i3">
<pre class="brush: objc">
- (void)onPackageListUpdated
{
  // called when package list is downloaded
  // now you can start downloading packages
 [_packageManager startPackageDownload:@"EE"];
}
</pre>
</div>
<div id="a3">
<pre class="brush: java">
@Override
public void onPackageListUpdated() {
  // called when package list is downloaded
  // now you can start downloading packages
  packageManager.startPackageDownload("EE");
}
</pre>
</div>
<div id="n3">
<pre class="brush: csharp">
public override void OnPackageListUpdated ()
{
	// called when package list is downloaded
	// now you can start downloading packages
	Log.Debug ("OnPackageListUpdated");

	// to make sure that package list is updated, full package download is called here
	if (_packageManager.GetLocalPackage ("EE") == null) {
		_packageManager.StartPackageDownload ("EE");
	}
 
}
</pre>
</div>
</div>

*  **Download of bounding box**

If you do not need whole country, then you can define smaller area with bounding box. There is limitation of 5000 map tiles per download, depending on latitude it means about 150x150 km area. 

Bouding box is defined in format **bbox(lonMin,latMin,lonMax,latMax)**. You can use nice 3rd party [BoundingBox](http://boundingbox.klokantech.com/) web service to define  areas; use CSV format there to get appropriate format for coordinates.

This download does not require PackageList download, so you can start it right away when PackageManger is created.

<div id="tabs5">
  <ul>
    <li><a href="#i5"><span>iOS ObjC</span></a></li>
    <li><a href="#a5"><span>Android Java</span></a></li>
    <li><a href="#n5"><span>.NET C#</span></a></li>
  </ul>
<div id="i5">
<pre class="brush: objc">
  // get London greater area, near maximum area package size
 [_packageManager startPackageDownload:@"bbox(-0.8164,51.2383,0.6406,51.7402)"];

</pre>
</div>
<div id="a5">
<pre class="brush: java">


// London (about 30MB)
String bbox = "bbox(-0.8164,51.2382,0.6406,51.7401)"; 
if (packageManager.getLocalPackage(bbox) == null) {
	packageManager.startPackageDownload (bbox);
}
</pre>
</div>
<div id="n5">
<pre class="brush: csharp">

// London (about 30MB)
var bbox = "bbox(-0.8164,51.2382,0.6406,51.7401)"; 
if (packageManager.GetLocalPackage(bbox) == null) {
	packageManager.StartPackageDownload (bbox);
}
</pre>
</div>
</div>

* **Add datasource with offline map**

There is special Vector Tile Data Source: **PackageManagerTileDataSource** which needs to be used.

<div id="tabs4">
  <ul>
    <li><a href="#i4"><span>iOS ObjC</span></a></li>
    <li><a href="#a4"><span>Android Java</span></a></li>
    <li><a href="#n4"><span>.NET C#</span></a></li>
  </ul>
<div id="i4">
<pre class="brush: objc">
	// Load vector tile styleset
	NSString* styleAssetName = @"osmbright.zip";

	NTUnsignedCharVector *vectorTileStyleSetData = [NTAssetUtils LoadBytes:styleAssetName];
	NTMBVectorTileStyleSet *vectorTileStyleSet = [[NTMBVectorTileStyleSet alloc] initWithData:vectorTileStyleSetData];

	// Create vector tile decoder using the styleset and update style parameters
	NTMBVectorTileDecoder* vectorTileDecoder = [[NTMBVectorTileDecoder alloc] initWithStyleSet:vectorTileStyleSet];
	[vectorTileDecoder setStyleStringParameter:@"lang" value:@"en"];
	
	// Optional: enable 3D elements
    [self.vectorTileDecoder setStyleBoolParameter:@"buildings3d" value:YES];
    [self.vectorTileDecoder setStyleStringParameter:@"markers3d" value:@"1"];
		
	// Create tile data source from PackageManager
	NTTileDataSource* vectorTileDataSource = [[NTPackageManagerTileDataSource alloc] initWithPackageManager:_packageManager];
	
	// Create vector tile layer, using previously created data source and decoder
	NTVectorTileLayer* baseLayer = [[NTVectorTileLayer alloc] initWithDataSource:vectorTileDataSource decoder:self.vectorTileDecoder];

	// Add vector tile layer as first layer
	[[self getLayers] insert:0 layer:baseLayer];
</pre>
</div>
<div id="a4">
<pre class="brush: java">
 // Create layer with vector styling
UnsignedCharVector styleBytes = AssetUtils.loadBytes("osmbright.zip");
MBVectorTileDecoder vectorTileDecoder = null;
if (styleBytes != null){

// Create style set
  MBVectorTileStyleSet vectorTileStyleSet = new MBVectorTileStyleSet(styleBytes);
  vectorTileDecoder = new MBVectorTileDecoder(vectorTileStyleSet);
}

// create offline datasource from Package Manager
PackageManagerTileDataSource dataSource = new PackageManagerTileDataSource(packageManager);
       
VectorTileLayer baseLayer = new VectorTileLayer(vectorTileDataSource, vectorTileDecoder);
 
mapView.getLayers().add(baseLayer);
</pre>
</div>
<div id="n4">
<pre class="brush: csharp">
// define styling for vector map
UnsignedCharVector styleBytes = AssetUtils.LoadBytes("osmbright.zip");
MBVectorTileDecoder vectorTileDecoder = null;
if (styleBytes != null) {
	// Create style set
	MBVectorTileStyleSet vectorTileStyleSet = new MBVectorTileStyleSet (styleBytes);
	vectorTileDecoder = new MBVectorTileDecoder (vectorTileStyleSet);
} else {
	Log.Error ("Failed to load style data");
}

var baseLayer = new VectorTileLayer(new PackageManagerTileDataSource(packageManager),vectorTileDecoder);
mapView.Layers.Add(baseLayer);
</pre>
</div>
</div>

<script>
		$( "#tabs1" ).tabs();
		$( "#tabs2" ).tabs();
		$( "#tabs3" ).tabs();
		$( "#tabs4" ).tabs();
  		$( "#tabs5" ).tabs();
</script>



### Updating packages


There is no special event or method to check package updates, so updates can be checked and controlled by application using following logic. You can call this logic as soon as you feel appropriate. Carto Maps service has OSM offline package updates approximately once a month, different packages can be updated in different point of time, and with different frequency.

1. Use packagemanager *startPackageListDownload* to get server packages
2. Wait for listener's *onPackageListUpdated* event, now server packages were downloaded
3. Use packageManager *getLocalPackages* to get already downloaded packages (local packages)
4. Iterate list of local packages, check from metadata if server package list has newer version of some
5. If there is newer version, then this package is updated
6. Updated package download is like normal first download

During re-download of same package application shows old map until download is complete. So the update can run in background safely.

### Additional notes

* *startPackageListDownload* method does not need to be called every time when package list is required. In fact, once package list is successfully downloaded, it will remain locally available. But refreshing the contents once a day or perhaps once a week is recommended as some older packages may become available once newer versions are uploaded. *getServerPackageListAge* method can be used to check the age of the package list data.

* Package manager keeps persistent task queue of all submitted requests. Even when it is stopped, downloads will automatically resume when it is started next time.

* It is possible to pause downloads by setting task priority to -1. Downloads will resume once priority is set to non-negative number.

* *PackageInfo* class provides two methods for human-readable package names: *getName* and *getNames*. *getNames* takes language argument and will return localized name of the package (assuming the language is supported, currently only major languages such as English, German, French are supported). *getNames* returns list of names, as some packages can be placed under multiple categories. So, using *getNames* should be preferred over *getName*.

* Each package includes **tile mask**. Tile mask basically lists all tiles of the package up to a certain zoom level (currently 10). Tile mask can be used to quickly find a package corresponding to given tile coordinates or to find a package containing given tile.
