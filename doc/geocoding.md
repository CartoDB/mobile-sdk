# Mobile Geocoding

**Geocoding** is the computational process of transforming a postal address description to a location on the Earth's surface (spatial representation in numerical coordinates). **Reverse geocoding**, on the other hand, converts geographic coordinates to a description of a location, usually the name of a place or an addressable location. [Wikipedia](https://en.wikipedia.org/wiki/Geocoding)

In layman's terms, geocoding is when you enter an address (text) and the output is latitude/longitude, reverse geocoding is when you click on the map, and it finds a nearby address or point of interest.


This section describes how to access prepackaged routing code from our Sample Apps and how to implement geocoding in your mobile app.

### Prepackaged Sample Code

For minimal routing implementation, use our sample app code for different mobile platforms. You can add this sample code to your mobile project.

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

### Parsing your GeocodingResult

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

    public static String getPrettyAddres(Address address) {

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

### Displaying your GeocodingResult on the Map

If you wish to display your `GeocodingResult` on the map, create a `LocalVectorDataSource`, initialize a `VectorLayer` and add it to the map, then you can add various `VectorElements` to your 
`LocalVectorDataSource`.

**Note:** This function does not work out of the box. Ensure your class contains a **vector element source** and **your mapView** object, as shown in the following example:

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
  
public void showResult(GeocodingResult result, String title, String description, boolean goToPosition) {

        geocodingSource.clear();

        AnimationStyleBuilder animationBuilder = new AnimationStyleBuilder();
        animationBuilder.setRelativeSpeed(2.0f);
        animationBuilder.setFadeAnimationType(AnimationType.ANIMATION_TYPE_SMOOTHSTEP);

        BalloonPopupStyleBuilder builder = new BalloonPopupStyleBuilder();
        builder.setLeftMargins(new BalloonPopupMargins(0, 0, 0, 0));
        builder.setRightMargins(new BalloonPopupMargins(6, 3, 6, 3));
        builder.setCornerRadius(5);
        builder.setAnimationStyle(animationBuilder.buildStyle());
        // Make sure this label is shown on top of all other labels
        builder.setPlacementPriority(10);

        FeatureCollection collection = result.getFeatureCollection();
        int count = collection.getFeatureCount();

        MapPos position = null;
        Geometry geometry;

        Color color = new Color((short)0, (short)100,(short)200, (short)150);

        for (int i = 0; i < count; i++) {
            geometry = collection.getFeature(i).getGeometry();

            PointStyleBuilder pointBuilder = new PointStyleBuilder();
            pointBuilder.setColor(color);

            LineStyleBuilder lineBuilder = new LineStyleBuilder();
            lineBuilder.setColor(color);

            PolygonStyleBuilder polygonBuilder = new PolygonStyleBuilder();
            polygonBuilder.setColor(color);

            if (geometry instanceof PointGeometry) {
                geocodingSource.add(new Point((PointGeometry)geometry, pointBuilder.buildStyle()));
            } else if (geometry instanceof LineGeometry) {
                geocodingSource.add(new Line((LineGeometry)geometry, lineBuilder.buildStyle()));
            }  else if (geometry instanceof PolygonGeometry) {
                geocodingSource.add(new Polygon((PolygonGeometry)geometry, polygonBuilder.buildStyle()));
            } else if (geometry instanceof MultiGeometry) {

                GeometryCollectionStyleBuilder collectionBuilder = new GeometryCollectionStyleBuilder();
                collectionBuilder.setPointStyle(pointBuilder.buildStyle());
                collectionBuilder.setLineStyle(lineBuilder.buildStyle());
                collectionBuilder.setPolygonStyle(polygonBuilder.buildStyle());

                geocodingSource.add(new GeometryCollection((MultiGeometry)geometry, collectionBuilder.buildStyle()));
            }

            position = geometry.getCenterPos();
        }

        if (goToPosition) {
            mapView.setFocusPos(position, 1.0f);
            mapView.setZoom(17.0f, 1.0f);
        }

        BalloonPopup popup = new BalloonPopup(position, builder.buildStyle(), title, description);
        geocodingSource.add(popup);
    }

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--csharp">
  {% highlight csharp %}

// This is an extension method of your LocalVectorDataSource
public static void ShowResult(this LocalVectorDataSource source, MapView map, GeocodingResult result, string title, string description, bool goToPosition)
{
	source.Clear();

	var builder = new BalloonPopupStyleBuilder();
	builder.LeftMargins = new BalloonPopupMargins(0, 0, 0, 0);
	builder.TitleMargins = new BalloonPopupMargins(6, 3, 6, 3);
	builder.CornerRadius = 5;
	// Make sure this label is shown on top of all other labels
	builder.PlacementPriority = 10;
    builder.DescriptionColor = new Carto.Graphics.Color(255, 255, 255, 255);

	FeatureCollection collection = result.FeatureCollection;
	int count = collection.FeatureCount;

    MapPos position = new MapPos();
	Geometry geometry;

	for (int i = 0; i < count; i++)
	{
		geometry = collection.GetFeature(i).Geometry;
		var color = new Carto.Graphics.Color(0, 100, 200, 150);

		var pointBuilder = new PointStyleBuilder();
		pointBuilder.Color = color;

		var lineBuilder = new LineStyleBuilder();
		builder.Color = color;

		var polygonBuilder = new PolygonStyleBuilder();
		polygonBuilder.Color = color;

        VectorElement element = null;

        if (geometry is PointGeometry)
        {
            element = new Point(geometry as PointGeometry, pointBuilder.BuildStyle());
        }
        else if (geometry is LineGeometry)
        {
            element = new Line(geometry as LineGeometry, lineBuilder.BuildStyle());
        }
        else if (geometry is PolygonGeometry)
        {
            element = new Polygon(geometry as PolygonGeometry, polygonBuilder.BuildStyle());
        }
        else if (geometry is MultiGeometry)
        {
            var collectionBuilder = new GeometryCollectionStyleBuilder();
            collectionBuilder.PointStyle = pointBuilder.BuildStyle();
            collectionBuilder.LineStyle = lineBuilder.BuildStyle();
            collectionBuilder.PolygonStyle = polygonBuilder.BuildStyle();

            element = new GeometryCollection(geometry as MultiGeometry, collectionBuilder.BuildStyle());
        }

        if (element != null)
        {
			position = geometry.CenterPos;
			source.Add(element);   
        }
	}

    if (goToPosition)
    {
        map.SetFocusPos(position, 1.0f);
        map.SetZoom(16, 1.0f);
    }

    var popup = new BalloonPopup(position, builder.BuildStyle(), title, description);
    source.Add(popup);
}

  {% endhighlight %}
  </div>

   <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--objective-c">
  {% highlight objc %}
  
- (void)showResult:(NTGeocodingResult *)result title:(NSString *)title description:(NSString *)description goToPosition: (BOOL)goToPosition {
    
    [_geocodingSource clear];
    
    NTAnimationStyleBuilder *animationBuilder = [[NTAnimationStyleBuilder alloc]init];
    [animationBuilder setRelativeSpeed:2.0f];
    [animationBuilder setFadeAnimationType:NT_ANIMATION_TYPE_SMOOTHSTEP];
    
    NTBalloonPopupStyleBuilder *builder = [[NTBalloonPopupStyleBuilder alloc]init];
    [builder setCornerRadius:5];
    [builder setAnimationStyle:[animationBuilder buildStyle]];
    // Make sure this label is shown on top of all other labels
    [builder setPlacementPriority:10];
    
    NTFeatureCollection *collection = [result getFeatureCollection];
    int count = [collection getFeatureCount];
    
    NTColor *color = [[NTColor alloc]initWithR:0 g:100 b:200 a:150];
    
    NTMapPos *position;
    NTGeometry *geometry;
    
    for (int i = 0; i < count; i++) {
        geometry = [[collection getFeature:i] getGeometry];
        
        NTPointStyleBuilder* pointStyleBuilder = [[NTPointStyleBuilder alloc] init];
        [pointStyleBuilder setColor: color];
        
        NTLineStyleBuilder* lineStyleBuilder = [[NTLineStyleBuilder alloc] init];
        [lineStyleBuilder setColor: color];
        
        NTPolygonStyleBuilder* polygonStyleBuilder = [[NTPolygonStyleBuilder alloc] init];
        [polygonStyleBuilder setColor: color];
        
        if ([geometry isKindOfClass:[NTPointGeometry class]]) {
            [_geocodingSource add: [[NTPoint alloc] initWithGeometry:(NTPointGeometry*)geometry style:[pointStyleBuilder buildStyle]]];
        }
        if ([geometry isKindOfClass:[NTLineGeometry class]]) {
            [_geocodingSource add: [[NTLine alloc] initWithGeometry:(NTLineGeometry*)geometry style:[lineStyleBuilder buildStyle]]];
        }
        if ([geometry isKindOfClass:[NTPolygonGeometry class]]) {
            [_geocodingSource add: [[NTPolygon alloc] initWithGeometry:(NTPolygonGeometry*)geometry style:[polygonStyleBuilder buildStyle]]];
        }
        if ([geometry isKindOfClass:[NTMultiGeometry class]]) {
            NTGeometryCollectionStyleBuilder* geomCollectionStyleBuilder = [[NTGeometryCollectionStyleBuilder alloc] init];
            [geomCollectionStyleBuilder setPointStyle:[pointStyleBuilder buildStyle]];
            [geomCollectionStyleBuilder setLineStyle:[lineStyleBuilder buildStyle]];
            [geomCollectionStyleBuilder setPolygonStyle:[polygonStyleBuilder buildStyle]];
            [_geocodingSource add: [[NTGeometryCollection alloc] initWithGeometry:(NTMultiGeometry*)geometry style:[geomCollectionStyleBuilder buildStyle]]];
        }
        position = [geometry getCenterPos];
    }
    
    if (goToPosition) {
        [self.mapView setFocusPos:position durationSeconds:1.0f];
        [self.mapView setZoom:17.0f durationSeconds:1.0f];
    }
    
    NTBalloonPopup *popup = [[NTBalloonPopup alloc]initWithPos:position style:[builder buildStyle] title:title desc:description];
    [_geocodingSource add:popup];
}

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--swift">
  {% highlight swift %}
  
func showResult(result: NTGeocodingResult!, title: String, description: String, goToPosition: Bool) {
    
    source.clear()
    
    let builder = NTBalloonPopupStyleBuilder()
    builder?.setLeftMargins(NTBalloonPopupMargins(left: 0, top: 0, right: 0, bottom: 0))
    builder?.setTitleMargins(NTBalloonPopupMargins(left: 6, top: 3, right: 6, bottom: 3))
    builder?.setCornerRadius(5)
    
    // Make sure this label is shown on top of all other labels
    builder?.setPlacementPriority(10)
    
    let collection = result.getFeatureCollection()
    let count = Int((collection?.getFeatureCount())!)
    
    var position: NTMapPos?
    
    var geometry: NTGeometry?
    
    for var i in 0..<count {
        
        geometry = collection?.getFeature(Int32(i)).getGeometry()
        let color = NTColor(r: 0, g: 100, b: 200, a: 150)
        
        // Build styles for the displayed geometry
        let pointBuilder = NTPointStyleBuilder()
        pointBuilder?.setColor(color)
        
        let lineBuilder = NTLineStyleBuilder()
        lineBuilder?.setColor(color)
        
        let polygonBuilder = NTPolygonStyleBuilder()
        polygonBuilder?.setColor(color)
        
        var element: NTVectorElement?
        
        if let pointGeometry = geometry as? NTPointGeometry {
            element = NTPoint(geometry: pointGeometry, style: pointBuilder?.buildStyle())
            
        } else if let lineGeometry = geometry as? NTLineGeometry {
            element = NTLine(geometry: lineGeometry, style: lineBuilder?.buildStyle())
        } else if let polygonGeometry = geometry as? NTPolygonGeometry {
            element = NTPolygon(geometry: polygonGeometry, style: polygonBuilder?.buildStyle())
            
        } else if let multiGeometry = geometry as? NTMultiGeometry {
            
            let collectionBuilder = NTGeometryCollectionStyleBuilder()
            collectionBuilder?.setPointStyle(pointBuilder?.buildStyle())
            collectionBuilder?.setLineStyle(lineBuilder?.buildStyle())
            collectionBuilder?.setPolygonStyle(polygonBuilder?.buildStyle())
            
            element = NTGeometryCollection(geometry: multiGeometry, style: collectionBuilder?.buildStyle())
        }
        
        position = geometry?.getCenterPos()
        source.add(element)
        
        i += 1
    }
    
    if (goToPosition) {
        map.setFocus(position, durationSeconds: 1.0)
        map.setZoom(16, durationSeconds: 1)
    }
    
    let popup = NTBalloonPopup(pos: position, style: builder?.buildStyle(), title: title, desc: description)
    source.add(popup)
}

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--kotlin">
  {% highlight kotlin %}

fun showResult(result: com.carto.geocoding.GeocodingResult, title: String, description: String, goToPosition: Boolean) {

        source.clear()

        val animationBuilder = AnimationStyleBuilder()
        animationBuilder.relativeSpeed = 2.0f
        animationBuilder.fadeAnimationType = AnimationType.ANIMATION_TYPE_SMOOTHSTEP

        val builder = com.carto.styles.BalloonPopupStyleBuilder()
        builder.leftMargins = com.carto.styles.BalloonPopupMargins(0, 0, 0, 0)
        builder.titleMargins = com.carto.styles.BalloonPopupMargins(6, 3, 6, 3)
        builder.cornerRadius = 5
        builder.animationStyle = animationBuilder.buildStyle()

        // Make sure this label is shown on top of all other labels
        builder.placementPriority = 10

        val collection = result.featureCollection
        val count = collection.featureCount

        var position: com.carto.core.MapPos? = null

        var geometry: com.carto.geometry.Geometry?

        for (i in 0..count - 1) {

            geometry = collection?.getFeature(i)!!.geometry
            val color = com.carto.graphics.Color(0, 100, 200, 150)

            // Build styles for the displayed geometry
            val pointBuilder = com.carto.styles.PointStyleBuilder()
            pointBuilder.color = color

            val lineBuilder = com.carto.styles.LineStyleBuilder()
            lineBuilder.color = color

            val polygonBuilder = com.carto.styles.PolygonStyleBuilder()
            polygonBuilder.color = color

            var element: com.carto.vectorelements.VectorElement? = null

            if (geometry is com.carto.geometry.PointGeometry) {
                element = com.carto.vectorelements.Point(geometry, pointBuilder.buildStyle())

            } else if (geometry is com.carto.geometry.LineGeometry) {
                element = com.carto.vectorelements.Line(geometry, lineBuilder.buildStyle())
            } else if (geometry is com.carto.geometry.PolygonGeometry) {
                element = com.carto.vectorelements.Polygon(geometry, polygonBuilder.buildStyle())

            } else if (geometry is com.carto.geometry.MultiGeometry) {

                val collectionBuilder = com.carto.styles.GeometryCollectionStyleBuilder()
                collectionBuilder.pointStyle = pointBuilder.buildStyle()
                collectionBuilder.lineStyle = lineBuilder.buildStyle()
                collectionBuilder.polygonStyle = polygonBuilder.buildStyle()

                element = com.carto.vectorelements.GeometryCollection(geometry, collectionBuilder.buildStyle())
            }

            position = geometry?.centerPos
            source.add(element)
        }

        if (goToPosition) {

            map.setFocusPos(position, 1.0f)
            map.setZoom(17.0f, 1.0f)
        }

        val popup = com.carto.vectorelements.BalloonPopup(position, builder.buildStyle(), title, description)
        source.add(popup)
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
