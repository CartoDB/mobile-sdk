## Package Manager

**Offline maps, routing and geocoding** in the CARTO Mobile SDK require 
that you pre-download map data. For this we provide SDK internal API called
**PackageManager** - this manages offline data packages, which are downloaded from 
CARTO server and used by the SDK for specific features. 

So for offline mapping (geocoding or routing) you need to 

* Create and configure PackageManager instance
* Download map data for the area what you need using the instance
* Create a map layer (geocoding or routing service) by refering to the PackageManager instance

The packages that are once downloaded are kept persistently on the device
and can be updated or removed through the PackageManager as the app requires.

### Create and configure Package Manager

#### 1. Data Source ID

When creating PackageManager, you specify package contents by setting `source` parameter value as following:

* `carto.streets` map data packages (vector tiles) for visual map layer.
* `routing:carto.streets` - routing data packages for Valhalla routing engine
* `geocoding:carto.streets` -  geocoding/address search data packages

All these sources are based on OpenStreetMaps map data.

#### 2. Define Folder

App has to decide to which folder the map files are stored. Typically you use standard file locations there, depending on platform. Note that each package source ID should be stored in a different folder, e.g. `mappackages`, `routingpackages`, `geocodingpackages`.

<div class="js-tabs-mobilesdk">
   <ul class="tab-navigation">
     <li class="tab-navigationItem">
       <a href="#tab-java">Java</a>
     </li>
     <li class="tab-navigationItem">
       <a href="#tab-csharp">C#</a>
     </li>
     <li class="tab-navigationItem">
       <a href="#tab-objectivec">Objective-C</a>
     </li>
     <li class="tab-navigationItem">
       <a href="#tab-swift">Swift</a>
     </li>
     <li class="tab-navigationItem">
       <a href="#tab-kotlin">Kotlin</a>
     </li>
   </ul>

  <div id="tab-java">
    {% highlight java linenos %}

    // Create PackageManager instance for dealing with offline packages
    File packageFolder = new File(getApplicationContext().getExternalFilesDir(null), "foldername");
        
    if (!(packageFolder.mkdirs() || packageFolder.isDirectory())) {
        Log.e(Const.LOG_TAG, "Could not create package folder!");
    }
    
    try {
        packageManager = new CartoPackageManager("<your-package-source>", packageFolder.getAbsolutePath());
    } catch (IOException e) {
        e.printStackTrace();
    }

    {% endhighlight %}
  </div>

  <div id="tab-csharp">
    {% highlight csharp linenos %}
	
    // Create PackageManager instance for dealing with offline packages
    var packageFolder = new File(GetExternalFilesDir(null), "foldername");

    if (!(packageFolder.Mkdirs() || packageFolder.IsDirectory))
    {
        Log.Fatal("Could not create package folder!");
    }

    packageManager = new CartoPackageManager("<your-package-source>", packageFolder);

    {% endhighlight %}
  </div>

  <div id="tab-objectivec">
    {% highlight objc linenos %}

    // Define PackageManger to download offline packages
    // Create folder for package manager. Package manager needs persistent writable folder.
    NSArray* paths = NSSearchPathForDirectoriesInDomains(NSApplicationSupportDirectory, NSUserDomainMask,YES);
    NSString* appSupportDir = [paths objectAtIndex: 0];
    NSString* packagesDir = [appSupportDir stringByAppendingString:@"/foldername"];
    NSError *error;
    [[NSFileManager defaultManager] createDirectoryAtPath:packagesDir withIntermediateDirectories:YES attributes:nil error:&error];
       
    packageManager = [[NTCartoPackageManager alloc] initWithSource:@"<your-package-source>" dataFolder:packagesDir];

    {% endhighlight %}
  </div>

  <div id="tab-swift">
    {% highlight swift linenos %}

    // Define PackageManger to download offline packages
    // Create folder for package manager. Package manager needs persistent writable folder.
    let packageFolder = NTAssetUtils.calculateWritablePath("foldername")

    do {
        try FileManager.default.createDirectory(atPath: packageFolder!, withIntermediateDirectories: false, attributes: nil)
    } catch let error as NSError {
        print(error.localizedDescription);
    }

    // Create PackageManager instance for dealing with offline packages
    var packageManager =  NTCartoPackageManager(source: "<your-package-source>", dataFolder: packageFolder)

    {% endhighlight %}
  </div>

  <div id="tab-kotlin">
    {% highlight kotlin linenos %}
  
    // Create PackageManager instance for dealing with offline packages
    val packageFolder = File(applicationContext.getExternalFilesDir(null), "foldername")

    if (!(packageFolder.mkdirs() || packageFolder.isDirectory())) {
        println("Could not create package folder!")
    }

    var packageManager = try {
        CartoPackageManager("<your-package-source>", packageFolder.absolutePath)
    } catch (e: IOException) {
        null
    }

    {% endhighlight %}
  </div>
  
