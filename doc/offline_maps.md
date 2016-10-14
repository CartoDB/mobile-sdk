# Offline Maps

For Enterprise accounts with offline feature activated map packages, CARTO provides a service to download map packages in special vector formats for offline use. There is no direct URL to get these packages, instead we provide cross-platform API from within SDK: **Package Manager API**. Your application should use this API to download packages. The API provides the following features:

* Listing of available packages (countries), get IDs and metadata of specific packages
* Start package download
* Pause and resume download
* Delete downloaded package
* Get status report of package state and download status (% downloaded)

Usage of the Package Manager API requires the registration of a license code.

### Typical Use Case

<img src = "http://share.gifyoutube.com/yaNw0r.gif" alt="Offline browser" align="right">

Currently we support the following use cases:

1. Provide **interactive list of all packages** to the user, who selects a country and downloads predefined map package. From app development point of view this is most complex, as you need to create a nice UX

2. You define **one specific country** of what the viewer wants to download. Your application provides UI button to start download; or even starts download automatically when user first starts your app. This is much simpler to implement, as there is no need for list and status handling

3. Download **bounding box** map area. For example, this is suitable for city guide apps where the whole country is not needed. You can even let the viewer define their own area of interest

## Offline Map Packages

Offline map packages enable mobile maps to work without an Internet connection and are downloaded to your local mobile app. There are several map packages that can be downloaded from CARTO for offline use. Your must use the **Package Manager** API to download the packages. _Offline map packages are only available to certain account plans._ [Contact us](mailto:support@carto.com) if you have questions about your Mobile SDK plan.

The following offline packages are available:

-  **Continent/Sub-continent/Country/Country Region**, for general geographical hierarchies

-  **Sub-continent**, only used in Europe, Asia, Africa and North America

-  **Country Region** are used in US, Germany, UK, Spain, Russia and Canada. Spain and UK also have their own individual, offline packages

-  **By Language** - offline packages are also available in following languages: **English (en), German, (de), French (fr), Italian (it), Chinese (zh), Spanish (es), Russian (ru) and Estonian (et)**. [Contact us](mailto:support@carto.com) if you need another language

### CARTO OpenStreetMap Packages

