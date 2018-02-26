## Mobile Geocoding

As per [Wikipedia](https://en.wikipedia), "**Geocoding** is the computational process of transforming a postal address description to a location on the Earth's surface (spatial representation in numerical coordinates). **Reverse geocoding** converts geographic coordinates to a description of a location, usually the name of a place or an addressable location."

In layman's terms, geocoding is when you enter an address (text) and the output is latitude/longitude. Reverse geocoding is when you click on the map and it finds a nearby address, or point of interest.

### Existing Samples

For minimal geocoding implementation, use our sample app code for different mobile platforms. You can add this sample code to your mobile project.

- iOS:
  - [Sample app repository](https://github.com/CartoDB/mobile-ios-samples)
  - [`BaseGeocodingController.m`](https://github.com/CartoDB/mobile-ios-samples/blob/master/AdvancedMap.Objective-C/AdvancedMap/BaseGeocodingController.m)

- Android: 
  - [Sample app repository](https://github.com/CartoDB/mobile-android-samples)
  - [`BaseGeocodingActivity.java`](https://github.com/CartoDB/mobile-android-samples/blob/master/AdvancedMapAndroid/app/src/main/java/com/carto/advancedmap/sections/geocoding/base/BaseGeocodingActivity.java)

- Xamarin (Android, iOS):
  - [Sample app repository](https://github.com/CartoDB/mobile-dotnet-samples)
  - [`Geocoding.cs`](https://github.com/CartoDB/mobile-dotnet-samples/blob/master/Shared/PackageManagerClient/Geocoding/Geocoding.cs)
  
### Online Geocoding

Online geocoding is available through MapBox's geocoding service. You will need your own Mapbox token. Sign up at [https://www.mapbox.com/](https://www.mapbox.com/) to obtain your token.

Implement online geocoding to initialize the service, create the request, and calculate addresses:

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
  
MapBoxOnlineGeocodingService service = new MapBoxOnlineGeocodingService("<your-mapbox-token>");
GeocodingRequest request = new GeocodingRequest(mapView.getOptions().getBaseProjection(), "Fifth Avenue, New York");

// Note: Geocoding is a complicated process and should not be done on the main thread
Thread thread = new Thread(new Runnable() {
    @Override
    public void run() {
		try {
		    GeocodingResultVector results = service.calculateAddresses(request);
		} catch (IOException e) {
		    e.printStackTrace();
		    return;
		}
	}
}

thread.start();
	                
  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--csharp">
  {% highlight csharp %}

Service = new MapBoxOnlineGeocodingService("<your-mapbox-token>");
var request = new GeocodingRequest(mapView.Options.BaseProjection, "Fifth Avenue, New York");

// Note: Geocoding is a complicated process and should not be done on the main thread
GeocodingResultVector results = Service.CalculateAddresses(request);

  {% endhighlight %}
  </div>

   <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--objective-c">
  {% highlight objc %}
  
NTMapBoxOnlineGeocodingService *service = [[NTMapBoxOnlineGeocodingService alloc]initWithApiKey:@"<your-mapbox-key>"];
NTProjection *projection = [[self.mapView getOptions] getBaseProjection];
NTGeocodingRequest *request = [[NTGeocodingRequest alloc]initWithProjection:projection query:@"text"];

// Note: Geocoding is a complicated process and should not be done on the main thread
NTGeocodingResultVector *results = [service calculateAddresses:request];

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--swift">
  {% highlight swift %}
  
let service = NTMapBoxOnlineGeocodingService(apiKey: "<your-mapbox-key>")
let request = NTGeocodingRequest(projection: self.contentView.map.getOptions().getBaseProjection(), query: "Fifth Avenue, New York")

// Note: Geocoding is a complicated process and should not be done on the main thread
let results = self.service.calculateAddresses(request)

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--kotlin">
  {% highlight kotlin %}

val service = MapBoxOnlineGeocodingService("<your-mapbox-token>")
val request = GeocodingRequest(map.options.baseProjection, "Fifth Avenue, New York")

// Note: Geocoding is a complicated process and should not be done on the main thread
val results = service!!.calculateAddresses(request)

  {% endhighlight %}
  </div>
</div>

`GeocodingResultVector` is just a list of results, you can loop through it as you would with any other list.

From your `GeocodingResult` objects, you can simply access `name`, `locality`, `country` etc. properties and display them to the user.

### Reverse Geocoding

Online reverse geocoding is also available through MapBox's geocoding service. You will need your own Mapbox token. Sign up at [https://www.mapbox.com/](https://www.mapbox.com/) obtain your token.

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

service = new MapBoxOnlineReverseGeocodingService("<your-mapbox-token>");

// Center of New York. Reverse Geocoding these coordinates will find City Hall Park
MapPos newYork = baseProjection.fromLatLong(40.7128, -74.0059);
ReverseGeocodingRequest request = new ReverseGeocodingRequest(baseProjection, newYork);
float meters = 125.0f;
request.setSearchRadius(meters);

// Note: Reverse geocoding is a complicated process and should not be done on the main thread
try {
    GeocodingResultVector results = service.calculateAddresses(request);
} catch (IOException e) {
    e.printStackTrace();
    return;
}
        
  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--csharp">
  {% highlight csharp %}

Service = new MapBoxOnlineReverseGeocodingService("<your-mapbox-token>");

// Center of New York. Reverse Geocoding these coordinates will find City Hall Park
MapPos newYork = projection.FromLatLong(40.7128, -74.0059);

var request = new ReverseGeocodingRequest(projection, newYork);
var meters = 125.0f;
request.SearchRadius = meters;

// Note: Reverse geocoding is a complicated process and should not be done on the main thread
GeocodingResultVector results = Service.CalculateAddresses(request);

  {% endhighlight %}
  </div>

   <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--objective-c">
  {% highlight objc %}

self.service = [[NTMapBoxOnlineReverseGeocodingService alloc]initWithApiKey:@"<mapbox-token>"];

NTProjection *projection = [self.controller getProjection];
// Center of New York. Reverse Geocoding these coordinates will find City Hall Park
NTMapPos *newYork = [projection fromLat:40.7128 lng:-74.0059];
NTReverseGeocodingRequest *request = [[NTReverseGeocodingRequest alloc]initWithProjection:projection location:newYork];
[request setSearchRadius:125.0f];
    
// Note: Reverse geocoding is a complicated process and should not be done on the main thread
NTGeocodingResultVector *results = [self.service calculateAddresses: request];

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--swift">
  {% highlight swift %}

service = NTMapBoxOnlineReverseGeocodingService(apiKey: "<your-mapbox-token>")

// Center of New York. Reverse Geocoding these coordinates will find City Hall Park
let newYork = projection.fromLat(40.7128, lng: -74.0059)
let request = NTReverseGeocodingRequest(projection: projection, location: newYork)
    
let meters: Float = 125.0
request?.setSearchRadius(meters)
    
// Note: Reverse geocoding is a complicated process and should not be done on the main thread
let results = service.calculateAddresses(request)

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--kotlin">
  {% highlight kotlin %}

service = MapBoxOnlineReverseGeocodingService("<your-mapbox-token>")

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

CARTO Mobile SDK also supports offline geocoding and reverse geocoding, In order to geocode offline, you must first download the required country packages. You will not be able to geocode without data!

The list of country packages for geocoding is the same as other offline maps. See [Offline Map Packages](https://github.com/CartoDB/mobile-sdk/wiki/List-of-Offline-map-packages) for the full list of offline packages. The download size of an offline geocoding package is somewhat smaller (10-40%) than the size of the corresponding offline map package.

Offline geocoding requires a more complicated preparation of your offline map packages, listener events, package initialization, and geocoding calculation parameters.

First, you need to initialize a package manager and a listener to download packages. See [PackageManager](/docs/carto-engine/mobile-sdk/packagemanager/) documentation to find more about offline packages.

If relevant packages are downloaded and (reverse) geocoding service is ready, you can start geocoding:

Create a `PackageManager` and `PackageManagerReverseGeocodingService` or `PackageManagerGeocodingService`. Then define the `GeocodingRequest` with at least two points. Start geocoding with the service and read response as `GeocodingResult`:

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

CartoPackageManager packageManager;
try {
	// Note: Provide an absolute path for your geocoding package folder
    packageManager = new CartoPackageManager("geocoding:carto.streets", "folder/geocodingpackages");
}
catch (IOException e) {
    Log.e(MapApplication.LOG_TAG, "Exception: " + e);
    finish();
}

// Geocoding service
PackageManagerGeocodingService service = new PackageManagerGeocodingService(manager);

// Reverse geocoding service
PackageManagerReverseGeocodingService service = new PackageManagerReverseGeocodingService(manager);

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--csharp">
  {% highlight csharp %}
  
// Note: Provide an absolute path for your geocoding package folder
var Manager = new CartoPackageManager("geocoding:carto.streets", "folder/geocodingpackages");

// Geocoding service
var Service = new PackageManagerGeocodingService(Manager);

// Reverse geocoding service
var Service = new PackageManagerReverseGeocodingService(Manager);

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--objective-c">
  {% highlight objc %}

// Note: Provide an absolute path for your geocoding package folder
NTCartoPackageManager *packageManager = [[NTCartoPackageManager alloc] initWithSource:@"geocoding:carto.streets" dataFolder:@"folder/geocodingpackages"];

// Geocoding service
NTPackageManagerGeocodingService *service = [[NTPackageManagerGeocodingService alloc] initWithPackageManager:self.packageManager];

// Reverse geocoding service
NTPackageManagerReverseGeocodingService *service = [[NTPackageManagerReverseGeocodingService alloc] initWithPackageManager:self.packageManager];

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--swift">
  {% highlight swift %}
  
// Note: Provide an absolute path for your geocoding package folder
let manager = NTCartoPackageManager(source: "geocoding:carto.streets", dataFolder: "folder/geocodingpackages")

// Geocoding service
let service = NTPackageManagerGeocodingService(packageManager: manager)

// Reverse geocoding service
let service = NTPackageManagerReverseGeocodingService(packageManager: manager)

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--kotlin">
  {% highlight kotlin %}

// Note: Provide an absolute path for your geocoding package folder
val manager = manager = CartoPackageManager("geocoding:carto.streets", "folder/geocodingpackages")

// Geocoding service
val service = PackageManagerGeocodingService(manager)

// Reverse geocoding service
val service = PackageManagerReverseGeocodingService(manager)

  {% endhighlight %}
  </div>
</div>

... Lastly, simply call `calculateAddresses` with the appropriate arguments, as presented in our online geocoding sample.

### Parsing Results

The following example displays how to parse your `GeocodingResult` into a readable string. This is only a suggestion, you can parse results however you prefer.

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
  
public class GeocodingUtils {

    public static String getPrettyAddress(Address address) {

        String string = "";

        if (!address.getName().isEmpty()) {
            string += address.getName();
        }

        if (!address.getStreet().isEmpty()) {
            if (string.length() > 0) {
                string += ", ";
            }
            string += address.getStreet();
        }

        if (!address.getHouseNumber().isEmpty()) {
            string += " " + address.getHouseNumber();
        }

        if (!address.getNeighbourhood().isEmpty()) {
            if (string.length() > 0) {
                string += ", ";
            }
            string += address.getNeighbourhood();
        }

        if (!address.getLocality().isEmpty()) {
            if (string.length() > 0) {
                string += ", ";
            }
            string += address.getLocality();
        }

        if (!address.getCounty().isEmpty()) {
            if (string.length() > 0) {
                string += ", ";
            }
            string += address.getCounty();
        }

        if (!address.getRegion().isEmpty()) {
            if (string.length() > 0) {
                string += ", ";
            }
            string += address.getRegion();
        }

        if (!address.getCountry().isEmpty()) {
            if (string.length() > 0) {
                string += ", ";
            }
            string += address.getCountry();
        }
		
	   // If you use this method to parse your GeocodingResult, the output string will be e.g.
	   // "Fifth Avenue, Fifth Avenue, Gainesville, Alachua County, Florida, United States" or
 	   // "Fifth Street, Fifth Street, Fort Bend County, Texas, United States"
        return string;
    }
}

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--csharp">
  {% highlight csharp %}

public static class GeocodingExtensions
{
    public static string GetPrettyAddress(this GeocodingResult result)
    {
        var parsed = "";
        var address = result.Address;

        if (address.Name.IsNotEmpty())
        {
            parsed += address.Name;
        }

        if (address.Street.IsNotEmpty())
        {
            parsed += parsed.AddCommaIfNecessary();
            parsed += address.Street;
        }

        if (address.HouseNumber.IsNotEmpty())
        {
            parsed += " " + address.HouseNumber;
        }

        if (address.Neighbourhood.IsNotEmpty())
        {
            parsed += parsed.AddCommaIfNecessary();
            parsed += address.Neighbourhood;
        }

        if (address.Locality.IsNotEmpty())
        {
            parsed += parsed.AddCommaIfNecessary();
            parsed += address.Locality;
        }

        if (address.County.IsNotEmpty())
        {
            parsed += parsed.AddCommaIfNecessary();
            parsed += address.County;
        }

        if (address.Region.IsNotEmpty())
        {
            parsed += parsed.AddCommaIfNecessary();
            parsed += address.Region;
        }

        if (address.Country.IsNotEmpty())
        {
            parsed += parsed.AddCommaIfNecessary();
            parsed += address.Country;
        }
        
	   // If you use this method to parse your GeocodingResult, the output string will be e.g.
	   // "Fifth Avenue, Fifth Avenue, Gainesville, Alachua County, Florida, United States" or
 	   // "Fifth Street, Fifth Street, Fort Bend County, Texas, United States"
        return parsed;
    }

    public static string AddCommaIfNecessary(this string original)
    {
        if (original.Length > 0)
        {
            return ", ";
        }

        return "";
    }

    public static bool IsNotEmpty(this string original)
    {
        return !string.IsNullOrWhiteSpace(original);
    }
}

  {% endhighlight %}
  </div>

   <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--objective-c">
  {% highlight objc %}
  
- (NSString *)getPrettyAddress:(NTGeocodingResult *)result {
    
    NTAddress *address = [result getAddress];
    NSString *string = @"";
    
    if ([address getName].length > 0) {
        string = [string stringByAppendingString:[address getName]];
    }
    
    if ([address getStreet].length > 0) {
        if (string.length > 0) {
            string = [string stringByAppendingString:@", "];
        }
        string = [string stringByAppendingString: [address getStreet]];
    }
    
    if ([address getHouseNumber].length > 0) {
        string = [string stringByAppendingString:@" "];
        string = [string stringByAppendingString:[address getHouseNumber]];
    }
    
    if ([address getNeighbourhood].length > 0) {
        if (string.length > 0) {
            string = [string stringByAppendingString:@", "];
        }
        string = [string stringByAppendingString: [address getStreet]];
    }
    
    if ([address getLocality].length > 0) {
        if (string.length > 0) {
            string = [string stringByAppendingString:@", "];
        }
        string = [string stringByAppendingString: [address getLocality]];
    }
    
    if ([address getCounty].length > 0) {
        if (string.length > 0) {
            string = [string stringByAppendingString:@", "];
        }
        string = [string stringByAppendingString: [address getCounty]];
    }
    
    if ([address getRegion].length > 0) {
        if (string.length > 0) {
            string = [string stringByAppendingString:@", "];
        }
        string = [string stringByAppendingString: [address getRegion]];
    }
    
    if ([address getCountry].length > 0) {
        if (string.length > 0) {
            string = [string stringByAppendingString:@", "];
        }
        string = [string stringByAppendingString: [address getCountry]];
    }
    
   // If you use this method to parse your GeocodingResult, the output string will be e.g.
   // "Fifth Avenue, Fifth Avenue, Gainesville, Alachua County, Florida, United States" or
   // "Fifth Street, Fifth Street, Fort Bend County, Texas, United States"
    return string;
}

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--swift">
  {% highlight swift %}
  
extension NTGeocodingResult {
    
    func getPrettyAddress() -> String {
        
        let address = self.getAddress()
        var string = ""
        
        if ((address?.getName().characters.count)! > 0) {
            string += (address?.getName())!
        }
        
        if ((address?.getStreet().characters.count)! > 0) {
            string.addCommaIfNecessary()
            string += (address?.getStreet())!
        }
        
        if ((address?.getHouseNumber().characters.count)! > 0) {
            string += " " + (address?.getHouseNumber())!
        }
        
        if ((address?.getNeighbourhood().characters.count)! > 0) {
            string.addCommaIfNecessary()
            string += (address?.getNeighbourhood())!
        }
        
        if ((address?.getLocality().characters.count)! > 0) {
            string.addCommaIfNecessary()
            string += (address?.getLocality())!
        }
        
        if ((address?.getCounty().characters.count)! > 0) {
            string.addCommaIfNecessary()
            string += (address?.getCounty())!
        }
        
        if ((address?.getRegion().characters.count)! > 0) {
            string.addCommaIfNecessary()
            string += (address?.getRegion())!
        }
        
        if ((address?.getCountry().characters.count)! > 0) {
            string.addCommaIfNecessary()
            string += (address?.getCountry())!
        }
        
    	   // If you use this method to parse your GeocodingResult, the output string will be e.g.
	   // "Fifth Avenue, Fifth Avenue, Gainesville, Alachua County, Florida, United States" or
 	   // "Fifth Street, Fifth Street, Fort Bend County, Texas, United States"
        return string
    }
}

extension String {
    mutating func addCommaIfNecessary() {
        if (characters.count > 0) {
            self += ", "
        }
    }
}
  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--kotlin">
  {% highlight kotlin %}

fun GeocodingResult.getPrettyAddress(): String {

    var string = ""

    if (address.name.isNotEmpty()) {
        string += address.name
    }

    if (address.street.isNotEmpty()) {
        string += string.addCommaIfNecessary()
        string += address.street
    }

    if (address.houseNumber.isNotEmpty()) {
        string += " " + address.houseNumber
    }

    if (address.neighbourhood.isNotEmpty()) {
        string += string.addCommaIfNecessary()
        string += address.neighbourhood
    }

    if (address.locality.isNotEmpty()) {
        string += string.addCommaIfNecessary()
        string += address.locality
    }

    if (address.county.isNotEmpty()) {
        string += string.addCommaIfNecessary()
        string += address.county
    }

    if (address.region.isNotEmpty()) {
        string += string.addCommaIfNecessary()
        string += address.region
    }

    if (address.country.isNotEmpty()) {
        string += string.addCommaIfNecessary()
        string += address.country
    }

   // If you use this method to parse your GeocodingResult, the output string will be e.g.
   // "Fifth Avenue, Fifth Avenue, Gainesville, Alachua County, Florida, United States" or
   // "Fifth Street, Fifth Street, Fort Bend County, Texas, United States"
    return string
}

fun String.addCommaIfNecessary(): String {

    if (length > 0) {
        return ", "
    }

    return ""
}

  {% endhighlight %}
  </div>
</div>