</div>
 
#### 3. Configure PackageManagerListener for Events

Package downloads for country packages cannot be started immediately, as the Mobile SDK needs to get latest definition of packages from CARTO online service. Once this list is received, PackageManagerListener's `onPackageListUpdated()` is called.

Therefore you should write your own `PackageManagerListener`, and start package download using the `onPackageListUpdated` method, which ensures that the package metadata is downloaded.

You see that `onPackageListUpdated()` callback starts immediately download of some packages, as we are quite impatient here. Real download starts when initialization is done and PackageManager is started.

<div class="js-tabs-mobilesdk">
   <ul class="tab-navigation">
     <li class="tab-navigationItem">
       <a href="#tab-java">Java</a>
     </li>
     <li class="tab-navigationItem">
       <a href="#tab-csharp">C#</a>
     </li>
     <li class="tab-navigationItem">
       <a href="#tab-objectivec">Objective-C</a>
     </li>
     <li class="tab-navigationItem">
       <a href="#tab-swift">Swift</a>
     </li>
     <li class="tab-navigationItem">
       <a href="#tab-kotlin">Kotlin</a>
     </li>
   </ul>

  <div id="tab-java">
    {% highlight java linenos %}
  
    public class MyPackageManagerListener extends PackageManagerListener {
        PackageManager packageManager;

        public MyPackageManagerListener(PackageManager manager) {
            packageManager = manager;
        }

        @Override
        public void onPackageListUpdated() {
            Log.d(Const.LOG_TAG, "Package list updated");
            // Start download of package of Estonia. You can call several package downloads here
            // see list of available ID-s: https://github.com/CartoDB/mobile-sdk/wiki/List-of-Offline-map-packages
            packageManager.startPackageDownload("EE");
            packageManager.startPackageDownload("LV");
        }

        @Override
        public void onPackageListFailed() {
            Log.e(Const.LOG_TAG, "Package list update failed. Network connection issues ? ");
        }

        @Override
        public void onPackageStatusChanged(String id, int version, PackageStatus status)
        {
            // you can monitor download process %
        }

        @Override
        public void onPackageCancelled(String id, int version) {
        }

        @Override
        public void onPackageUpdated(String id, int version) {
            Log.d(Const.LOG_TAG, "Offline package updated: " + id);
        }

        @Override
        public void onPackageFailed(String id, int version, PackageErrorType errorType) {
            Log.e(Const.LOG_TAG, "Offline package update failed: " + id);
        }
    }

    {% endhighlight %}
  </div>

  <div id="tab-csharp">
    {% highlight csharp linenos %}
  
    public class MyPackageManagerListener : PackageManagerListener
    {
        PackageManager packageManager;

        public MyPackageManagerListener(PackageManager manager)
        {
            packageManager = manager;
        }

        public override void OnPackageListUpdated()
        {
            Log.Debug("Package list updated");
            // We have packages for all country/regions
            // see list of available ID-s: https://github.com/CartoDB/mobile-sdk/wiki/List-of-Offline-map-packages
            packageManager.StartPackageDownload("EE");
            packageManager.StartPackageDownload("LV");
        }

        public override void OnPackageListFailed()
        {
            Log.Error("Package list update failed");
        }

        public override void OnPackageStatusChanged(String id, int version, PackageStatus status)
        {
            // here you can get progress of download
        }

        public override void OnPackageCancelled(String id, int version)
        {
        }

        public override void OnPackageUpdated(String id, int version)
        {
            Log.Debug("Offline package updated: " + id);
        }

        public override void OnPackageFailed(String id, int version, PackageErrorType errorType)
        {
            Log.Error("Offline package download failed: " + id);
        }
    }

    {% endhighlight %}
  </div>

  <div id="tab-objectivec">
    {% highlight objc linenos %}
    
    @interface MyPackageManagerListener : NTPackageManagerListener

    @property NTPackageManager* _packageManager;
    - (void)setPackageManager:(NTPackageManager*)manager;
    
    @end

    @implementation MyPackageManagerListener

    - (void)onPackageListUpdated
    {
        NSLog(@"onPackageListUpdated");
        // We have packages for all country/regions
        // see list of available ID-s: https://github.com/CartoDB/mobile-sdk/wiki/List-of-Offline-map-packages
        [self._packageManager startPackageDownload: @"EE"];
        [self._packageManager startPackageDownload: @"LV"];
    }

    - (void)onPackageListFailed
    {
        NSLog(@"onPackageListFailed");
    }

    - (void)onPackageUpdated:(NSString*)packageId version:(int)version
    {
    }

    - (void)onPackageCancelled:(NSString*)packageId version:(int)version
    {
    }

    - (void)onPackageFailed:(NSString*)packageId version:(int)version errorType:(enum NTPackageErrorType)errorType
    {
        NSLog(@"onPackageFailed");
    }

    - (void)onPackageStatusChanged:(NSString*)packageId version:(int)version status:(NTPackageStatus*)status
    {
        // here you can get progress of download
        NSLog(@"onPackageStatusChanged progress: %f", [status getProgress]);
    }

    - (void)setPackageManager:(NTPackageManager*)manager
    {
        self._packageManager = manager;
    }

    @end

    {% endhighlight %}
  </div>

  <div id="tab-swift">
    {% highlight swift linenos %}
      
    public class MyPackageManagerListener : NTPackageManagerListener {
        var packageManager: NTPackageManager?
        
        convenience init(manager: NTCartoPackageManager) {
            self.init()
            self.packageManager = manager
        }
        
        public override func onPackageListUpdated() {
            
            // Start download of package of Estonia and Latvia
            // see list of available ID-s: https://github.com/CartoDB/mobile-sdk/wiki/List-of-Offline-map-packages
            self.packageManager?.startPackageDownload("EE");
            self.packageManager?.startPackageDownload("LV");
        }
        
        public override func onPackageListFailed() {
            
        }
        
        public override func onPackageStatusChanged(_ arg1: String!, version: Int32, status: NTPackageStatus!) {
            // Here you can monitor download process %
        }
        public override func onPackageUpdated(_ arg1: String!, version: Int32) {
            
        }
        
        public override func onPackageCancelled(_ arg1: String!, version: Int32) {
            
        }
        
        public override func onPackageFailed(_ arg1: String!, version: Int32, errorType: NTPackageErrorType) {
            
        }
    }

    {% endhighlight %}
  </div>

  <div id="tab-kotlin">
    {% highlight kotlin linenos %}
  
    class MyPackageManagerListener(val packageManager: CartoPackageManager) : PackageManagerListener() {
        var packageManager: PackageManager? = null

        constructor(val manager: PackageManager?) {
            packageManager = manager;
        }

        override fun onPackageListUpdated() {

            // Start download of package of Estonia and Latvia
            // see list of available ID-s: https://github.com/CartoDB/mobile-sdk/wiki/List-of-Offline-map-packages
            packageManager.startPackageDownload("EE");
            packageManager.startPackageDownload("LV");
        }

        override fun onPackageListFailed() {

        }

        override fun onPackageStatusChanged(id: String?, version: Int, status: PackageStatus?) {
            // Here you can monitor download process %
        }

        override fun onPackageUpdated(id: String?, version: Int) {

        }

        override fun onPackageCancelled(id: String?, version: Int) {

        }

        override fun onPackageFailed(id: String?, version: Int, errorType: PackageErrorType?) {

        }
    }

    {% endhighlight %}
  </div>
    
