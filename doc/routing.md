# Mobile Routing

Routing is the navigation from a defined start location to a defined end location. The calculated results are displayed as turn-by-turn directions on your map, based on the transportation mode that you specified. Routing functionality through the Mobile SDK includes [online routing](#online-routing), based on CARTOs online service, and [offline routing](#offline-routing), which requires that you install an offline data package on your local device.

Mobile SDK supports the following routing features:

 - Find the fastest route from A to B
 - Find the fastest route between X points, in a given order
 - Get the complete result by route geometry and display it on the map
 - Set instructions for navigation actions (turn left/right, u-turn, leave roundabout etc.)
 - Specify instruction details, such as the street name, turn angle, azimuth, distance and time for the next leg
 - Plan for turn restrictions and one-way streets as part of the route
 - Fast calculations in new devices, approximately 200-300 ms is expected, even for long routes
 - Multi-country route calculations

## Routing Limitations

Mobile Routing is optimized for low memory usage and calculation speed, including very large road networks using *Contraction Hierarchy* representation and algorithms. As a result, this creates some expected limitations:

- Route profile is precalculated from the server and hardcoded in the data. For different profiles, such as driving or walking, download different map data packages to accomodate for offline routing

- There is no shortest or fastest choice in the calculation, this is pre-coded in the routing data

- There are no alternative routes provided

- There is no live data in routing, traffic and temporarily closed roads do not appear

**Note:** Routing does not include live navigation features, such as following a GPS location, initiating automatic recalculations, or guided voice instructions. However, these features can be built on top of routing by your device application.

## Offline Routing Packages

CARTO has created a customized routing package, similar to other offline map packages, that contain **world-wide offline route packages**. This routing package includes the corresponding online service for most common profiles, using **osm.car** and **osm.foot** OpenStreetMap data, as the map source.

- The list of country packages for routing is the same as other offline maps. See [Offline Map Packages](/docs/carto-engine/mobile-sdk/offline-maps/#offline-map-packages) for the full list of offline packages

- The download size of the offline routing package is significantly larger (10-40% greater) than the size of the corresponding offline map package. Car profile packages are considerably smaller than walking packages

- For Enterprise accounts, offline routing packages include **HERE.com map data**. In many countries, especially outside Europe, offline routing packages contain more granular results. In addition, HERE includes address data. Please [contact us](mailto:sales@carto.com) if you are interested in Enterprise mobile features

## Apply Routing in your App

This section describes how to access prepackaged routing code from our Sample Apps, how how to apply routing in your mobile app.

### Prepackaged Sample Code

For minimal routing implementation, use our sample app code for different mobile platforms. You can add this sample code to your mobile project.

- iOS Platform:
  
  - [Sample App repository](https://github.com/nutiteq/hellomap3d-ios)

  - [`OfflineRoutingController.mm`](https://github.com/nutiteq/hellomap3d-ios/blob/master/advancedmap3/advancedmap3/OfflineRoutingController.mm) parameter

- Android Platform: 

  - [Sample App repository](https://github.com/nutiteq/hellomap3d-android)

  - [`OfflineRoutingActivity.java`](https://github.com/nutiteq/hellomap3d-android/blob/master/com.nutiteq.advancedmap3/src/com/nutiteq/advancedmap3/OfflineRoutingActivity.java) parameter

- Xamarin (Android):

  - [Sample App repository](https://github.com/nutiteq/hellomap3d-dotnet)

  -  [`OfflineRouting.cs`](https://github.com/nutiteq/hellomap3d-dotnet/blob/master/hellomap-android/OfflineRouting.cs) parameter

### Example Procedure

The following procedure describes how to apply the sample routing code in your mobile app project.

1. Download the [sample code](#prepackaged-sample-code) to your mobile app project

    The pre-coded, offline route packages are automatically loaded. 

    **Tip:** For this example, Estonia and Latvia are downloaded by default. _You can easily change the code to download any other country or state._

2. Press and hold (long-click) on map to set route start point. Long-click a second time to set the end point

    **Tip:** See [Listening to Events](/docs/carto-engine/mobile-sdk/developer-guide-to-sdk-features/#listening-to-events) for details about the `CLICK_TYPE_LONG` ClickType.

3. Once the end-point is set, the route is automatically calculated

4. The route is shown as a line on map, and the navigation instructions appear as markers

See this video demonstration of how routing appears in a mobile app.

<iframe width="420" height="315" src="https://www.youtube.com/embed/8u-DpOAt0zQ" frameborder="0" allowfullscreen></iframe>

_Note the following simplifications applied within the sample code_ You may need to adjust your code accordingly if want to apply different options:

- The background map is online 

- Online routing is applied before any offline routing is completed. For offline routing, wait a few minutes until the download finishes. The download progress is not indicated in the app

- To prevent routing errors, ensure the route start and end points are within the downloaded package areas. If the shortest route passes another country or area that is not downloaded, routing fails

- If the route start and route stop points are in located in different countries, you must also find and download each country package that will appear along the route

The following _Online Routing_ and _Offline Routing_ procedures demonstrate the required routing code.

## Online Routing
 
Online routing requires that you create a simple call and request to calculate the route.

1) Create the `CartoOnlineRoutingService` call

<div class="js-TabPanes">
  <ul class="Tabs">
    <li class="Tab js-Tabpanes-navItem is-active">
      <a href="#/0" class="js-Tabpanes-navLink--lang js-Tabpanes-navLink--lang--java">Java</a>
    </li>
    <li class="Tab js-Tabpanes-navItem">
      <a href="#/1" class="js-Tabpanes-navLink--lang js-Tabpanes-navLink--lang--csharp">C#</a>
    </li>
    <li class="Tab js-Tabpanes-navItem">
      <a href="#/2" class="js-Tabpanes-navLink--lang js-Tabpanes-navLink--lang--objective-c">Objective-C</a>
    </li>
    <li class="Tab js-Tabpanes-navItem">
      <a href="#/3" class="js-Tabpanes-navLink--lang js-Tabpanes-navLink--lang--swift">Swift</a>
    </li>
  </ul>

  <div class="Carousel-item js-Tabpanes-item is-active">
  {% highlight java %}onlineRoutingService = new CartoOnlineRoutingService("nutiteq.osm.car");

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item">
  {% highlight csharp %}onlineRoutingService = new CartoOnlineRoutingService("nutiteq.osm.car");

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item">
  {% highlight objc %}_onlineRoutingService = [[NTCartoOnlineRoutingService alloc] initWithSource:@"nutiteq.osm.car"];

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item">
  {% highlight swift %}COMING SOON...

  {% endhighlight %}
  </div>
  
</div>

2) Calculate the route with the `calculateRoute` request

Calculating routing requests are expensive (in quota consumption and in processing times). It is recommended to use a background task for more efficient performance. Otherwise, the original routing task can be blocked for a few seconds on slower mobile devices.

These code samples display how to show navigation instructions on the map, as indicated by line and markers.

<div class="js-TabPanes">
  <ul class="Tabs">
    <li class="Tab js-Tabpanes-navItem is-active">
      <a href="#/0" class="js-Tabpanes-navLink--lang js-Tabpanes-navLink--lang--java">Java</a>
    </li>
    <li class="Tab js-Tabpanes-navItem">
      <a href="#/1" class="js-Tabpanes-navLink--lang js-Tabpanes-navLink--lang--csharp">C#</a>
    </li>
    <li class="Tab js-Tabpanes-navItem">
      <a href="#/2" class="js-Tabpanes-navLink--lang js-Tabpanes-navLink--lang--objective-c">Objective-C</a>
    </li>
    <li class="Tab js-Tabpanes-navItem">
      <a href="#/3" class="js-Tabpanes-navLink--lang js-Tabpanes-navLink--lang--swift">Swift</a>
    </li>
  </ul>

  <div class="Carousel-item js-Tabpanes-item is-active">
  {% highlight java %}AsyncTask<Void, Void, RoutingResult> task = new AsyncTask<Void, Void, RoutingResult>() {

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
  {% highlight csharp %}ThreadPool.QueueUserWorkItem(delegate
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
  {% highlight objc %}

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
  {% highlight swift %}COMING SOON...

  {% endhighlight %}
  </div>
  
</div>

**Tip:** As processing online queries may take some time, it is suggested to use a background task for more efficient performance. Use the following code to apply online routing to your application:

<div class="js-TabPanes">
  <ul class="Tabs">
    <li class="Tab js-Tabpanes-navItem is-active">
      <a href="#/0" class="js-Tabpanes-navLink">Android Java</a>
    </li>
    <li class="Tab js-Tabpanes-navItem">
      <a href="#/1" class="js-Tabpanes-navLink">Xamarin</a>
    </li>
    <li class="Tab js-Tabpanes-navItem">
      <a href="#/2" class="js-Tabpanes-navLink--lang js-Tabpanes-navLink--lang--objective-c">Objective-C</a>
    </li>
    <li class="Tab js-Tabpanes-navItem">
      <a href="#/3" class="js-Tabpanes-navLink--lang js-Tabpanes-navLink--lang--swift">Swift</a>
    </li>
  </ul>

  <div class="Carousel-item js-Tabpanes-item is-active">
  {% highlight java %}COMING SOON...{% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item">
  {% highlight xamarin %}COMING SOON...{% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item">
  {% highlight objc %}COMING SOON...{% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item">
  {% highlight swift %}COMING SOON...{% endhighlight %}
  </div>
</div>

## Offline Routing

Offline routing requires a more complicated preparation of your offline map packages, listener events, package initialization, and routing calculation parameters.

1) Create CartoPackageManager to prepare download of routing packages

For offline routing, you must download routing packages.  You can use the same *PackageManager* that is used for offline map packages. The download process and listener events are the same. See [Offline Map Service](/docs/carto-engine/mobile-sdk/offline-map-service/) for details. 

**Note:** If you are using both offline map packages and routing packages, you must create two instances of the Package Manager, since offline routing uses different packages (as specified by the *DataSource*).

- Define the folder where to store the routing files (which should be different from your map packages), and use *PackageManagerRoutingService* to calculate the routes

<div class="js-TabPanes">
  <ul class="Tabs">
    <li class="Tab js-Tabpanes-navItem is-active">
      <a href="#/0" class="js-Tabpanes-navLink--lang js-Tabpanes-navLink--lang--java">Java</a>
    </li>
    <li class="Tab js-Tabpanes-navItem">
      <a href="#/1" class="js-Tabpanes-navLink--lang js-Tabpanes-navLink--lang--csharp">C#</a>
    </li>
    <li class="Tab js-Tabpanes-navItem">
      <a href="#/2" class="js-Tabpanes-navLink--lang js-Tabpanes-navLink--lang--objective-c">Objective-C</a>
    </li>
    <li class="Tab js-Tabpanes-navItem">
      <a href="#/3" class="js-Tabpanes-navLink--lang js-Tabpanes-navLink--lang--swift">Swift</a>
    </li>
  </ul>

  <div class="Carousel-item js-Tabpanes-item is-active">
  {% highlight java %}

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
  {% highlight csharp %}

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
  {% highlight objc %}

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
  {% highlight swift %}COMING SOON...

  {% endhighlight %}
  </div>
  
</div>
 
2) Use PackageManagerListener to get DownloadManager events

Routing package downloads cannot be started immediately, as the Mobile SDK needs to get latest definition of packages from CARTO online service. Once this list is received, `PackageManagerListener's .onPackageListUpdated()` is called.

- Write your own PackageManagerListener, and start the offline download using the `onPackageListUpdated` method, which ensure that the package metadata is downloaded

<div class="js-TabPanes">
  <ul class="Tabs">
    <li class="Tab js-Tabpanes-navItem is-active">
      <a href="#/0" class="js-Tabpanes-navLink--lang js-Tabpanes-navLink--lang--java">Java</a>
    </li>
    <li class="Tab js-Tabpanes-navItem">
      <a href="#/1" class="js-Tabpanes-navLink--lang js-Tabpanes-navLink--lang--csharp">C#</a>
    </li>
    <li class="Tab js-Tabpanes-navItem">
      <a href="#/2" class="js-Tabpanes-navLink--lang js-Tabpanes-navLink--lang--objective-c">Objective-C</a>
    </li>
    <li class="Tab js-Tabpanes-navItem">
      <a href="#/3" class="js-Tabpanes-navLink--lang js-Tabpanes-navLink--lang--swift">Swift</a>
    </li>
  </ul>

  <div class="Carousel-item js-Tabpanes-item is-active">
  {% highlight java %}public class RoutePackageManagerListener extends PackageManagerListener {
    @Override
    public void onPackageListUpdated() {
        Log.d(Const.LOG_TAG, "Package list updated");
        // Start download of package of Estonia
        // see list of available ID-s: http://cartod.com/docs/carto-engine/mobile-sdk/offline-map-service/
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
  {% highlight csharp %}public class RoutePackageManagerListener : PackageManagerListener
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
      // see list of available ID-s: https://carto.com/carto-engine-mobile-sdk/offline-map-service
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
  {% highlight objc %}@interface RoutePackageManagerListener : NTPackageManagerListener

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
  {% highlight swift %}COMING SOON...

  {% endhighlight %}
  </div>
  
</div>

**Note:** If you are wondering why these code samples include `EE` and `LV` (Estonia and Latvia) packages, there is a  multi-country route that is needed for offline routing. For example, see why [these countries might be linked](http://www.baltictimes.com/estonian_president_marries_latvian_cyber_defence_expert/). This shows that offline routing works across borders.

3) Initialize instance of PackageManagerListener, start PackageManager and package list update

To link PackageManagerListener with PackageManager (and to have a fully working RoutingService as a result), apply the following code. 

- Pass the packageManager reference to the listener to start the downloads. This is also dependent on your mobile platform and your app architecture.

<div class="js-TabPanes">
  <ul class="Tabs">
    <li class="Tab js-Tabpanes-navItem is-active">
      <a href="#/0" class="js-Tabpanes-navLink--lang js-Tabpanes-navLink--lang--java">Java</a>
    </li>
    <li class="Tab js-Tabpanes-navItem">
      <a href="#/1" class="js-Tabpanes-navLink--lang js-Tabpanes-navLink--lang--csharp">C#</a>
    </li>
    <li class="Tab js-Tabpanes-navItem">
      <a href="#/2" class="js-Tabpanes-navLink--lang js-Tabpanes-navLink--lang--objective-c">Objective-C</a>
    </li>
    <li class="Tab js-Tabpanes-navItem">
      <a href="#/3" class="js-Tabpanes-navLink--lang js-Tabpanes-navLink--lang--swift">Swift</a>
    </li>
  </ul>

  <div class="Carousel-item js-Tabpanes-item is-active">
  {% highlight java %}

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
  {% highlight csharp %}

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
  {% highlight objc %}NTCartoPackageManager* packageManager = [[NTCartoPackageManager alloc] initWithSource:@"routing:nutiteq.osm.car" dataFolder:packagesDir];
    
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
  {% highlight swift %}COMING SOON...

  {% endhighlight %}
  </div>
  
</div>

4) Calculate the Route

Now that all routing packages are downloaded and routing service is ready, you can calculate routing.

- Create the `CartoOfflineRoutingService` call

   Define the *RoutingRequest* with at least two points. Start routing with the service and read response as *RoutingResult*.

- Calculate the route with the `calculateRoute` request

  **Note:** This step is identical to the [online routing calculation code](/docs/carto-engine/mobile-sdk/mobile-routing/#online-routing).
