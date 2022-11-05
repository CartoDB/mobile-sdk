## Mobile Geocoding

Terms:

* **Geocoding** is the process of transforming textual address to a location or a geographic identity, basically same as **Address Search**
* **Reverse geocoding** converts geographic location coordinates back to an address.

CARTO Mobile SDK supports both geocoding and reverse geocoding and both are available in online and offline modes. Offline geocoding requires that you download special offline data package through **Package Manager**, see chapter above for more about this.

Mobile SDK supports the following geocoding features:

 - Free-form text input. A specific order of the address components is not assumed.
 - Autocomplete support, get results while typing.
 - Location bias for geocoding. SDK will prefer the addresses close to specified location.
 - Localization of addresses. The geocoding addresses will be returned in specified language, if possible.


### Online Geocoding

CARTO Online geocoding is an API wrapper to external online service provider. CARTO does not currently provide own service, so SDK has built-in support for **Mapbox**, **TomTom** and **Pelias** (former MapZen) online geocoding APIs. For all these services you need either a token or an API key from the service provider, or you can use **custom URL** to connect to your on-premises instance.

The following sample uses TomTom service, but Mapbox and Pelias services can be used instead by replacing `TomTomOnlineGeocodingService` with `PeliasOnlineGeocodingService` or `MapboxOnlineGeocodingService`.

The resulting `GeocodingResultVector` is just a list of results, you can loop through it as you would with any other list. From your `GeocodingResult` objects, you can find `name`, `locality`, `country` etc. properties and display them to the user.

**Note:** As the service is making web requests, then you should NOT call `CalculateAddresses` from your UI thread.

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
  
    TomTomOnlineGeocodingService service = new TomTomOnlineGeocodingService("<your-tomtom-key>");
    GeocodingRequest request = new GeocodingRequest(mapView.getOptions().getBaseProjection(), "Fifth Avenue, New York");

    // Note: Geocoding is a complicated process and should not be done on the main thread
    GeocodingResultVector results = service.calculateAddresses(request);
	                
    {% endhighlight %}
  </div>

  <div id="tab-csharp">
    {% highlight csharp linenos %}

    var service = new TomTomOnlineGeocodingService("<your-tomtom-key>");
    var request = new GeocodingRequest(mapView.Options.BaseProjection, "Fifth Avenue, New York");

    // Note: Geocoding is a complicated process and should not be done on the main thread
    GeocodingResultVector results = service.CalculateAddresses(request);

    {% endhighlight %}
  </div>

  <div id="tab-objectivec">
    {% highlight objc linenos %}
  
    NTTomTomOnlineGeocodingService* service = [[NTTomTomOnlineGeocodingService alloc] initWithApiKey:@"<your-tomtom-key>"];
    NTProjection* projection = [[self.mapView getOptions] getBaseProjection];
    NTGeocodingRequest* request = [[NTGeocodingRequest alloc] initWithProjection:projection query:@"text"];

    // Note: Geocoding is a complicated process and should not be done on the main thread
    NTGeocodingResultVector* results = [service calculateAddresses:request];

    {% endhighlight %}
  </div>

  <div id="tab-swift">
    {% highlight swift linenos %}
  
    let service = NTTomTomOnlineGeocodingService(apiKey: "<your-tomtom-key>")
    let request = NTGeocodingRequest(projection: self.contentView.map.getOptions().getBaseProjection(), query: "Fifth Avenue, New York")

    // Note: Geocoding is a complicated process and should not be done on the main thread
    let results = self.service.calculateAddresses(request)

    {% endhighlight %}
  </div>

  <div id="tab-kotlin">
    {% highlight kotlin linenos %}

    val service = TomTomOnlineGeocodingService("<your-tomtom-key>")
    val request = GeocodingRequest(map.options.baseProjection, "Fifth Avenue, New York")

    // Note: Geocoding is a complicated process and should not be done on the main thread
    val results = service!!.calculateAddresses(request)

    {% endhighlight %}
  </div>
</div>


### Reverse Geocoding

