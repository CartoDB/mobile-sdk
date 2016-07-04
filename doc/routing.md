# Routing - offline and online

Carto SDK includes routing functionality starting from version 3.3.0. This includes both offline routing using special data packages stored on the device, and online routing using our online service.

## Functionality

<img src = "/images/route.png" alt="Offline routing with Carto" align="right">


Carto SDK provides following routing features:

 * **find fastest route** from A to B
 * find fastest route between X points, in given order
 * get complete result **route geometry** to show it on map
 * get **instructions for actions** (turn left/right, u-turn, leave roundabout etc)
 * get **instruction details**: street name, turn angle, azimuth, distance and time for the next leg
 * takes into account turn restrictions and one-way streets
 * **fast calculation**: in new devices about 200-300 ms is expected, even for long routes.
 * **multi-country route** calculation

### Limitations

Initial public version is optimised for low memory usage and calculation speed (even for very large road networks by using *Contraction Hierarchy* representation and algorithms), but this creates also some limitations in flexibility:

* route profile is pre-calculated in server side, and hardcoded in the data. For different profiles  (e.g. car, walking) you should download different map data packages if offline routing is needed.
*  no shortest/fastest choice in calculation, this is precoded in the routing data
*  no alternative routes
*  no live data in routing - traffic, temporarily closed roads etc

Note that initial routing feature does not include live navigation features, like following of GPS location, initiating automatic recalculations or giving voice instructions. This can be built on top of routing by your application.

### Offline route packages

Offline routing is based on special routing packages, similar to offline map packages. Carto has prepared **world-wide offline route packages** and corresponding online service for most common profiles: **osm.car** and **osm.foot** using OpenStreetMap as map data source. Other profiles will be added based on demand. List of country packages is the same as for offline maps, see https://developer.nutiteq.com/guides/packages for the full list.

Download size of the offline routing package is about 10-40% of corresponding offline map package. Car profile packages are considerably smaller than walking packages, for example.

For commercial / enterprise users we can also provide sets of offline routing packages using **HERE.com map data**. In many countries (especially outside Europe) this is cleaner, has more roads covered and provides higher quality results. In addition, HERE includes address data. Please contact Carto if you consider using this commercial map data in your app.

## Using routing in your app

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


### Online routing
 
Online routing is quite simple: just create *CartoOnlineRoutingService* and call the *calculateRoute* request to calculate route. As processing the request may take some time (online query), using a background thread/task is a good idea.

#### 1. Create service

<div id="tabs4">
  <ul>
    <li><a href="#i4"><span>iOS ObjC</span></a></li>
    <li><a href="#a4"><span>Android Java</span></a></li>
    <li><a href="#n4"><span>.NET C#</span></a></li>
  </ul>
<div id="i4">
<pre class="brush: objc">
 // create  online routing service 
 _onlineRoutingService = [[NTCartoOnlineRoutingService alloc] initWithSource:@"nutiteq.osm.car"];
</pre>
</div>
<div id="a4">
<pre class="brush: java">
 // create  online routing service 
 onlineRoutingService = new CartoOnlineRoutingService("nutiteq.osm.car");
</pre>
</div>
<div id="n4">
<pre class="brush: csharp">
 // create  online routing service
 onlineRoutingService = new CartoOnlineRoutingService("nutiteq.osm.car");
</pre>
</div>
</div>


#### 2. Calculate route


Note that the **calculation request is expensive**. So use of separate thread is strongly suggested, as in the sample below.  Otherwise the main thread could be blocked for up to couple of seconds on slow devices.

See our full sample to see how to show the instructions on the map, as Line and Markers.

<div id="tabs5">
  <ul>
    <li><a href="#i5"><span>iOS ObjC</span></a></li>
    <li><a href="#a5"><span>Android Java</span></a></li>
    <li><a href="#n5"><span>.NET C#</span></a></li>
  </ul>