</div>


#### 4. Initialize PackageManager

To link PackageManagerListener with PackageManager, apply the following code.

<div class="js-tabs-mobilesdk">
   <ul class="tab-navigation">
     <li class="tab-navigationItem">
       <a href="#tab-java">Java</a>
     </li>
     <li class="tab-navigationItem">
       <a href="#tab-csharp">C#</a>
     </li>
     <li class="tab-navigationItem">
       <a href="#tab-objectivec">Objective-C</a>
     </li>
     <li class="tab-navigationItem">
       <a href="#tab-swift">Swift</a>
     </li>
     <li class="tab-navigationItem">
       <a href="#tab-kotlin">Kotlin</a>
     </li>
   </ul>

  <div id="tab-java">
    {% highlight java linenos %}

    // 1. Set listener, and start PackageManager
    packageManager.setPackageManagerListener(new MyPackageManagerListener(packageManager));
    packageManager.start();

    // 2. Fetch list of available packages from server. Note that this is asynchronous operation and listener will be notified via onPackageListUpdated when this succeeds.
    packageManager.startPackageListDownload();

    {% endhighlight %}
  </div>

  <div id="tab-csharp">
    {% highlight csharp linenos %}

    // 1. Create and set listener, and start PackageManager
    packageManager.PackageManagerListener = new MyPackageManagerListener(packageManager);
    packageManager.Start();

    // 2. Fetch list of available packages from server. 
    // Note that this is asynchronous operation and the listener will be notified via OnPackageListUpdated when this succeeds.        
    packageManager.StartPackageListDownload();

    {% endhighlight %}
  </div>

  <div id="tab-objectivec">
    {% highlight objc linenos %}

    NTCartoPackageManager* packageManager = [[NTCartoPackageManager alloc] initWithSource:@"<your-package-source>" dataFolder:packagesDir];
        
    // 1. Create PackageManagerListener with your listener class
    MyPackageManagerListener* _packageManagerListener = [[MyPackageManagerListener alloc] init];
    [_packageManagerListener setPackageManager: packageManager];

    // Attach package manager listener
    [packageManager setPackageManagerListener:_packageManagerListener];

    // Start PackageManager
    [packageManager start];

    // 2. Start download of packageList. When download is done, then the listener's onPackageListUpdated() is called
    [packageManager startPackageListDownload];

    {% endhighlight %}
  </div>

  <div id="tab-swift">
    {% highlight swift linenos %}

    // Create PackageManager instance for dealing with offline packages
    var packageManager =  NTCartoPackageManager(source: "<your-package-source>", dataFolder: packageFolder)

    // 1. Set listener, and start PackageManager
    packageManager?.setPackageManagerListener(MyPackageManagerListener(manager: packageManager!))
    packageManager?.start()

    // 2. Fetch list of available packages from server.
    // Note that this is asynchronous operation
    // and listener will be notified via onPackageListUpdated when this succeeds.
    packageManager?.startPackageListDownload()

    {% endhighlight %}
  </div>

  <div id="tab-kotlin">
    {% highlight kotlin linenos %}

    // 1. Set listener, and start PackageManager
    packageManager?.packageManagerListener = MyPackageManagerListener(packageManager)
    packageManager.start()

    // 2. Fetch list of available packages from server. 
    // Note that this is asynchronous operation 
    // and listener will be notified via onPackageListUpdated when this succeeds.
    packageManager.startPackageListDownload()

    {% endhighlight %}
  </div>
    