Like online geocoding, online reverse geocoding is available through multiple online service providers. SDK has built-in support for Mapbox, TomTom and Pelias (former MapZen) reverse geocoders.

For these services you need to get a token or an API key from the service provider, or use custom URL as optional method for on-premises servers.

The following sample uses TomTom service, but Mapbox and Pelias services can be used instead by replacing `TomTomOnlineReverseGeocodingService` with `PeliasOnlineReverseGeocodingService` or `MapboxOnlineReverseGeocodingService`.

**Note:** As the service is making web requests, then you should NOT call `CalculateAddresses` from your UI thread.


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

    ReverseGeocodingService service = new TomTomOnlineReverseGeocodingService("<your-tomtom-token>");

    // Center of New York. Reverse Geocoding these coordinates will find City Hall Park
    MapPos newYork = projection.fromLatLong(40.7128, -74.0059);
    ReverseGeocodingRequest request = new ReverseGeocodingRequest(baseProjection, newYork);
    float meters = 125.0f;
    request.setSearchRadius(meters);

    // Note: Reverse geocoding is a complicated process and should not be done on the main thread
    GeocodingResultVector results = service.calculateAddresses(request);
        
    {% endhighlight %}
  </div>

  <div id="tab-csharp">
    {% highlight csharp linenos %}

    var service = new TomTomOnlineReverseGeocodingService("<your-tomtom-token>");

    // Center of New York. Reverse Geocoding these coordinates will find City Hall Park
    MapPos newYork = projection.FromLatLong(40.7128, -74.0059);
    var request = new ReverseGeocodingRequest(projection, newYork);
    var meters = 125.0f;
    request.SearchRadius = meters;

    // Note: Reverse geocoding is a complicated process and should not be done on the main thread
    GeocodingResultVector results = service.CalculateAddresses(request);

    {% endhighlight %}
  </div>

  <div id="tab-objectivec">
    {% highlight objc linenos %}

    NTReverseGeocodingService* service = [[NTTomTomOnlineReverseGeocodingService alloc] initWithApiKey:@"<your-tomtom-key>"];

    // Center of New York. Reverse Geocoding these coordinates will find City Hall Park
    NTMapPos* newYork = [projection fromLat:40.7128 lng:-74.0059];
    NTReverseGeocodingRequest* request = [[NTReverseGeocodingRequest alloc] initWithProjection:projection location:newYork];
    float meters = 125.0f;
    [request setSearchRadius:meters];
        
    // Note: Reverse geocoding is a complicated process and should not be done on the main thread
    NTGeocodingResultVector* results = [self.service calculateAddresses: request];

    {% endhighlight %}
  </div>

  <div id="tab-swift">
    {% highlight swift linenos %}

    let service = NTTomTomOnlineReverseGeocodingService(apiKey: "<your-tomtom-key>")

    // Center of New York. Reverse Geocoding these coordinates will find City Hall Park
    let newYork = projection.fromLat(40.7128, lng: -74.0059)
    let request = NTReverseGeocodingRequest(projection: projection, location: newYork)
    let meters: Float = 125.0
    request?.setSearchRadius(meters)
        
    // Note: Reverse geocoding is a complicated process and should not be done on the main thread
    let results = service.calculateAddresses(request)

    {% endhighlight %}
  </div>

  <div id="tab-kotlin">
    {% highlight kotlin linenos %}

    val service = TomTomOnlineReverseGeocodingService("<your-tomtom-key>")

    // Center of New York. Reverse Geocoding these coordinates will find City Hall Park
    val newYork = contentView?.projection?.fromLatLong(40.7128, -74.0059)
    val request = ReverseGeocodingRequest(contentView?.projection, newYork)
    val meters = 125.0f
    request.searchRadius = meters

    // Note: Reverse geocoding is a complicated process and should not be done on the main thread
    val results = service?.calculateAddresses(request)
                
    {% endhighlight %}
  </div>
</div>

### Offline Geocoding

CARTO Mobile SDK also supports offline geocoding and reverse geocoding, In order to geocode offline, you must first download the required country geocoding data packages. 