<div id="i5">
<pre class="brush: objc">

     // set route start end end points
     NTMapPosVector* poses = [[NTMapPosVector alloc] init];
     [poses add:startPos];
     [poses add:stopPos];
    
    NTRoutingRequest* request = [[NTRoutingRequest alloc] initWithProjection:[[self.mapView getOptions] getBaseProjection] points:poses];

    // this calculation should be in background thread
    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
        NTRoutingResult* route = [_onlineRoutingService calculateRoute:request];

       // process results in main thread, so it can update UI
        dispatch_async(dispatch_get_main_queue(), ^{
            if (route == nil){
                NSLog(@"route error");
                return;
            }
            
            NSDateFormatter *dateFormatter = [[NSDateFormatter alloc] init];
            [dateFormatter setDateFormat:@"HH:mm:ss"];
            [dateFormatter setTimeZone:[NSTimeZone timeZoneForSecondsFromGMT:0]];
            
            NSString* routeDesc = [NSString stringWithFormat:@"Route: %0.3f m, travel %@. Calculation took %0.3f s", [route getTotalDistance]/1000.0,
                                   [dateFormatter stringFromDate: [NSDate dateWithTimeIntervalSince1970:[route getTotalTime]]],
                                   duration];
            
            NSLog(@"%@",routeDesc);
            
            // get instructions
            for(int i=0; i &lt; [[route getInstructions] size];i++){
                NTRoutingInstruction *instruction =[[route getInstructions] get:i];
                NSLog(@"%@",[instruction description]);

            }
        });
    });

</pre>
</div>
<div id="a5">
<pre class="brush: java">

    AsyncTask&lt;Void, Void, RoutingResult&gt; task = new AsyncTask&lt;Void, Void, RoutingResult&gt;() {

        protected RoutingResult doInBackground(Void... v) {
                MapPosVector poses = new MapPosVector();
                poses.add(startPos);
                poses.add(stopPos);
                RoutingRequest request = new RoutingRequest(mapView.getOptions().getBaseProjection(), poses);
                RoutingResult result = onlineRoutingService.calculateRoute(request);

                return result;
            }

            protected void onPostExecute(RoutingResult result) {
            	if (result == null) {
                 	Log.e(Const.LOG_TAG,"routing error");
                    return;
                }

                String routeText = "The route is " + (int) (result.getTotalDistance() / 100) / 10f
                        + "km (" + result.getTotalTime()
                        + " s)";
                Log.i(Const.LOG_TAG,routeText);

                // get instruction details
                RoutingInstructionVector instructions = result.getInstructions();
                boolean first = true;
                for (int i = 0; i &lt; instructions.size(); i++) {
                	RoutingInstruction instruction = instructions.get(i);
                        Log.d(Const.LOG_TAG, instruction.toString());
                     }

                }
            }
        };

    	task.execute();


</pre>
</div>
<div id="n5">
<pre class="brush: csharp">

    Task.Run (() => {
		MapPosVector poses = new MapPosVector ();
		poses.Add (startPos);
		poses.Add (stopPos);
		RoutingRequest request = new RoutingRequest (baseProjection, poses);
		RoutingResult result = onlineRoutingService.CalculateRoute (request);
				
		// get total route results
        String routeText = "The route is " + (int)(result.TotalDistance / 100) / 10f
		              + "km (" + result.TotalTime
		              + " sec) ";
		Log.Debug(routeText);	
				
        // get instructions, just log them
		RoutingInstructionVector instructions = result.Instructions;
		for (int i = 0; i &lt; instructions.Count; i++) {
			RoutingInstruction instruction = instructions [i];
			Log.Debug(instruction.toString());

		}
	});

</pre>
</div>
</div>

### Offline routing


#### 1. Create CartoPackageManager to prepare download of routing packages

For offline routing you need to download routing packages, for this you use the same *PackageManager* what is used for [offline map packages](offline-maps). The download process and listener events are the same, so see offline map package manual for some details. However, as it uses different packages (specified by *source*), you must create two instances if you need both offline map packages and routing packages.

First you need to define folder where to keep the files (different from your map packages), and use *PackageManagerRoutingService* with the PackageManager to calculate the routes.

