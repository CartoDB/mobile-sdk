## Mobile Routing

**Routing** in CARTO Mobile SDK provides fastest path between two or more geographic locations.

The resulting Route includes waypoints and instructions. It can be displayed as turn-by-turn directions on your map, or just a line or list of instructions. Routing functionality through the Mobile SDK includes [online routing](#online-routing), based on CARTOs online service (or third party services), and [offline routing](#offline-routing), which requires that you install an offline data package. Offline packages have to be downloaded via **Package Manager**.

Mobile SDK supports the following routing features:

 - Find the **fastest route from A to B**
 - Find the **fastest route between X points**, in a given order
 - Get the complete result by **route geometry** and display it on the map
 - Get instructions for **navigation actions** (turn left/right, u-turn, leave roundabout etc.)
 - Specify **instruction details**, such as the street name, turn angle, azimuth, distance and time for the next leg
 - Plan for **turn restrictions** and **one-way streets** as part of the route
 - **Fast calculations even in offline mode**, approximately 50-300 ms is expected, even for long routes
 - **Multi-country route calculations**, even in offline mode

### Online Routing
 
Online routing requires that you create a simple call and request to calculate the route.

First create the `CartoOnlineRoutingService` and then  the route with the `calculateRoute` request. 

**Note:** Calculating online routing makes a network call, so do NOT do it in UI thread.

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

    CartoOnlineRoutingService onlineRoutingService = new CartoOnlineRoutingService("nutiteq.osm.car");
    
    // create separate task, as network request is done
    
    AsyncTask<Void, Void, RoutingResult> task = new AsyncTask<Void, Void, RoutingResult>() {

        protected RoutingResult doInBackground(Void... v) {
            MapPosVector poses = new MapPosVector();
            poses.add(startPos);
            poses.add(stopPos);
            RoutingRequest request = new RoutingRequest(baseProjection, poses);
            
            // Routing request is done here:
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

  <div id="tab-csharp">
    {% highlight csharp linenos %}
  
    var onlineRoutingService = new CartoOnlineRoutingService("nutiteq.osm.car");

    // create separate task, as network request is done
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

  <div id="tab-objectivec">
    {% highlight objc linenos %}
  
    NTCartoOnlineRoutingService* onlineRoutingService = [[NTCartoOnlineRoutingService alloc] initWithApiKey:@"nutiteq.osm.car"];

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

  <div id="tab-swift">
    {% highlight swift linenos %}
  
    let onlineRoutingService = NTCartoOnlineRoutingService(source: "nutiteq.osm.car");

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

  <div id="tab-kotlin">
    {% highlight kotlin linenos %}
  
    val onlineRoutingService = CartoOnlineRoutingService("nutiteq.osm.car");

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


### Offline Routing

Offline routing requires a more complicated preparation of your offline map packages, listener events, package initialization, and routing calculation parameters.

First, you need to initialize a package manager and a listener to download packages, and then download routing map package using this. View the **PackageManager** page to find more about it.

If the required routing packages are downloaded and routing service is ready, you can calculate routing.

- Create `PackageManagerValhallaRoutingService`
- Define `RoutingRequest` with at least two points. 
- Calculate the route with the `calculateRoute` request, read response as *RoutingResult*.


#### Limitations of offline routing

CARTO Mobile SDK provides two built-in offline routing engines: the legacy routing engine is based on **OSRM project** and newer one is using **Valhalla routing engine**. The OSRM-based routing engine is better optimized for low memory usage and calculation speed, including very large road networks using *Contraction Hierarchy* representation and algorithms. However, this has some expected limitations:

- Route profile is precalculated and hardcoded in the data. For different profiles, such as driving or walking, download different map data packages to accomodate for offline routing
- Only the fastest route is calculated, there is no shortest route choice
- There are no alternative routes provided
- There is no live data in routing, traffic and temporarily closed roads do not appear
- You have to download whole country package, no custom bounding box area downloads are possible

Valhalla routing engine is more flexible, but requires more memory and is slower.

**Note:** SDK itself does not include live navigation features, such as following a GPS location, initiating automatic recalculations, or guided voice instructions. These features can still be built on top of routing by your device application, and our sample apps have code for this.

To download *Valhalla* routing engine offline packages using `PackageManager`, use `"routing:carto.streets"`  as source ID. You can then request list of country packages (we have all countries in the world covered, but some are divided to smaller parts), or use custom bounding box area to download route data for e.g. smaller metropolitan area.

Note that the routing is very similar to online routing, just the service name is different. Following code assumes that you already have `PackageManager` with correct packages, and have downloaded a package.

{% highlight java linenos %}

    PackageManagerValhallaRoutingService service =
                    new PackageManagerValhallaRoutingService(packageManager);
    
    // create separate task, as network request is done
    
    AsyncTask<Void, Void, RoutingResult> task = new AsyncTask<Void, Void, RoutingResult>() {

        protected RoutingResult doInBackground(Void... v) {
            MapPosVector poses = new MapPosVector();
            poses.add(startPos);
            poses.add(stopPos);
            RoutingRequest request = new RoutingRequest(baseProjection, poses);
            
            // Routing request is done here:
            RoutingResult result = service.calculateRoute(request);

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