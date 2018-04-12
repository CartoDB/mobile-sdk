## Mobile Routing

**Routing** in CARTO Mobile SDK provides navigation info
from a defined start location to a defined end location.

The calculated route includes waypoints and instructions.
It can be displayed as turn-by-turn directions on your map,
based on the transportation mode that you specified.
Routing functionality through the Mobile SDK includes
[online routing](#online-routing), based on CARTOs online service (or third party services),
and [offline routing](#offline-routing), which requires that you install an offline data package 
(usually through **Package Manager**) on your local device.

Mobile SDK supports the following routing features:

 - Find the fastest route from A to B
 - Find the fastest route between X points, in a given order
 - Get the complete result by route geometry and display it on the map
 - Get instructions for navigation actions (turn left/right, u-turn, leave roundabout etc.)
 - Specify instruction details, such as the street name, turn angle, azimuth, distance and time for the next leg
 - Plan for turn restrictions and one-way streets as part of the route
 - Fast calculations even in offline mode, approximately 50-300 ms is expected, even for long routes
 - Multi-country route calculations, even in offline mode

#### Existing Samples

For minimal routing implementation, use our sample app code for different mobile platforms. You can add this sample code to your mobile project.

- iOS Platform:
  
  - [Sample app repository](https://github.com/CartoDB/mobile-ios-samples)

  - [`OfflineRoutingController.mm`](https://github.com/CartoDB/mobile-ios-samples/blob/master/AdvancedMap.Objective-C/AdvancedMap/OfflineRoutingController.mm)

- Android Platform: 

  - [Sample app repository](https://github.com/CartoDB/mobile-android-samples)

  - [`OfflineRoutingActivity.java`](https://github.com/CartoDB/mobile-android-samples/blob/master/AdvancedMapAndroid/app/src/main/java/com/carto/advancedmap/sections/routing/offline/OfflineRoutingActivity.java)

- Xamarin (Android, iOS):

  - [Sample app repository](https://github.com/CartoDB/mobile-dotnet-samples)

  - [`OfflineRouting.cs`](https://github.com/CartoDB/mobile-dotnet-samples/blob/master/AdvancedMap.Droid/Sections/Routing/OfflineRouting/OfflineRoutingActivity.cs)

#### Example Procedure

The following procedure describes how to apply the sample routing code in your mobile app project.

1. Download the [sample code](#prepackaged-sample-code) to your mobile app project

    The pre-coded, offline route packages are automatically loaded. 

    **Tip:** For this example, Estonia and Latvia are downloaded by default. _You can easily change the code to download any other country or state._

2. Press and hold (long-click) on map to set route start point. Long-click a second time to set the end point

    **Tip:** See [Listening to Events](/docs/carto-engine/mobile-sdk/04-map-listeners/#implementing-mapeventlistener) for details about the `CLICK_TYPE_LONG` ClickType.

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

#### Online Routing
 
Online routing requires that you create a simple call and request to calculate the route.

1) Create the `CartoOnlineRoutingService` call

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
    <li class="Tab js-Tabpanes-navItem--lang">
      <a href="#/3" class="js-Tabpanes-navLink--lang js-Tabpanes-navLink--lang--kotlin">Kotlin</a>
    </li>
  </ul>


  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--java is-active">
    {% highlight java %}

    CartoOnlineRoutingService onlineRoutingService = new CartoOnlineRoutingService("nutiteq.osm.car");

    {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--csharp">
    {% highlight csharp %}
  
    var onlineRoutingService = new CartoOnlineRoutingService("nutiteq.osm.car");

    {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--objective-c">
    {% highlight objc %}
  
    NTCartoOnlineRoutingService* onlineRoutingService = [[NTCartoOnlineRoutingService alloc] initWithApiKey:@"nutiteq.osm.car"];

    {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--swift">
    {% highlight swift %}
  
    let onlineRoutingService = NTCartoOnlineRoutingService(source: "nutiteq.osm.car");

    {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--kotlin">
    {% highlight kotlin %}
  
    val onlineRoutingService = CartoOnlineRoutingService("nutiteq.osm.car");

    {% endhighlight %}
  </div>
    
</div>

2) Calculate the route with the `calculateRoute` request

Calculating routing requests are expensive (in quota consumption and in processing times). It is recommended to use a background task for more efficient performance. Otherwise, the original routing task can be blocked for a few seconds on slower mobile devices.

These code samples display how to show navigation instructions on the map, as indicated by line and markers.

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
    <li class="Tab js-Tabpanes-navItem--lang">
      <a href="#/3" class="js-Tabpanes-navLink--lang js-Tabpanes-navLink--lang--kotlin">Kotlin</a>
    </li>
  </ul>

  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--java is-active">
    {% highlight java %}
  
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

  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--csharp">
    {% highlight csharp %}
  
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

  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--objective-c">
    {% highlight objc %}

    // Set route start end end points
    NTMapPosVector* poses = [[NTMapPosVector alloc] init];
    [poses add:startPos];
    [poses add:stopPos];
    
    NTRoutingRequest* request = [[NTRoutingRequest alloc] initWithProjection:[[mapView getOptions] getBaseProjection] points:poses];
    
    // This calculation should be in background thread
    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
        NTRoutingResult* route = [onlineRoutingService calculateRoute:request];
        
        // Process results in main thread, so it can update UI
        dispatch_async(dispatch_get_main_queue(), ^{
            
            if (route == nil){
                NSLog(@"route error");
                return;
            }
            
            NSDateFormatter* dateFormatter = [[NSDateFormatter alloc] init];
            [dateFormatter setDateFormat:@"HH:mm:ss"];
            [dateFormatter setTimeZone:[NSTimeZone timeZoneForSecondsFromGMT:0]];
            
            NSString* routeDesc = [NSString stringWithFormat:@"Route: %0.3f km, travel %@.",
                                   [route getTotalDistance]/1000.0,
                                   [dateFormatter stringFromDate: [NSDate dateWithTimeIntervalSince1970:[route getTotalTime]]]];
            
            NSLog(@"%@",routeDesc);
            
            // Get instructions
            for (int i=0; i < [[route getInstructions] size]; i++){
                NTRoutingInstruction* instruction = [[route getInstructions] get:i];
                NSLog(@"%@", [instruction description]);
            }
        });
    });

    {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--swift">
    {% highlight swift %}
  
    // Sample positions, from from Tallinn (Estonia) to Tartu (Estonia)
    let startPos = projection?.fromWgs84(NTMapPos(x: 24.7536, y: 59.4370))
    let stopPos = projection?.fromWgs84(NTMapPos(x: 26.7290, y: 58.3776))
    
    // This calculation should be in background thread
    DispatchQueue.global(qos: .userInitiated).async {
        
        let poses = NTMapPosVector()
        poses?.add(startPos)
        poses?.add(stopPos)
        
        let request = NTRoutingRequest(projection: projection, points: poses)
        let result = onlineRoutingService?.calculateRoute(request)
        
        let km = ((result?.getTotalDistance())! / 100) / 10
        let seconds = result?.getTotalTime()
        let routeText = "The route is \(km) km ( \(seconds) s)"
        
        print("RouteText: " + routeText)
        
        // Get instruction details
        let instructions = result?.getInstructions()
        
        for i in 0..<Int((instructions?.size())!) {
            let instruction = instructions?.get(Int32(i))
        }
    }

    {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--kotlin">
    {% highlight kotlin %}
  
    // Sample positions, from from Tallinn (Estonia) to Tartu (Estonia)
    val startPos = projection?.fromWgs84(MapPos(24.7536, 59.4370))
    val stopPos = projection?.fromWgs84(MapPos(26.7290, 58.3776))

    // Remember: Put your operations back on the main thread to change the UI
    // Note:
    // doAsync requires anko coroutines dependency
    // compile "org.jetbrains.anko:anko-sdk25-coroutines:$anko_version"
    doAsync {

        val poses = MapPosVector()
        poses.add(startPos)
        poses.add(stopPos)

        val request = RoutingRequest(projection, poses)
        val result = onlineRoutingService.calculateRoute(request)

        val routeText = "The route is " + (result.totalDistance / 100) / 10f + "km (" + result.totalTime + " s)"
        println("RouteText: " + routeText)
        // get instruction details
        val instructions = result.instructions

        for (i in 0..instructions.size()) {
            val instruction = instructions.get(i.toInt())
        }
    }

    {% endhighlight %}
  </div>
    
</div>

#### Offline Routing

Offline routing requires a more complicated preparation of your offline map packages, listener events, package initialization, and routing calculation parameters.

First, you need to initialize a package manager and a listener to download packages. View the [PackageManager](/docs/carto-engine/mobile-sdk/05-package-manager/) documentation to find more about offline packages.

If all required routing packages are downloaded and routing service is ready, you can calculate routing.

- Create the `PackageManagerValhallaRoutingService` call

   Define the `RoutingRequest` with at least two points. Start routing with the service and read response as *RoutingResult*.

- Calculate the route with the `calculateRoute` request

**Note:** This step is identical to the [online routing calculation code](/docs/carto-engine/mobile-sdk/mobile-routing/#online-routing).

#### Limitations of offline routing

CARTO Mobile SDK provides two built-in offline routing engines: the legacy
routing engine (based loosely on OSRM project) and Valhalla routing engine.
The legacy routing engine is better optimized for low memory usage and calculation speed,
including very large road networks using *Contraction Hierarchy* representation and algorithms.
As a result, this creates some expected limitations:

- Route profile is precalculated and hardcoded in the data. For different profiles, such as driving or walking, download different map data packages to accomodate for offline routing

- Only the fastest route is calculated, there is no shortest route choice

- There are no alternative routes provided

- There is no live data in routing, traffic and temporarily closed roads do not appear

Valhalla routing engine is more flexible, but requires more memory and is slower.

**Note:** Routing does not include live navigation features, such as following a GPS location, initiating automatic recalculations, or guided voice instructions. However, these features can be built on top of routing by your device application.

#### Offline Packages

CARTO has created a customized routing package, similar to other offline map packages, that contain **world-wide offline route packages**. This routing package includes the corresponding online service for most common profiles, using **osm.car** and **osm.foot** OpenStreetMap data, as the map source.

- The list of country packages for routing is the same as other offline maps. See [Offline Map Packages](https://github.com/CartoDB/mobile-sdk/wiki/List-of-Offline-map-packages) for the full list of offline packages

- The download size of the offline routing package is significantly larger (10-40% greater) than the size of the corresponding offline map package. Car profile packages are considerably smaller than walking packages

- For Enterprise accounts, offline routing packages can include an extra agreement to include **HERE.com map data**. In many countries, especially outside Europe, offline routing packages contain more granular results. In addition, HERE includes address data. Please [contact us](mailto:sales@carto.com) if you are interested in extra Enterprise mobile features