<div id="tabs1">
  <ul>
    <li><a href="#i1"><span>iOS ObjC</span></a></li>
    <li><a href="#a1"><span>Android Java</span></a></li>
    <li><a href="#n1"><span>.NET C#</span></a></li>
  </ul>
<div id="i1">
<pre class="brush: objc">

   // define PackageManger to download offline routing packages
    // Create folder for package manager. Package manager needs persistent writable folder.
    NSArray* paths = NSSearchPathForDirectoriesInDomains(NSApplicationSupportDirectory, NSUserDomainMask,YES);
    NSString* appSupportDir = [paths objectAtIndex: 0];
    NSString* packagesDir = [appSupportDir stringByAppendingString:@"/packages"];
    NSError *error;
    [[NSFileManager defaultManager] createDirectoryAtPath:packagesDir withIntermediateDirectories:YES attributes:nil error:&error];
    
    NTCartoPackageManager* packageManager = [[NTCartoPackageManager alloc] initWithSource:@"routing:nutiteq.osm.car" dataFolder:packagesDir];

</pre>
</div>
<div id="a1">
<pre class="brush: java">

    // create PackageManager instance for dealing with offline packages
        File packageFolder = new File(getApplicationContext().getExternalFilesDir(null), "routingpackages");
        if (!(packageFolder.mkdirs() || packageFolder.isDirectory())) {
        	Log.e(Const.LOG_TAG, "Could not create package folder!");
        }
        packageManager = new CartoPackageManager("routing:nutiteq.osm.car", packageFolder.getAbsolutePath());
        
</pre>
</div>
<div id="n1">
<pre class="brush: csharp">

	// create PackageManager instance for dealing with offline packages
	var packageFolder = new File (GetExternalFilesDir(null), "routingpackages");
	if (!(packageFolder.Mkdirs() || packageFolder.IsDirectory)) {
		Log.Fatal("Could not create package folder!");
	}

	packageManager = new CartoPackageManager("routing:nutiteq.osm.car", packageFolder.AbsolutePath);
</pre>
</div>
</div>

	
			
#### 2. Use PackageManagerListener to get DownloadManager events

Routing package download cannot be started immediately - SDK needs to get latest definition of packages from Carto online service. Once this list is received, PackageManagerListener's .onPackageListUpdated() is called. This similar to offline map packages - see [call flow diagram](/images/pm_flow.png)

For this you need to write your own PackageManagerListener, and start offline download in the *onPackageListUpdated* method, where it is sure that package metadata is already downloaded and known.


<div id="tabs2">
  <ul>
    <li><a href="#i2"><span>iOS ObjC</span></a></li>
    <li><a href="#a2"><span>Android Java</span></a></li>
    <li><a href="#n2"><span>.NET C#</span></a></li>
  </ul>