</div>

### Downloading map packages

To have offline map data the PackageManager has to download some maps. Thre are two types of offline downloads:  code sample above already includes downloading for country ID-s, and you can download also offline maps for any custom areas e.g. cities.

During any package download PackageManagerListener gets `onPackageStatusChanged()` call back events, this can be used to display progress bar UI.

#### Using country ID

Se list of available ID-s: https://github.com/CartoDB/mobile-sdk/wiki/List-of-Offline-map-packages

Start download of e.g. Estonia with this PackageManager use method: `.startPackageDownload("EE");`. Note that you must be sure that the PackageList is downloaded at least once: this is started with `.startPackageListDownload()` and confirmed in `onPackageListUpdated()` callback.


#### Bounding Box

CARTO Mobile SDK allows for the download of custom areas, called bounding boxes. It can be a city or national park for example. Note that there is size limit of about 50x50 km for the allowed areas here. For bigger areas you need to use several downloads, or country package.

A bounding box is constructed as `bbox(west longitude,south latitude,east longitude,north latitude`, so the bounding box of Berlin would be: `bbox(13.2285,52.4698,13.5046,52.57477)`. There is no separate method for bounding box download start, just use same as string instead of  instead of a country or county code, as package ID. So Berlin download would start with `.startPackageDownload("bbox(13.2285,52.4698,13.5046,52.57477)");`

