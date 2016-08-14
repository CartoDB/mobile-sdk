NOT DONE YET

# Mobile Routing

 SDK has now support for routing. Both online and offline routing (based on country-based routing packages from our server, similar to offline map packages) is supported



Routing is the navigation from a defined start location to a defined end location. The calculated results are displayed as turn-by-turn directions on your map, based on the transportation mode that you specified. Routing functionality through the Mobile SDK includes [online routing] (based on CARTOs online service) and [offline routing], which require that you install a data packages stored on the device, and online routing using our online service.

CARTO are available by using the available functions in the Data Services API.

Mobile SDK includes routing functionality for both offline routing using special data packages stored on the device, and online routing using our online service.

## Routing Functionality

<img src = "/images/route.png" alt="Offline routing with CARTO" align="right">

Mobile SDK provides the following routing features:

 - **Find the fastest route** from A to B
 - Find the fastest route between X points, in a given order
 - Get the complete result **route geometry** and display it on the map
 - Set **instructions for actions** (turn left/right, u-turn, leave roundabout etc.)
 - Specify **instruction details**, such as the street name, turn angle, azimuth, distance and time for the next leg
 - Plan for turn restrictions and one-way streets as part of the route
 - **Fast calculations* in new devices, approximately 200-300 ms is expected, even for long routes
 - **Multi-country route** calculation

## Routing Limitations

Routing is optimized for low memory usage and calculation speed, including very large road networks using *Contraction Hierarchy* representation and algorithms. As a result, this creates some expected limitations:

- Route profile is pre-calculated from the server and hardcoded in the data. For different profiles, such as driving or walking, download different map data packages to accomodate for offline routing
- There is no shortest or fastest choice in the calculation, this is pre-coded in the routing data
- There are no alternative routes provided
- There is no live data in routing, traffic or temporarily closed roads do not appear

**Note:** Routing does not include live navigation features, such as following a GPS location, initiating automatic recalculations, or guided voice instructions. These features can be built on top of routing by your application.

## Offline Routing Packages

Offline routing is based on special routing packages, similar to offline map packages. CARTO has prepared **world-wide offline route packages**, and the corresponding online service for most common profiles, using **osm.car** and **osm.foot** OpenStreetMap data as the map source.

A list of country packages is the same as for offline maps, see https://developer.nutiteq.com/guides/packages for the full list.

The download size of the offline routing package is 10-40% of corresponding offline map package. Car profile packages are considerably smaller than walking packages, for example.

For Enterprise accounts, offline routing packages include **HERE.com map data**. In many countries, especially outside Europe, offline routing packages contain more granular results. In addition, HERE includes address data. Please [contact us](mailto:sales@carto.com) if you are interested in Enterprise mobile features.

## Applying Routing in your App

### Ready-made sample code
For minimal working implementation see our *advanced map* app code samples on different platforms:

* iOS: https://github.com/nutiteq/hellomap3d-ios : [OfflineRoutingController.mm](https://github.com/nutiteq/hellomap3d-ios/blob/master/advancedmap3/advancedmap3/OfflineRoutingController.mm)
* Android: https://github.com/nutiteq/hellomap3d-android : [OfflineRoutingActivity.java](https://github.com/nutiteq/hellomap3d-android/blob/master/com.nutiteq.advancedmap3/src/com/nutiteq/advancedmap3/OfflineRoutingActivity.java)
* Xamarin (Android): https://github.com/nutiteq/hellomap3d-dotnet : [OfflineRouting.cs](https://github.com/nutiteq/hellomap3d-dotnet/blob/master/hellomap-android/OfflineRouting.cs)

These samples work as following:

1. App downloads automatically several pre-coded offline route packages right after you start the sample. Estonia and Latvia are downloaded by default, but you can change the code easily to download any other country or state
2. Long-click on map set route start point, second long-click sets end points. 
3. When end-point is set, then route is automatically calculated
4. Route is shown as a line on map, instructions are shown as markers.

Sample has some simplifications to keep code simpler:

* Background map is still on-line 
* Online routing is used before download is not finished. So for offline routing wait for a minute, depending on your network speed. Download progress is not indicated in UI.
* Make sure that route start and end points are within downloaded package areas, otherwise routing error occurs. Also, if the shortest route passes another country/area that is not downloaded, routing fails. 
* if start and stop are in different countries/packages, then also in-between country packages must be downloaded to find multi-country route.