-  When initializing the Package Manager API, use `nutiteq.osm` as the source ID
-  For map data, see the [OpenStreetMap](http://www.openstreetmap.org) contributors, ODbL license
- The Package Name IDs are located through [Nutiteq OpenStreetMap Packages](https://developer.nutiteq.com/guides/packages)

### Offline Routing Packages

See [Mobile Routing](/docs/carto-engine/mobile-sdk/mobile-routing/) for details about the offline routing packages for mobile applications.

## Example code - complex scenario

The Mobile SDK provides pre-packaged advanced map examples: [**PackageManagerActivity**](https://github.com/nutiteq/hellomap3d-android/blob/master/com.nutiteq.advancedmap3/src/com/nutiteq/advancedmap3/PackageManagerActivity.java) for Android, and [**PackageMapController**](https://github.com/nutiteq/hellomap3d-ios/blob/master/advancedmap3/advancedmap3/PackageManagerController.mm) for iOS. You can list all packages and select the example to view the map.

## Event Diagram

The following flow diagram displays key events and messages between the app, Package Manager, and online service.

<span class="wrap-border"><img src="/docs/img/layout/mobile/package_manager_flow.jpg" alt="Package Manager flow" /></span>

## Additional Events

Applications must be able to properly handle any possible asynchronous events:

- *onPackageFailed* - package download failed, e.g. network connection issue

- *onPackageCancelled* - download canceled by application

- *onPackageListFailed* - package list download failed, probably network issue

## Single Package Download

Use the following steps to implement **single map package** downloading:

* **Initialize CartoPackageManager**

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

// 1. Register license, this must be done before PackageManager can be used!
      MapView.registerLicense("YOUR_LICENSE_KEY", getApplicationContext());

// 2. Create package manager
      File packageFolder = new File(getApplicationContext().getExternalFilesDir(null), "mappackages");
        if (!(packageFolder.mkdirs() || packageFolder.isDirectory())) {
            Log.e(Const.LOG_TAG, "Could not create package folder!");
        }
        CartoPackageManager packageManager = null;
        try {
            packageManager = new CartoPackageManager("nutiteq.mbstreets", packageFolder.getAbsolutePath());
        } catch (IOException e) {
            e.printStackTrace();
        }
        packageManager.start();

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--csharp">
  {% highlight csharp %}

// Android: Register license, this must be done before PackageManager can be used!
      Carto.Ui.MapView.RegisterLicense("YOUR-NUTITEQ-LICENSE", ApplicationContext);

      // Create/find folder for packages
      var packageFolder = new File(GetExternalFilesDir(null), "packages");
      if (!(packageFolder.Mkdirs() || packageFolder.IsDirectory))
      {
        Log.Fatal("Could not create package folder!");
      }

// iOS: Register license, this must be done before PackageManager can be used!
      Carto.Ui.MapView.RegisterLicense("YOUR-NUTITEQ-LICENSE");

      // Find folder for packages
      var paths = NSSearchPath.GetDirectories(NSSearchPathDirectory.ApplicationSupportDirectory, NSSearchPathDomain.User);
      var packageFolder = paths[0] + "packages";
      NSFileManager.DefaultManager.CreateDirectory(packageFolder, true, null);

// Following code is identical for all platforms        

      // Create package manager

      // Define PackageManager listener, definition is in same class above
      var packageManager = new CartoPackageManager("nutiteq.mbstreets", packageFolder);
      var downloadedPackage = "";
      packageManager.PackageManagerListener = new PackageListener(packageManager, downloadedPackage);

      // Download new package list only if it is older than 24h
      // Note: this is only needed if pre-made packages are used
      if (packageManager.ServerPackageListAge > 24 * 60 * 60)
      {
        packageManager.StartPackageListDownload();
      }

      // Start manager - mandatory
      packageManager.Start();

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--objective-c">
  {% highlight objc %}

// Create folder for package manager files. Package manager needs persistent writable folder.
    NSArray* paths = NSSearchPathForDirectoriesInDomains(NSApplicationSupportDirectory, NSUserDomainMask,YES);
    NSString* appSupportDir = [paths objectAtIndex: 0];
    NSString* packagesDir = [appSupportDir stringByAppendingString:@"packages"];
    NSError *error;
    [[NSFileManager defaultManager] createDirectoryAtPath:packagesDir withIntermediateDirectories:NO attributes:nil error:&error];
    
// Create package manager and package manager listener
    // we had defined packageManager and packageManagerListener already, as properties
    // currently the only package data source is nutiteq.mbstreets, it has OpenStreetMap global data
    NTCartoPackageManager* packageManager = [[NTCartoPackageManager alloc] initWithSource:@"nutiteq.mbstreets" dataFolder:packagesDir];
    [packageManager start];

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--swift">
  {% highlight swift %}COMING SOON...

  {% endhighlight %}
  </div>
  
</div>

* **Implement and set PackageManagerListener** 

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

// Add the following to your activity:

  packageManager.setPackageManagerListener(new PackageListener());
  packageManager.startPackageListDownload();
   
  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--csharp">
  {% highlight csharp %}

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

// Add the following to your activity/controller:

    packageManager.PackageManagerListener = new PackageListener(packageManager);
    if (packageManager.ServerPackageListAge > 24 * 60 * 60) {
        packageManager.StartPackageListDownload ();
    }

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--objective-c">
  {% highlight objc %}

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

  // 2. Create listener implementation

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
  
  // And the following to your Controller class:

// 3. Create listener object instance
      _packageManagerListener = [[PackageManagerListener alloc] init];

// 4. Register this controller with listener to receive notifications about events
      [_packageManagerListener addPackageManagerController:self];

// 5. Attach package manager listener
      [_packageManager setPackageManagerListener:_packageManagerListener];

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--swift">
  {% highlight swift %}COMING SOON...

  {% endhighlight %}
  </div>
  
</div>

*  **Download of country package**

The following starts download of map of Estonia. See [list of available packages](#offline-map-packages) for available countries. Generally packages are for ISO 2-letter administrative country codes, with two exceptions:

* Some bigger countries: USA, Canada, Russia, Germany and France have one level of subdivision 
* Some countries (UK, Spain) have are both options: whole country, and subdivisions

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

  @Override
  public void onPackageListUpdated() {
    // called when package list is downloaded
    // now you can start downloading packages
    packageManager.startPackageDownload("EE");
  }

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--csharp">
  {% highlight csharp %}

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

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--objective-c">
  {% highlight objc %}

  - (void)onPackageListUpdated
  {
    // called when package list is downloaded
    // now you can start downloading packages
   [_packageManager startPackageDownload:@"EE"];
  }

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--swift">
  {% highlight swift %}COMING SOON...

  {% endhighlight %}
  </div>
  
</div>

*  **Download of bounding box**

If you do not need whole country, then you can define smaller area with bounding box. There is limitation of 5000 map tiles per download, depending on latitude (@ 150x150 km area). 

Bounding box is defined using the following format **bbox(lonMin,latMin,lonMax,latMax)**. You can use this 3rd party [BoundingBox](http://boundingbox.klokantech.com/) web service to define  areas (use CSV format to get the appropriate format for coordinates).

**Note:** This download does not require PackageList download, so you can start it as soon as the PackageManger is created.

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

// London (about 30MB)
  String bbox = "bbox(-0.8164,51.2382,0.6406,51.7401)"; 
  if (packageManager.getLocalPackage(bbox) == null) {
    packageManager.startPackageDownload (bbox);
  }

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--csharp">
  {% highlight csharp %}

// London (about 30MB)
  var bbox = "bbox(-0.8164,51.2382,0.6406,51.7401)"; 
  if (packageManager.GetLocalPackage(bbox) == null) {
    packageManager.StartPackageDownload (bbox);
  }

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--objective-c">
  {% highlight objc %}

// Get London greater area, near maximum area package size
 [_packageManager startPackageDownload:@"bbox(-0.8164,51.2383,0.6406,51.7402)"];

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--swift">
  {% highlight swift %}COMING SOON...

  {% endhighlight %}
  </div>
  
</div>

* **Add datasource with offline map**

This special Vector Tile Data Source must be used: **PackageManagerTileDataSource**.

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

// 1. Create layer with vector styling
        BinaryData styleBytes = AssetUtils.loadAsset("osmbright.zip");
        ZippedAssetPackage assetPackage = new ZippedAssetPackage(styleBytes);

        MBVectorTileDecoder vectorTileDecoder = null;

        if (styleBytes != null) {
            // Create style set
            CompiledStyleSet vectorTileStyleSet = new CompiledStyleSet(assetPackage);
            vectorTileDecoder = new MBVectorTileDecoder(vectorTileStyleSet);
        }

// 2. Create offline datasource from Package Manager
        PackageManagerTileDataSource dataSource = new PackageManagerTileDataSource(packageManager);

        VectorTileLayer baseLayer = new VectorTileLayer(dataSource, vectorTileDecoder);

        mapView.getLayers().add(baseLayer);

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--csharp">
  {% highlight csharp %}

// Define styling for vector map
      BinaryData styleBytes = AssetUtils.LoadAsset("osmbright.zip");
      ZippedAssetPackage assetPackage = new ZippedAssetPackage(styleBytes);

      MBVectorTileDecoder vectorTileDecoder = null;

      if (styleBytes != null)
      {
        // Create style set
        CompiledStyleSet vectorTileStyleSet = new CompiledStyleSet(assetPackage);
        vectorTileDecoder = new MBVectorTileDecoder(vectorTileStyleSet);
      }
      else {
        Log.Error("Failed to load style data");
      }

      var baseLayer = new VectorTileLayer(new PackageManagerTileDataSource(packageManager), vectorTileDecoder);
      MapView.Layers.Add(baseLayer);

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--objective-c">
  {% highlight objc %}

// 1. Load vector tile styleset
      NSString* styleAssetName = @"osmbright.zip";
      
      NTBinaryData *vectorTileStyleSetData = [NTAssetUtils loadAsset:styleAssetName];
      NTZippedAssetPackage * package = [[NTZippedAssetPackage alloc]initWithZipData:vectorTileStyleSetData];
      
      NTCompiledStyleSet *vectorTileStyleSet = [[NTCompiledStyleSet alloc] initWithAssetPackage:package];
    
// 2. Create vector tile decoder using the styleset and update style parameters
      NTMBVectorTileDecoder* vectorTileDecoder = [[NTMBVectorTileDecoder alloc] initWithCompiledStyleSet:vectorTileStyleSet];
      [vectorTileDecoder setStyleParameter:@"lang" value:@"en"];
      
      // Optional: enable 3D elements
      [vectorTileDecoder setStyleParameter:@"buildings3d" value:@"YES"];
      [vectorTileDecoder setStyleParameter:@"markers3d" value:@"1"];
    
// 3. Create tile data source from PackageManager
      NTTileDataSource* vectorTileDataSource = [[NTPackageManagerTileDataSource alloc] initWithPackageManager:_packageManager];
    
// 4. Create vector tile layer, using previously created data source and decoder
      NTVectorTileLayer* baseLayer = [[NTVectorTileLayer alloc] initWithDataSource:vectorTileDataSource decoder:self.vectorTileDecoder];
    
// 5. Add vector tile layer as first layer
      [[mapView getLayers] insert:0 layer:baseLayer];

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--swift">
  {% highlight swift %}COMING SOON...

  {% endhighlight %}
  </div>
  
</div>

## Updating Packages

There is no method to regularly check for event package updates, so updates can be managed in your applications by using the following logic. You can call request this logic on-demand. CARTO maps service contains an OSM offline package, which updates approximately once a month. Different packages can be updated for different frequencies.

The following procedure describes how to manage your event listener packages:

1. Use packagemanager *startPackageListDownload* to request server packages

2. Wait for listener's *onPackageListUpdated* event, and new server packages are downloaded

3. Use packageManager *getLocalPackages* to view the previously downloaded, local packages

4. Iterate a list of local packages, compare the metadata to see if the server package list contains a newer timestamp

5. If there is newer version, the package is automatically pdated

6. The updated package download behaves as an initial download

**Note:** Old maps retain all the original listener events until the new download is completed, ensuring that existing mobile maps will not be interrupted by new downloads.

### Additional Notes

- The *startPackageListDownload* method does not need to be requested every time a package list is required. In fact, once a package list is successfully downloaded, it will saved locally. It is recommended to refresh the package contents once a day, or even once a week. Use the *getServerPackageListAge* method for checking the age of the package list data

- The Package Manager keeps a persistent task queue of all submitted requests. Even when it is stopped, downloads will automatically resume when it is restarted

- You can pause downloads by setting the task priority to -1. Downloads will resume once the priority is set to non-negative number

- *PackageInfo* class provides two methods for human-readable package names: *getNames* and *getName*

  - *getName* takes language argument and returns a localized name of the package (assuming the language is supported). Currently, only the following languages are supported: English, German, French

  - *getNames* returns list of names across multiple categories. Using *getNames* is preferred over *getName*

- Each package includes **tile mask**. Tile mask lists all tiles of the package up to a certain zoom level (currently ten). Tile mask can be used to quickly find a package corresponding to given tile coordinates, or to find a package containing given tile