Note that if you only download bounding box areas in your app, then PackageListDownload is not needed.


### Consume PackageManager packages

####  Offline Map Layers

You should add `CartoOfflineVectorTileLayer` to the MapView, using PackageManager and map style for offline map layer as constructor parameters. 

**Warning** - until map is downloaded, then this layer will have no map. So you may want to add another online tiled layer with same style, which will be replaced once offline map is downloaded

  {% highlight java linenos %}
  CartoOfflineVectorTileLayer layer = new CartoOfflineVectorTileLayer(cartoPackageManager, CartoBaseMapStyle.CARTO_BASEMAP_STYLE_VOYAGER);
  mapView.getLayers().add(layer);
  {% endhighlight %}

#### Offline Routing

Offline Routing is covered in our [Offline Routing document]({{site.mobilesdk_docs}}/guides/mobile-routing/#offline-routing)

#### Offline Geocoding

Offline geocoding is covered in our [Offline Geocoding document]({{site.mobilesdk_docs}}/guides/mobile-geocoding/#offline-geocoding)

### Additional notes

#### Updating packages

There is no special event or method to check package updates, so updates can be checked and controlled by application using following logic. You can call this logic as soon as you feel appropriate. Different packages can be updated in different point of time, and with different frequency.

- Use packagemanager *startPackageListDownload* to get server packages
- Wait for listener's *onPackageListUpdated* event, now server packages were downloaded
- Use packageManager *getLocalPackages* to get already downloaded packages (local packages)
- Iterate list of local packages, check from metadata if server package list has newer version of some
- If there is newer version, then this package is updated
- Updated package download is like normal first download

During re-download of same package application shows old map until download is complete. So the update can run in background safely.

#### Additional notes

* *startPackageListDownload* method does not need to be called every time when package list is required. In fact, once package list is successfully downloaded, it will remain locally available. But refreshing the contents once a day or perhaps once a week is recommended as some older packages may become available once newer versions are uploaded. *getServerPackageListAge* method can be used to check the age of the package list data.
* Package manager keeps persistent task queue of all submitted requests. Even when it is stopped, downloads will automatically resume when it is started next time.
* It is possible to pause downloads by setting task priority to -1. Downloads will resume once priority is set to non-negative number.
* *PackageInfo* class provides two methods for human-readable package names: *getName* and *getNames*. *getNames* takes language argument and will return localized name of the package (assuming the language is supported, currently only major languages such as English, German, French are supported). *getNames* returns list of names, as some packages can be placed under multiple categories. So, using *getNames* should be preferred over *getName*.
* Each package includes **tile mask**. Tile mask basically lists all tiles of the package up to a certain zoom level (currently 10). Tile mask can be used to quickly find a package corresponding to given tile coordinates or to find a package containing given tile.