<iframe width="420" height="315" src="https://www.youtube.com/embed/8u-DpOAt0zQ" frameborder="0" allowfullscreen></iframe>


## Step-by-step instructions

Following code samples demonstrate essential routing code. Linking this to UI and map graphics is up to your app, you can see our samples about one way how to do it. You may want to have more advanced logic, e.g. show instructions as textual list, what is not in our samples, customise UI etc.


## Online routing
 
Online routing is quite simple: just create *CartoOnlineRoutingService* and call the *calculateRoute* request to calculate route. As processing the request may take some time (online query), using a background thread/task is a good idea.

<div class="js-TabPanes">
  <ul class="Tabs">
    <li class="Tab js-Tabpanes-navItem is-active">
      <a href="#/0" class="js-Tabpanes-navLink">Android Java</a>
    </li>
    <li class="Tab js-Tabpanes-navItem">
      <a href="#/1" class="js-Tabpanes-navLink">Xamarin</a>
    </li>
    <li class="Tab js-Tabpanes-navItem">
      <a href="#/2" class="js-Tabpanes-navLink">Objective-C</a>
    </li>
    <li class="Tab js-Tabpanes-navItem">
      <a href="#/3" class="js-Tabpanes-navLink">Swift</a>
    </li>
  </ul>

  <div class="Carousel-item js-Tabpanes-item is-active">
  {% highlight html %}HERE GOES THE CODE SNIPPET{% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item">
  {% highlight html %}HERE GOES THE CODE SNIPPET{% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item">
  {% highlight html %}HERE GOES THE CODE SNIPPET{% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item">
  {% highlight html %}HERE GOES THE CODE SNIPPET{% endhighlight %}
  </div>
</div>




### 1. Create service

<div class="js-TabPanes">
  <ul class="Tabs">
    <li class="Tab js-Tabpanes-navItem is-active">
      <a href="#/0" class="js-Tabpanes-navLink">Java</a>
    </li>
    <li class="Tab js-Tabpanes-navItem">
      <a href="#/1" class="js-Tabpanes-navLink">C#</a>
    </li>
    <li class="Tab js-Tabpanes-navItem">
      <a href="#/2" class="js-Tabpanes-navLink">Objective-C</a>
    </li>
    <li class="Tab js-Tabpanes-navItem">
      <a href="#/3" class="js-Tabpanes-navLink">Swift</a>
    </li>
  </ul>

  <div class="Carousel-item js-Tabpanes-item is-active">
  {% highlight html %}

  onlineRoutingService = new CartoOnlineRoutingService("nutiteq.osm.car");

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item">
  {% highlight html %}

  onlineRoutingService = new CartoOnlineRoutingService("nutiteq.osm.car");

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item">
  {% highlight html %}

  _onlineRoutingService = [[NTCartoOnlineRoutingService alloc] initWithSource:@"nutiteq.osm.car"];

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item">
  {% highlight html %}

  COMING SOON...

  {% endhighlight %}
  </div>
  
</div>


### 2. Calculate route

Note that the **calculation request is expensive**. So use of separate thread is strongly suggested, as in the sample below.  Otherwise the main thread could be blocked for up to couple of seconds on slow devices.

See our full sample to see how to show the instructions on the map, as Line and Markers.

<div class="js-TabPanes">
  <ul class="Tabs">
    <li class="Tab js-Tabpanes-navItem is-active">
      <a href="#/0" class="js-Tabpanes-navLink">Java</a>
    </li>
    <li class="Tab js-Tabpanes-navItem">
      <a href="#/1" class="js-Tabpanes-navLink">C#</a>
    </li>
    <li class="Tab js-Tabpanes-navItem">
      <a href="#/2" class="js-Tabpanes-navLink">Objective-C</a>
    </li>
    <li class="Tab js-Tabpanes-navItem">
      <a href="#/3" class="js-Tabpanes-navLink">Swift</a>
    </li>
  </ul>

  <div class="Carousel-item js-Tabpanes-item is-active">
  {% highlight html %}

  AsyncTask<Void, Void, RoutingResult> task = new AsyncTask<Void, Void, RoutingResult>() {

            protected RoutingResult doInBackground(Void... v) {
                MapPosVector poses = new MapPosVector();
                poses.add(startPos);
                poses.add(stopPos);
                RoutingRequest request = new RoutingRequest(baseProjection, poses);
                RoutingResult result = onlineRoutingService.calculateRoute(request);

                return result;
            }

            protected void onPostExecute(RoutingResult result) {
                if (result == null) {
                    Log.e(Const.LOG_TAG,"routing error");
                    return;
                }

                String routeText = "The route is " + (int) (result.getTotalDistance() / 100) / 10f + "km (" + result.getTotalTime() + " s)";
                Log.i(Const.LOG_TAG,routeText);

                // get instruction details
                RoutingInstructionVector instructions = result.getInstructions();

                boolean first = true;
        
        // Remember: Put your operations back on the main thread to change the UI
                
                for (int i = 0; i < instructions.size(); i++) {
                    RoutingInstruction instruction = instructions.get(i);
                    Log.d(Const.LOG_TAG, instruction.toString());
                }

            }
        };

        task.execute();

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item">
  {% highlight html %}

    ThreadPool.QueueUserWorkItem(delegate
    {
      MapPosVector poses = new MapPosVector();
      poses.Add(startPos);
      poses.Add(stopPos);

      RoutingRequest request = new RoutingRequest(baseProject, poses);
      RoutingResult result = onlineRoutingService.CalculateRoute(request);

      // get total route results
      String routeText = "The route is " + (int)(result.TotalDistance / 100) / 10f + "km (" + result.TotalTime + " sec) ";
      Log.Debug(routeText);

      // get instructions, just log them
      RoutingInstructionVector instructions = result.Instructions;
      

      // Remember: Put your operations back on the main thread to change the UI
      for (int i = 0; i < instructions.Count; i++)
      {
        RoutingInstruction instruction = instructions[i];
        Log.Debug(instruction.ToString());

      }
    });

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item">
  {% highlight html %}

    // Set route start end end points
    NTMapPosVector* poses = [[NTMapPosVector alloc] init];
    [poses add:startPos];
    [poses add:stopPos];
    
    NTRoutingRequest* request = [[NTRoutingRequest alloc] initWithProjection:[[mapView getOptions] getBaseProjection] points:poses];
    
    // This calculation should be in background thread
    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
        NTRoutingResult* route = [_onlineRoutingService calculateRoute:request];
        
        // Process results in main thread, so it can update UI
        dispatch_async(dispatch_get_main_queue(), ^{
            
            if (route == nil){
                NSLog(@"route error");
                return;
            }
            
            NSDateFormatter *dateFormatter = [[NSDateFormatter alloc] init];
            [dateFormatter setDateFormat:@"HH:mm:ss"];
            [dateFormatter setTimeZone:[NSTimeZone timeZoneForSecondsFromGMT:0]];
            
            double duration = 0.3;
            
            NSString* routeDesc = [NSString stringWithFormat:@"Route: %0.3f m, travel %@. Calculation took %0.3f s", [route getTotalDistance]/1000.0,
                                   [dateFormatter stringFromDate: [NSDate dateWithTimeIntervalSince1970:[route getTotalTime]]],
                                   duration];
            
            NSLog(@"%@",routeDesc);
            
            // get instructions
            for(int i=0; i < [[route getInstructions] size];i++){
                
                NTRoutingInstruction *instruction =[[route getInstructions] get:i];
                NSLog(@"%@",[instruction description]);
                
            }
        });
    });

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item">
  {% highlight html %}

  COMING SOON...

  {% endhighlight %}
  </div>
  
</div>


## Offline routing


### 1. Create CartoPackageManager to prepare download of routing packages

For offline routing you need to download routing packages, for this you use the same *PackageManager* what is used for [offline map packages](offline-maps). The download process and listener events are the same, so see offline map package manual for some details. However, as it uses different packages (specified by *source*), you must create two instances if you need both offline map packages and routing packages.

First you need to define folder where to keep the files (different from your map packages), and use *PackageManagerRoutingService* with the PackageManager to calculate the routes.

<div class="js-TabPanes">
  <ul class="Tabs">
    <li class="Tab js-Tabpanes-navItem is-active">
      <a href="#/0" class="js-Tabpanes-navLink">Java</a>
    </li>
    <li class="Tab js-Tabpanes-navItem">
      <a href="#/1" class="js-Tabpanes-navLink">C#</a>
    </li>
    <li class="Tab js-Tabpanes-navItem">
      <a href="#/2" class="js-Tabpanes-navLink">Objective-C</a>
    </li>
    <li class="Tab js-Tabpanes-navItem">
      <a href="#/3" class="js-Tabpanes-navLink">Swift</a>
    </li>
  </ul>

  <div class="Carousel-item js-Tabpanes-item is-active">
  {% highlight html %}

        // Create PackageManager instance for dealing with offline packages
        File packageFolder = new File(getApplicationContext().getExternalFilesDir(null), "routingpackages");
        
        if (!(packageFolder.mkdirs() || packageFolder.isDirectory())) {
            Log.e(Const.LOG_TAG, "Could not create package folder!");
        }

        try {
            packageManager = new CartoPackageManager("routing:nutiteq.osm.car", packageFolder.getAbsolutePath());
        } catch (IOException e) {
            e.printStackTrace();
        }

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item">
  {% highlight html %}

    // Create PackageManager instance for dealing with offline packages
    var packageFolder = new File(GetExternalFilesDir(null), "routingpackages");

    if (!(packageFolder.Mkdirs() || packageFolder.IsDirectory))
    {
      Log.Fatal("Could not create package folder!");
    }

    packageManager = new CartoPackageManager("routing:nutiteq.osm.car", packageFolder);

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item">
  {% highlight html %}

    // Define PackageManger to download offline routing packages
    // Create folder for package manager. Package manager needs persistent writable folder.
    NSArray* paths = NSSearchPathForDirectoriesInDomains(NSApplicationSupportDirectory, NSUserDomainMask,YES);
    NSString* appSupportDir = [paths objectAtIndex: 0];
    NSString* packagesDir = [appSupportDir stringByAppendingString:@"/packages"];
    NSError *error;
    [[NSFileManager defaultManager] createDirectoryAtPath:packagesDir withIntermediateDirectories:YES attributes:nil error:&error];
    
    packageManager = [[NTCartoPackageManager alloc] initWithSource:@"routing:nutiteq.osm.car" dataFolder:packagesDir];

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item">
  {% highlight html %}

  COMING SOON...

  {% endhighlight %}
  </div>
  
</div>

     
### 2. Use PackageManagerListener to get DownloadManager events

Routing package download cannot be started immediately - SDK needs to get latest definition of packages from CARTO online service. Once this list is received, PackageManagerListener's .onPackageListUpdated() is called. This similar to offline map packages - see [call flow diagram](/images/pm_flow.png)

For this you need to write your own PackageManagerListener, and start offline download in the *onPackageListUpdated* method, where it is sure that package metadata is already downloaded and known.


<div class="js-TabPanes">
  <ul class="Tabs">
    <li class="Tab js-Tabpanes-navItem is-active">
      <a href="#/0" class="js-Tabpanes-navLink">Java</a>
    </li>
    <li class="Tab js-Tabpanes-navItem">
      <a href="#/1" class="js-Tabpanes-navLink">C#</a>
    </li>
    <li class="Tab js-Tabpanes-navItem">
      <a href="#/2" class="js-Tabpanes-navLink">Objective-C</a>
    </li>
    <li class="Tab js-Tabpanes-navItem">
      <a href="#/3" class="js-Tabpanes-navLink">Swift</a>
    </li>
  </ul>

  <div class="Carousel-item js-Tabpanes-item is-active">
  {% highlight html %}

  public class RoutePackageManagerListener extends PackageManagerListener {
    @Override
    public void onPackageListUpdated() {
        Log.d(Const.LOG_TAG, "Package list updated");
        // Start download of package of Estonia
        // see list of available ID-s: https://developer.nutiteq.com/guides/packages
        // just append -routing to the ID-s
        // You can download several packages, and route is found through all of them

        packageManager.startPackageDownload("EE-routing");
        packageManager.startPackageDownload("LV-routing");
    }

    @Override
    public void onPackageListFailed() {
        Log.e(Const.LOG_TAG, "Package list update failed");
    }

    @Override
    public void onPackageStatusChanged(String id, int version, PackageStatus status)
    {
        // here you can monitor download process %
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

  <div class="Carousel-item js-Tabpanes-item">
  {% highlight html %}

    public class RoutePackageManagerListener : PackageManagerListener
  {
    PackageManager packageManager;

    public RoutePackageManagerListener(PackageManager manager)
    {
      packageManager = manager;
    }

    public override void OnPackageListUpdated()
    {
      Log.Debug("Package list updated");
      // We have packages all country/regions
      // see list of available ID-s: https://developer.nutiteq.com/guides/packages
      // just append -routing to the ID-s
      // You can download several packages, and route is found through all of them

      packageManager.StartPackageDownload("EE-routing");
      packageManager.StartPackageDownload("LV-routing");
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

  <div class="Carousel-item js-Tabpanes-item">
  {% highlight html %}

    @interface RoutePackageManagerListener : NTPackageManagerListener

    @property NTPackageManager* _packageManager;
  - (void)setPackageManager:(NTPackageManager*)manager;
  
  @end

  @implementation RoutePackageManagerListener

  - (void)onPackageListUpdated
  {
      NSLog(@"onPackageListUpdated");
      // We have packages all country/regions
      // see list of available ID-s: https://developer.nutiteq.com/guides/packages
      // just append -routing to the ID-s
      // You can download several packages, and route is found through all of them
      
      [self._packageManager startPackageDownload: @"EE-routing"];
      [self._packageManager startPackageDownload: @"LV-routing"];
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

  <div class="Carousel-item js-Tabpanes-item">
  {% highlight html %}

  COMING SOON...

  {% endhighlight %}
  </div>
  
</div>

You may ask why here EE and LV (Estonia and Latvia) packages are used in the sample? It is not by chance - there are recent [tight personal links between these two countries](http://www.baltictimes.com/estonian_president_marries_latvian_cyber_defence_expert/), so offline routing has to go across borders.

### 3. Initialize instance of PackageManagerListener, start PackageManager and package list update

To link PackageManagerListener with PackageManager properly (and to have a fully working RoutingService as a result) you need to do following steps in the code. Note that we need to pass packageManager reference to the listener, otherwise the listener can not start downloads. This depends on platform and your app architecture.

<div class="js-TabPanes">
  <ul class="Tabs">
    <li class="Tab js-Tabpanes-navItem is-active">
      <a href="#/0" class="js-Tabpanes-navLink">Java</a>
    </li>
    <li class="Tab js-Tabpanes-navItem">
      <a href="#/1" class="js-Tabpanes-navLink">C#</a>
    </li>
    <li class="Tab js-Tabpanes-navItem">
      <a href="#/2" class="js-Tabpanes-navLink">Objective-C</a>
    </li>
    <li class="Tab js-Tabpanes-navItem">
      <a href="#/3" class="js-Tabpanes-navLink">Swift</a>
    </li>
  </ul>

  <div class="Carousel-item js-Tabpanes-item is-active">
  {% highlight html %}

    // 1. Set listener, and start PackageManager
    packageManager.setPackageManagerListener(new RoutePackageManagerListener());
    packageManager.start();

    // 2. Fetch list of available packages from server. Note that this is asynchronous operation and listener will be notified via onPackageListUpdated when this succeeds.
    packageManager.startPackageListDownload();

    // 3. Create offline routing service connected to package manager
    offlineRoutingService = new PackageManagerRoutingService(packageManager);

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item">
  {% highlight html %}

  // 1. Create and set listener, and start PackageManager
  packageManager.PackageManagerListener = new RoutePackageManagerListener(packageManager);
  packageManager.Start();

  // 2. Fetch list of available packages from server. 
  // Note that this is asynchronous operation and the listener will be notified via OnPackageListUpdated when this succeeds.        
  packageManager.StartPackageListDownload();

  // 3. Create offline routing service connected to package manager
  offlineRoutingService = new PackageManagerRoutingService(packageManager);

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item">
  {% highlight html %}

    NTCartoPackageManager* packageManager = [[NTCartoPackageManager alloc] initWithSource:@"routing:nutiteq.osm.car" dataFolder:packagesDir];
    
    // 1. Create routePackageManagerListener with your listener class
    RoutePackageManagerListener* _packageManagerListener = [[RoutePackageManagerListener alloc] init];
    [_packageManagerListener setPackageManager: packageManager];
    
    // Attach package manager listener
    [packageManager setPackageManagerListener:_packageManagerListener];
    
    // Start PackageManager
    [packageManager start];
    
    // 2. Start download of packageList. When download is done, then the listener's onPackageListUpdated() is called
    [packageManager startPackageListDownload];
    
    // 3. Create offline routing service connected to package manager
    _offlineRoutingService = [[NTPackageManagerRoutingService alloc] initWithPackageManager:packageManager];

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item">
  {% highlight html %}

  COMING SOON...

  {% endhighlight %}
  </div>
  
</div>

### 4. Do actual route calculation

All this preparation was needed to ensure that you have routing package downloaded and service is prepared. Actual routing is quite simple - you define *RoutingRequest* with at least 2 points, start routing with the service and read response as *RoutingResult*. In fact, it is exactly the same as in online routing chapter, just replace onlineRoutingService with offlineRoutingService, see example 2 above.
