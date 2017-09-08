# Mobile Geocoding

**Geocoding** is the computational process of transforming a postal address description to a location on the Earth's surface (spatial representation in numerical coordinates). **Reverse geocoding**, on the other hand, converts geographic coordinates to a description of a location, usually the name of a place or an addressable location. [Wikipedia](https://en.wikipedia.org/wiki/Geocoding)

In layman's terms, geocoding is when you enter an address (text) and the output is latitude/longitude, reverse geocoding is when you click on the map, and it finds a nearby address or point of interest.

This section describes how to access prepackaged routing code from our Sample Apps and how to implement geocoding and reverse geocoding in your mobile app.

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

Online geocoding is available through [Pelias](https://github.com/pelias/pelias). You will need your own Mapzen API key. Sign-up at [https://mapzen.com/](https://mapzen.com/) to receive an API key.

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
  
PeliasOnlineGeocodingService service = new PeliasOnlineGeocodingService("<your-mapzen-api-key>");
GeocodingRequest request = new GeocodingRequest(mapView.getOptions().getBaseProjection(), "text");

// Note: Geocoding is a complicated process and shouldn't be done on the main thread
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

Service = new PeliasOnlineGeocodingService(<your-mapzen-api-key>);
var request = new GeocodingRequest(mapView.Options.BaseProjection, text);

// Note: Geocoding is a complicated process and shouldn't be done on the main thread
GeocodingResultVector results = Service.CalculateAddresses(request);

  {% endhighlight %}
  </div>

   <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--objective-c">
  {% highlight objc %}
  
NTPeliasOnlineGeocodingService *service = [[NTPeliasOnlineGeocodingService alloc]initWithApiKey:@"<your-mapzen-api-key>"];
NTProjection *projection = [[self.mapView getOptions] getBaseProjection];
NTGeocodingRequest *request = [[NTGeocodingRequest alloc]initWithProjection:projection query:@"text"];

// Note: Geocoding is a complicated process and shouldn't be done on the main thread
NTGeocodingResultVector *results = [service calculateAddresses:request];

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--swift">
  {% highlight swift %}
  
let service = NTPeliasOnlineGeocodingService(apiKey: "<your-mapzen-api-key>")
let request = NTGeocodingRequest(projection: self.contentView.map.getOptions().getBaseProjection(), query: text)

// Note: Geocoding is a complicated process and shouldn't be done on the main thread
let results = self.service.calculateAddresses(request)

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--kotlin">
  {% highlight kotlin %}

val service = PeliasOnlineGeocodingService("<your-mapzen-api-key>")
val request = GeocodingRequest(map.options.baseProjection, "text")

// Note: Geocoding is a complicated process and shouldn't be done on the main thread
val results = service!!.calculateAddresses(request)

  {% endhighlight %}
  </div>
</div>

`GeocodingResultVector` is just a list of results, you can loop through it as you would with any other list.

From your `GeocodingResult` objects, you can simply access `name`, `locality`, `country` etc. properties and display them to the user.

### Reverse Geocoding

Online reverse geocoding is also available through [Pelias](https://github.com/pelias/pelias). You will need your own Mapzen API key. Sign up at [https://mapzen.com/](https://mapzen.com/) to receive an API key.

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

service = new PeliasOnlineReverseGeocodingService("<your-mapzen-key>");

// Center of New York. Reverse Geocoding these coordinates should find City Hall Park
MapPos newYork = baseProjection.fromLatLong(40.7128, -74.0059);
ReverseGeocodingRequest request = new ReverseGeocodingRequest(baseProjection, newYork);
float meters = 125.0f;
request.setSearchRadius(meters);

// Note: Reverse geocoding is a complicated process and shouldn't be done on the main thread
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

Service = new PeliasOnlineReverseGeocodingService("<your-mapzen-key>");

// Center of New York. Reverse Geocoding these coordinates should find City Hall Park
MapPos newYork = projection.FromLatLong(40.7128, -74.0059);

var request = new ReverseGeocodingRequest(projection, newYork);
var meters = 125.0f;
request.SearchRadius = meters;

// Note: Reverse geocoding is a complicated process and shouldn't be done on the main thread
GeocodingResultVector results = Service.CalculateAddresses(request);

  {% endhighlight %}
  </div>

   <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--objective-c">
  {% highlight objc %}

self.service = [[NTPeliasOnlineReverseGeocodingService alloc]initWithApiKey:@"<mapzen-api-key>"];

NTProjection *projection = [self.controller getProjection];
// Center of New York. Reverse Geocoding these coordinates should find City Hall Park
NTMapPos *newYork = [projection fromLat:40.7128 lng:-74.0059];
NTReverseGeocodingRequest *request = [[NTReverseGeocodingRequest alloc]initWithProjection:projection location:newYork];
[request setSearchRadius:125.0f];
    
// Note: Reverse geocoding is a complicated process and shouldn't be done on the main thread
NTGeocodingResultVector *results = [self.service calculateAddresses: request];

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--swift">
  {% highlight swift %}

service = NTPeliasOnlineReverseGeocodingService(apiKey: "<your-mapzen-key>")

// Center of New York. Reverse Geocoding these coordinates should find City Hall Park
let newYork = projection.fromLat(40.7128, lng: -74.0059)
let request = NTReverseGeocodingRequest(projection: projection, location: newYork)
    
let meters: Float = 125.0
request?.setSearchRadius(meters)
    
// Note: Reverse geocoding is a complicated process and shouldn't be done on the main thread
let results = service.calculateAddresses(request)

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--kotlin">
  {% highlight kotlin %}

service = PeliasOnlineReverseGeocodingService("<your-mapzen-key>")

// Center of New York. Reverse Geocoding these coordinates should find City Hall Park
val newYork = contentView?.projection?.fromLatLong(40.7128, -74.0059)
val request = ReverseGeocodingRequest(contentView?.projection, newYork)
val meters = 125.0f
request.searchRadius = meters

// Note: Reverse geocoding is a complicated process and shouldn't be done on the main thread
val results = service?.calculateAddresses(request)
                
  {% endhighlight %}
  </div>
</div>

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

### Offline Geocoding

- The list of country packages for geocoding is the same as other offline maps. See [Offline Map Packages](/docs/carto-engine/mobile-sdk/offline-maps/#offline-map-packages) for the full list of offline packages.

- The download size of an offline geocoding package is somewhat smaller (10-40%) than the size of the corresponding offline map package.

Offline geocoding requires a more complicated preparation of your offline map packages, listener events, package initialization, and geocoding calculation parameters.

First, you need to initialize a package manager and a listener to download packages. View the [PackageManager](/docs/carto-engine/mobile-sdk/package-manager/) documentation to find more about offline packages.

If all packages are downloaded and routing service is ready, you can start geocoding:

- Create the `PackageManagerReverseGeocodingService` or `PackageManagerGeocodingService`. The constructor takes your package manager as an argument.

-  Define the *RoutingRequest* with at least two points. Start routing with the service and read response as *RoutingResult*.

- Calculate the route with the `calculateRoute` request.

**Note:** This step is identical to the [online geocoding calculation code](/docs/carto-engine/mobile-sdk/mobile-geocoding/#online-geocoding) or [online reverse geocoding calculation code](/docs/carto-engine/mobile-sdk/mobile-geocoding/#online-reverse-geocoding).