<div id="i2">
<pre class="brush: objc">

	@implementation RoutePackageManagerListener

	- (void)onPackageListUpdated
	{
	    NSLog(@"onPackageListUpdated");
	    // We have packages all country/regions
	    // see list of available ID-s: https://developer.nutiteq.com/guides/packages
	    // just append -routing to the ID-s
	    // You can download several packages, and route is found through all of them
	    
	     [_packageManager startPackageDownload: @"EE-routing"];
	     [_packageManager startPackageDownload: @"LV-routing"];
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

	@end

</pre>
</div>
<div id="a2">
<pre class="brush: java">

    /**
	 * Minimal for package manager events. Contains some empty methods.
	 */
	class RoutePackageManagerListener extends PackageManagerListener {
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


</pre>
</div>
<div id="n2">
<pre class="brush: csharp">

    /**
	 * Listener for package manager events. Contains many empty methods.
	 */
	class RoutePackageManagerListener : PackageManagerListener {

		private OfflineRouting controller;

		public RoutingPackageListener(OfflineRouting controller){
			this.controller = controller;
		}

		public override void OnPackageListUpdated() {
			Log.Debug("Package list updated");
		    // We have packages all country/regions
		    // see list of available ID-s: https://developer.nutiteq.com/guides/packages
		    // just append -routing to the ID-s
		    // You can download several packages, and route is found through all of them

			controller.packageManager.StartPackageDownload("EE-routing");
			controller.packageManager.StartPackageDownload("LV-routing");
		}

		public override void OnPackageListFailed() {
			Log.Error("Package list update failed");
		}

		public override void OnPackageStatusChanged(String id, int version, PackageStatus status) {
		  // here you can get progress of download
		}

		public override void OnPackageCancelled(String id, int version) {
		}

		public override void OnPackageUpdated(String id, int version) {
			Log.Debug("Offline package updated: " + id);
		}

		public override void OnPackageFailed(String id, int version, PackageErrorType errorType) {
			Log.Error("Offline package download failed: " + id);
		}
	}

</pre>
</div>
</div>

You may ask why here EE and LV (Estonia and Latvia) packages are used in the sample? It is not by chance - there are recent [tight personal links between these two countries](http://www.baltictimes.com/estonian_president_marries_latvian_cyber_defence_expert/), so offline routing has to go across borders.

#### 3. Init instance of PackageManagerListener, start PackageManager and package list update

To link PackageManagerListener with PackageManager properly (and to have a fully working RoutingService as a result) you need to do following steps in the code. Note that we need to pass packageManager reference to the listener, otherwise the listener can not start downloads. This depends on platform and your app architecture.

<div id="tabs3">
  <ul>
    <li><a href="#i3"><span>iOS ObjC</span></a></li>
    <li><a href="#a3"><span>Android Java</span></a></li>
    <li><a href="#n3"><span>.NET C#</span></a></li>
  </ul>
<div id="i3">
<pre class="brush: objc">

     // 1. create routePackageManagerListener with your listener class
     RoutePackageManagerListener* _packageManagerListener = [[RoutePackageManagerListener alloc] init];
	[_packageManagerListener setPackageManager: packageManager];
	
    // Attach package manager listener
    [packageManager setPackageManagerListener:_packageManagerListener];

    // start PackageManager
    [packageManager start];
    
    // 2. now start download of packageList. When download is done, then the 
    // listener's OnPackageListUpdated() is called
    [packageManager startPackageListDownload];
    
    // 3. create offline routing service connected to package manager
    _offlineRoutingService = [[NTPackageManagerRoutingService alloc] initWithPackageManager:packageManager];

</pre>
</div>
<div id="a3">
<pre class="brush: java">
        
        // 1. set listener, and start PackageManager
	    packageManager.setPackageManagerListener(new RoutePackageManagerListener());
        packageManager.start();
        
        // 2. fetch list of available packages from server. Note that this is asynchronous operation and listener will be notified via onPackageListUpdated when this succeeds.        
        packageManager.startPackageListDownload();
        
         // 3. create offline routing service connected to package manager
        offlineRoutingService = new PackageManagerRoutingService(packageManager);
        
</pre>
</div>
<div id="n3">
<pre class="brush: csharp">

			// 1. create and set listener, and start PackageManager
			packageManager.PackageManagerListener = new RoutingPackageListener(this);
			packageManager.Start();

			// 2. fetch list of available packages from server. Note that this is asynchronous operation and listener will be notified via onPackageListUpdated when this succeeds.        
			packageManager.StartPackageListDownload();

			// 3. create offline routing service connected to package manager
			offlineRoutingService = new PackageManagerRoutingService(packageManager);
</pre>
</div>
</div>

#### 4. Do actual route calculation

All this preparation was needed to ensure that you have routing package downloaded and service is prepared. Actual routing is quite simple - you define *RoutingRequest* with at least 2 points, start routing with the service and read response as *RoutingResult*. In fact, it is exactly the same as in online routing chapter, just replace onlineRoutingService with offlineRoutingService, see example 2 above.



<script>
		$( "#tabs1" ).tabs();
		$( "#tabs2" ).tabs();
		$( "#tabs3" ).tabs();
		$( "#tabs4" ).tabs();
  		$( "#tabs5" ).tabs();
</script>