**Note:** You will not be able to geocode without downloaded data packages, and the geocoding packages are different from Offline Map packages, do not mix them up !

The list of country packages for geocoding is the same as other offline maps. See [Offline Map Packages](https://github.com/CartoDB/mobile-sdk/wiki/List-of-Offline-map-packages) for the full list of offline packages. The download size of an offline geocoding package is somewhat smaller (10-40%) than the size of the corresponding offline map package.

First, you need to initialize a package manager and a listener to download packages, and then download package for the country what you need. See **PackageManager page** above to find more about offline packages. If relevant package(s) are downloaded and (reverse) geocoding service is ready, you can start geocoding:

Use your `PackageManager` within `PackageManagerReverseGeocodingService` or `PackageManagerGeocodingService` as given in sample below. Then define the `GeocodingRequest` and start geocoding using service's `calculateAddresses` and read response as `GeocodingResult`, just as in Online Geocoding and Reverse Geocoding samples above.


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

    // Note: Provide an absolute path for your geocoding package folder
    CartoPackageManager packageManager = new CartoPackageManager("geocoding:carto.streets", "folder/geocodingpackages");
    ...
    // Now define also PackageManagerListener and download your selected package or map area, wait via listener until it is downloaded!


    // Geocoding service
    PackageManagerGeocodingService service = new PackageManagerGeocodingService(packageManager);

    // Reverse geocoding service
    PackageManagerReverseGeocodingService service = new PackageManagerReverseGeocodingService(packageManager);

    {% endhighlight %}
  </div>

  <div id="tab-csharp">
    {% highlight csharp linenos %}
  
    // Note: Provide an absolute path for your geocoding package folder
    var packageManager = new CartoPackageManager("geocoding:carto.streets", "folder/geocodingpackages");
    ...
    // Now define also PackageManagerListener and download your selected package or map area, wait via listener until it is downloaded!

    // Geocoding service
    var service = new PackageManagerGeocodingService(packageManager);

    // Reverse geocoding service
    var service = new PackageManagerReverseGeocodingService(packageManager);

    {% endhighlight %}
  </div>

  <div id="tab-objectivec">
    {% highlight objc linenos %}

    // Note: Provide an absolute path for your geocoding package folder
    NTCartoPackageManager* packageManager = [[NTCartoPackageManager alloc] initWithSource:@"geocoding:carto.streets" dataFolder:@"folder/geocodingpackages"];
    ...
    // Now define also PackageManagerListener and download your selected package or map area, wait via listener until it is downloaded!

    // Geocoding service
    NTPackageManagerGeocodingService* service = [[NTPackageManagerGeocodingService alloc] initWithPackageManager:self.packageManager];

    // Reverse geocoding service
    NTPackageManagerReverseGeocodingService* service = [[NTPackageManagerReverseGeocodingService alloc] initWithPackageManager:self.packageManager];

    {% endhighlight %}
  </div>

  <div id="tab-swift">
    {% highlight swift linenos %}
  
    // Note: Provide an absolute path for your geocoding package folder
    let packageManager = NTCartoPackageManager(source: "geocoding:carto.streets", dataFolder: "folder/geocodingpackages")
    ...
    // Now define also PackageManagerListener and download your selected package or map area, wait via listener until it is downloaded!

    // Geocoding service
    let service = NTPackageManagerGeocodingService(packageManager: packageManager)

    // Reverse geocoding service
    let service = NTPackageManagerReverseGeocodingService(packageManager: packageManager)

    {% endhighlight %}
  </div>

  <div id="tab-kotlin">
    {% highlight kotlin linenos %}

    // Note: Provide an absolute path for your geocoding package folder
    val packageManager = CartoPackageManager("geocoding:carto.streets", "folder/geocodingpackages")
    ...
    // Now define also PackageManagerListener and download your selected package or map area, wait via listener until it is downloaded!

    // Geocoding service
    val service = PackageManagerGeocodingService(packageManager)

    // Reverse geocoding service
    val service = PackageManagerReverseGeocodingService(packageManager)

    {% endhighlight %}
  </div>
</div>
