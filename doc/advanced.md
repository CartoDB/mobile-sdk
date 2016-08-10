# Raster tile sources

With Carto Mobile **Pro and Enterprise plans** (not Lite) you can use any raster tile map source: your own, 3rd party etc. Here is collection of various raster map tile sources.

To use following tile sources use *HTTPTileDataSource*, and create *RasterTileLayer* using it. The DataSource constructor takes URL pattern which are given below, and requires also minimum and maximum zoom levels. For all above 0 is minimum zoom, and maximum varies from 14 to 19.

## Code sample
<pre class="brush: java">
  TileDataSource tileDataSource = new HTTPTileDataSource(
  	0, 18, 
    "http://your-url-with-placeholders-see-below");
  RasterTileLayer layer = new RasterTileLayer(tileDataSource);
  mapView.getLayers().add(layer);
</pre>

## <a href="https://carto.com/location-data-services/basemaps/">CARTO Positron tiles</a>
<pre>http://a.basemaps.cartocdn.com/dark_all/{zoom}/{x}/{y}.png</pre>
<img src="http://a.basemaps.cartocdn.com/light_all/17/65490/43588.png"/>

Free.

## <a href="https://carto.com/location-data-services/basemaps/">CARTO Dark Matter tiles</a>
<pre>http://a.basemaps.cartocdn.com/dark_all/{zoom}/{x}/{y}.png</pre>
<img src="http://a.basemaps.cartocdn.com/dark_all/17/65490/43588.png"/>

Free.

## <a href="http://www.osm.org">OpenStreetMap Mapnik tiles</a>
<pre>http://a.tile.openstreetmap.org/{zoom}/{x}/{y}.png</pre>
<img src="http://b.tile.openstreetmap.org/15/5241/12661.png"/>

Free for limited use. See <a href="http://wiki.openstreetmap.org/wiki/Tile_usage_policy">tile usage policy</a>
## <a href="http://www.mapbox.com">MapBox raster tiles</a>
<pre>http://api.mapbox.com/v4/{mapid}/{zoom}/{x}/{y}.png?access_token=&lt;your access token&gt;</pre>
<img src="http://api.tiles.mapbox.com/v3/nutiteq.map-f0sfyluv/17/65490/43588.png"/>

See <a href=" https://www.mapbox.com/developers/api/maps/">MapBox API spec</a> for details and to create access token. <a href='http://mapbox.com/about/maps' target='_blank'>MapBox Terms & Feedback</a>. 

## <a href="http://maps.stamen.com">Stamen Toner</a>
<pre>http://a.tile.stamen.com/toner/{zoom}/{x}/{y}.png</pre>
<img src="http://a.tile.stamen.com/toner/13/1310/3166.png"/>

Please contact http://www.stamen.com
## <a href="http://maps.stamen.com/#watercolor/12/37.7706/-122.3782">Stamen Watercolor</a>
<pre>http://tile.stamen.com/watercolor/{zoom}/{x}/{y}.png</pre>
<img src="http://tile.stamen.com/watercolor/12/654/1583.jpg"/>

Please contact http://www.stamen.com
## <a href="http://maps.stamen.com/#terrain/12/37.7706/-122.3782">Stamen Terrain</a> - US only
<pre>http://tile.stamen.com/terrain/{zoom}/{x}/{y}.png</pre>
<img src="http://tile.stamen.com/terrain/12/654/1583.jpg"/>

Please contact http://www.stamen.com

## Microsoft Bing Maps
<pre>http://ecn.t3.tiles.virtualearth.net/tiles/r{quadkey}.png?g=1&mkt=en-US&shading=hill&n=z</pre>

<img width="256" height="256" src="http://ecn.t3.tiles.virtualearth.net/tiles/r1202.png?g=1&mkt=en-US&shading=hill&n=z" alt=""/>

<a href="http://www.microsoft.com/maps/product/terms.html">Commercial terms of Bing Maps API</a>. Must use <a href="http://msdn.microsoft.com/en-us/library/ff701716.aspx">Get Imagery Metadata Bing Service </a> to get supported URL parameters. 

## Microsoft Bing Aerials
<pre>http://ecn.t3.tiles.virtualearth.net/tiles/a{quadkey}.jpeg?g=1&mkt=en-US</pre>

<img width="256" height="256"  src="http://ecn.t3.tiles.virtualearth.net/tiles/a032010110123333.jpeg?g=1&mkt=en-US" alt=""/>

<a href="http://www.microsoft.com/maps/product/terms.html">Commercial terms of Bing Maps API</a>. Must use <a href="http://msdn.microsoft.com/en-us/library/ff701716.aspx">Get Imagery Metadata Bing Service</a> to get supported URL parameters.

## ESRI World Imagery (and other ESRI basemaps)
<pre>http://services.arcgisonline.com/arcgis/rest/services/World_Imagery/MapServer/tile/{zoom}/{y}/{x}</pre>

<img width="256" height="256"  src="http://server.arcgisonline.com/ArcGIS/rest/services/World_Imagery/MapServer/tile/16/24351/35037" alt=""/>

<a href="http://www.arcgis.com/home/item.html?id=10df2279f9684e4a9f6a7f08febac2a9">Terms and more info</a>. ArcGIS Developer service subscription is required (from $20/mo).

## OpenCycleMap

<pre>http://a.tile.opencyclemap.org/cycle/{zoom}/{x}/{y}.png</pre>
<img width="256" height="256"  src="http://b.tile.opencyclemap.org/cycle/13/4092/2719.png" alt=""/>

© <a href="http://www.opencyclemap.org">OpenCycleMap</a>

## OpenCycleMap Transport

<pre>http://a.tile2.opencyclemap.org/transport/{zoom}/{x}/{y}.png</pre>
<img width="256" height="256"  src="http://a.tile2.opencyclemap.org/transport/16/35209/21491.png" alt=""/>

Free to use. Source: <a href="http://www.thunderforest.com/transport/">Thunderforest</a>

## USGS Topo National Map (US Only)

<pre>http://basemap.nationalmap.gov/arcgis/rest/services/USGSTopo/MapServer/WMTS/tile/1.0.0/USGSTopo/default/GoogleMapsCompatible/{zoom}/{y}/{x}</pre>
<img width="256" height="256"  src="http://basemap.nationalmap.gov/arcgis/rest/services/USGSTopo/MapServer/WMTS/tile/1.0.0/USGSTopo/default/GoogleMapsCompatible/13/3162/1306" alt="USGS Topo"/>

**Copyright**: USGS The National Map: National Boundaries Dataset, National Elevation Dataset, Geographic Names Information System, National Hydrography Dataset, National Land Cover Database, National Structures Dataset, and National Transportation Dataset; U.S. Census Bureau - TIGER/Line; HERE Road Data. <a href="http://basemap.nationalmap.gov/arcgis/rest/services/USGSTopo/MapServer">USGS MapServer</a>

### References


* [OSM Wiki TMS page](http://wiki.openstreetmap.org/wiki/TMS)
* [OSM Featured tiles](http://wiki.openstreetmap.org/wiki/Featured_tiles)


# Clusters

In many cases you may have so many Points of Interest (POIs) that these cannot be shown on map as individual markers - map view would be cluttered and eventually also slow or even run out of memory. One of the methods to resolve the issue would be to replace individual objects with *clusters* - Markers which show location of several objects. Often marker clusters shows number of objects which each marker includes.

![Cluster sample](http://share.gifyoutube.com/vMPDzX.gif)

Carto Mobile SDK has built-in cluster feature, which is highly customizable. You can define in you app code:

* style for cluster objects
* generate cluster object style dynamically, so it has e.g. number of objects in it
* define minimum zoom level for clusters
* set minimum distance between objects before it becomes cluster
* decide action for clicking on marker - e.g. zoom in, or expand cluster right away (without zoom). The latter works for small clusters (up to 4 objects inside)


Clusters are generated dynamically, based on VectorDataSource data, during loading of the map view with the layer. From API point of view it works as a special Layer: *ClusteredVectorLayer*, and this creation of it you give two parameters:

1. **DataSource** to be used. In most cases it would be *LocalVectorDataSource* which has already all the elements. It is important the the DataSource gives all elements of layer, not limiting it to current map view bounding box.
2. **ClusterElementBuilder** implementation, which defines single method **buildClusterElement**.

## 1. Define data and layer
<div id="tabs1">
  <ul>
    <li><a href="#i1"><span>iOS ObjC</span></a></li>
    <li><a href="#a1"><span>Android Java</span></a></li>
    <li><a href="#n1"><span>.NET C#</span></a></li>
  </ul>
<div id="i1">
<pre class="brush: objc">
// Initialize a local vector data source
NTProjection* proj = [[self.mapView getOptions] getBaseProjection];
NTLocalVectorDataSource* vectorDataSource = [[NTLocalVectorDataSource alloc] initWithProjection:proj];

// Now create Marker objects and add them to vectorDataSource.
// TODO: this depends on your app! See AdvancedMap for samples with JSON loading and random point generation

// Create element builder
MyMarkerClusterElementBuilder* clusterElementBuilder = [[MyMarkerClusterElementBuilder alloc] init];

// Initialize a vector layer with the previous data source
NTClusteredVectorLayer* vectorLayer = [[NTClusteredVectorLayer alloc] initWithDataSource:vectorDataSource clusterElementBuilder:clusterElementBuilder];

// Add the previous vector layer to the map
[[self.mapView getLayers] add:vectorLayer];

</pre>
</div>
<div id="a1">
<pre class="brush: java">

// Initialize a local vector data source
LocalVectorDataSource vectorDataSource1 = new LocalVectorDataSource(baseProjection);

// Now create Marker objects and add them to vectorDataSource.
// TODO: this depends on your app! See AdvancedMap for samples with JSON loading and random point generation

// Initialize a vector layer with the previous data source
ClusteredVectorLayer vectorLayer1 = new ClusteredVectorLayer(vectorDataSource1, new MyClusterElementBuilder(this.getApplication()));
vectorLayer1.setMinimumClusterDistance(20);

// Add the previous vector layer to the map
mapView.getLayers().add(vectorLayer1);

</pre>
</div>
<div id="n1">
<pre class="brush: csharp">

var proj = new EPSG3857 ();

// Create overlay layer for markers
var dataSource = new LocalVectorDataSource (proj);

// Now create Marker objects and add them to vectorDataSource.
// TODO: this depends on your app! See samples with JSON loading

var overlayLayer = new ClusteredVectorLayer (dataSource, new MyClusterElementBuilder());
overlayLayer.MinimumClusterDistance = 20; // in pixels
mapView.Layers.Add (overlayLayer);

</pre>
</div>
</div>




## 2. Define ClusterElementBuilder

Essentially Cluster Element Builder takes set of original markers (map objects) as input, and returns one Marker (or another VectorElement, like Point or BalloonPopup) which is dynamically replacing the original ones.

What makes it more complicated is that in the ClusterElementBuilder we strongly suggest to reuse (cache) styles to reduce memory usage significantly. So marker style with specific number is only created once. Android and iOS samples use platform-specific graphics APIs to generate bitmap for the marker. .NET example just uses BalloonPopup, which is slower but works same across platforms.

<div id="tabs2">
  <ul>
    <li><a href="#i2"><span>iOS ObjC</span></a></li>
    <li><a href="#a2"><span>Android Java</span></a></li>
    <li><a href="#n2"><span>.NET C#</span></a></li>
  </ul>
<div id="i2">
<pre class="brush: objc">
@implementation MyMarkerClusterElementBuilder

-(NTVectorElement*)buildClusterElement:(NTMapPos *)mapPos elements:(NTVectorElementVector *)elements
{
    if (!self.markerStyles) {
        self.markerStyles = [NSMutableDictionary new];
    }
    NSString* styleKey = [NSString stringWithFormat:@"%d",(int)[elements size]];
    if ([elements size] > 1000) {
        styleKey = @">1K";
    }
    NTMarkerStyle* markerStyle = [self.markerStyles valueForKey:styleKey];
    if ([elements size] == 1) {
        markerStyle = [(NTMarker*)[elements get:0] getStyle];
    }
    if (!markerStyle) {
        UIImage* image = [UIImage imageNamed:@"marker_black.png"];
        UIGraphicsBeginImageContext(image.size);
        [image drawAtPoint:CGPointMake(0, 0)];
        CGRect rect = CGRectMake(0, 15, image.size.width, image.size.height);
        [[UIColor blackColor] set];
        NSMutableParagraphStyle *style = [[NSParagraphStyle defaultParagraphStyle] mutableCopy];
        [style setAlignment:NSTextAlignmentCenter];
        NSDictionary *attr = [NSDictionary dictionaryWithObject:style forKey:NSParagraphStyleAttributeName];
        [styleKey drawInRect:CGRectIntegral(rect) withAttributes:attr];
        UIImage* newImage = UIGraphicsGetImageFromCurrentImageContext();
        UIGraphicsEndImageContext();
        
        NTBitmap* markerBitmap = [NTBitmapUtils createBitmapFromUIImage:newImage];
        NTMarkerStyleBuilder* markerStyleBuilder = [[NTMarkerStyleBuilder alloc] init];
        [markerStyleBuilder setBitmap:markerBitmap];
        [markerStyleBuilder setSize:30];
        //[markerStyleBuilder setHideIfOverlapped:NO];
        [markerStyleBuilder setPlacementPriority:-(int)[elements size]];
        markerStyle = [markerStyleBuilder buildStyle];
        [self.markerStyles setValue:markerStyle forKey:styleKey];
    }
    
    NTMarker* marker = [[NTMarker alloc] initWithPos:mapPos style:markerStyle];
    [marker setMetaDataElement:@"elements" element:[@([elements size]) stringValue]];
    return marker;
}

@end


</pre>
</div>
<div id="a2">
<pre class="brush: java">

private static class MyClusterElementBuilder extends ClusterElementBuilder {
		@SuppressLint("UseSparseArrays")
		private Map < Integer, MarkerStyle >  markerStyles = new HashMap < Integer, MarkerStyle > ();
		private android.graphics.Bitmap markerBitmap;
		
		MyClusterElementBuilder(Application context) {
			markerBitmap = android.graphics.Bitmap.createBitmap(BitmapFactory.decodeResource(context.getResources(), R.drawable.marker_black));
		}

		@Override
		public VectorElement buildClusterElement(MapPos pos, VectorElementVector elements) {
			// Try to reuse existing marker styles
			MarkerStyle style = markerStyles.get((int) elements.size());
			if (elements.size() == 1) {
				style = ((Marker) elements.get(0)).getStyle();
			}
			if (style == null) {
				android.graphics.Bitmap canvasBitmap = markerBitmap.copy(android.graphics.Bitmap.Config.ARGB_8888, true);
				android.graphics.Canvas canvas = new android.graphics.Canvas(canvasBitmap); 
				android.graphics.Paint paint = new android.graphics.Paint(android.graphics.Paint.ANTI_ALIAS_FLAG);
				paint.setTextAlign(Align.CENTER);
				paint.setTextSize(12);
				paint.setColor(android.graphics.Color.argb(255, 0, 0, 0));
				canvas.drawText(Integer.toString((int) elements.size()), markerBitmap.getWidth() / 2, markerBitmap.getHeight() / 2 - 5, paint);
				MarkerStyleBuilder styleBuilder = new MarkerStyleBuilder();
				styleBuilder.setBitmap(BitmapUtils.createBitmapFromAndroidBitmap(canvasBitmap));
				styleBuilder.setSize(30);
		        styleBuilder.setPlacementPriority((int)-elements.size());
				style = styleBuilder.buildStyle();
				markerStyles.put((int) elements.size(), style);
			}

			// Create marker for the cluster
			Marker marker = new Marker(pos, style);
			return marker;
		}
	}
    

</pre>
</div>
<div id="n2">
<pre class="brush: csharp">

class MyClusterElementBuilder : ClusterElementBuilder
	{
		BalloonPopupStyleBuilder balloonPopupStyleBuilder;

		public MyClusterElementBuilder() {
			balloonPopupStyleBuilder = new BalloonPopupStyleBuilder();
			balloonPopupStyleBuilder.CornerRadius = 3;
			balloonPopupStyleBuilder.TitleMargins = new BalloonPopupMargins (6, 6, 6, 6);
			balloonPopupStyleBuilder.LeftColor = new Color(240,230,140,255);
		}

		public override VectorElement BuildClusterElement(MapPos pos, VectorElementVector elements) {
			var popup = new BalloonPopup(
				pos,
				balloonPopupStyleBuilder.BuildStyle(),
				elements.Count.ToString(), "");
			return popup;
		}

	}

</pre>
</div>
</div>


# MBTiles map data
[MBTiles](http://mapbox.com/developers/mbtiles/) support is included in Carto Mobile SDK. MBTiles uses SQLite database format, so you have just one file to be downloaded and managed. Following are some tools to create MBTiles packages:

## a) Raster maps

* [MapTiler](http://www.maptiler.com/) is nice utility to create MBTiles from raster geo files (GeoTIFF, JPG, ECW etc), it is well worth small price to support the developer
* [TileMill](http://mapbox.com/tilemill/) is open source generator of very nice map packages, if source is vector geo file, e.g. Shapefile or PosgGIS geo data.
* [MOBAC](http://mobac.sourceforge.net) - download from variety of free sources (e.g. Bing, OpenStreetMap etc), can even load from WMS with added configuration
* [MBUtil](https://github.com/mapbox/mbutil) enables to create mbtiles from/to TMS-style tile folders, created with e.g. GDAL utility
* [Portable Basemap Server](https://geopbs.codeplex.com/) - free utility for Windows, loads data not only from various commercial servers and custom sources, but also from ESRI formats. Works mainly as WMTS server, but can create MBTiles as extra feature.

## b) Vector maps

* Nutiteq SDK bundled [Package Manager API](/guides/offline-maps) downloads country-wide or bounding box based map packages with **OpenStreetMap data**. These files are not directly visible, as Package Manager downlods the files, you only use the API.
* If you need to create vector map files **from different sources or your own data** (e.g. Shapefiles), then please contact Carto support - we can provide you converter toolchain, and do free demo conversions.

# Ground Overlays

Carto Mobile SDK since 3.1 supports *Ground Overlays* - bitmaps (PNG, JPG etc) which are put to map to your defined location. Most commonly this is used for **indoor floorplans**, but it can be used also for other cases.

<img src = "/images/ground_overlay.png" alt = "Sample bitmap" width="550">

Your bitmap must be have defined **geographical map coordinates**, and this is done using *Ground Control Points*. Currently we support exactly 3 or 4 points defined on bitmap and real world, with following rules:

* With 3 control points you set location, size and rotation of bitmap. This does *linear affine transformation*.
* With 4 control points you can have *perspective transformation*.
* If you have more control points in your data, then you should select 3/4 best ones, and use these.
* Control points must be defined in app code. SDK does not read this automatically from source file metadata. So if you need GeoTIFF, ESRI World File, MapInfo TAB, Geospatial PDF or another already referenced data, then you will get this from GIS Extension (see below).

Key limitation with Carto standard SDK package is that **whole bitmap must fit to device memory** (RAM), so maximum size could be something like 2000x2000 pixels or so, depending on target device. For larger rasters you can use **Carto Mobile SDK GIS Extension** which allows to show any size bitmaps, even hundreds of megabytes, and these are read directly from common GIS raster formats, e.g. GeoTIFF, BSB, ECW, MrSID, JPEG2000 etc, and source data can be in different coordinate systems. Please ask from sales@nutiteq.com if you need this.

Following sample assumes that you have bitmap file **jefferson-building-ground-floor.jpg** in your application project: under *assets* in Android, anywhere in project in iOS. Here we use only one geographical coordinate, and we know size of the building and that it is exactly heading to north, so we can calculate other ground points with the code. Now four ground control points are set to the corners of the bitmap, which gives usually most accurate result. 

<div id="tabs1">
  <ul>
    <li><a href="#i1"><span>iOS ObjC</span></a></li>
    <li><a href="#a1"><span>Android Java</span></a></li>
    <li><a href="#n1"><span>.NET C#</span></a></li>
  </ul>
<div id="i1">
<pre class="brush: objc">
// Load ground overlay bitmap
NTBitmap* overlayBitmap = [[NTBitmap alloc] initWithCompressedData:[NTAssetUtils loadBytes:@"jefferson-building-ground-floor.jpg"] pow2Padding:NO];

// Create two vector containing geographical positions and corresponding raster image pixel coordinates
NTMapPos* pos = [proj fromWgs84:[[NTMapPos alloc] initWithX:-77.004590 y:38.888702]];
double sizeNS = 110, sizeWE = 100;

NTMapPosVector* mapPoses = [[NTMapPosVector alloc] init];
[mapPoses add:[[NTMapPos alloc] initWithX:[pos getX]-sizeWE y:[pos getY]+sizeNS]];
[mapPoses add:[[NTMapPos alloc] initWithX:[pos getX]+sizeWE y:[pos getY]+sizeNS]];
[mapPoses add:[[NTMapPos alloc] initWithX:[pos getX]+sizeWE y:[pos getY]-sizeNS]];
[mapPoses add:[[NTMapPos alloc] initWithX:[pos getX]-sizeWE y:[pos getY]-sizeNS]];

NTScreenPosVector* bitmapPoses = [[NTScreenPosVector alloc] init];
[bitmapPoses add:[[NTScreenPos alloc] initWithX:0 y:0]];
[bitmapPoses add:[[NTScreenPos alloc] initWithX:0 y:[overlayBitmap getOrigHeight]]];
[bitmapPoses add:[[NTScreenPos alloc] initWithX:[overlayBitmap getOrigWidth] y:[overlayBitmap getOrigHeight]]];
[bitmapPoses add:[[NTScreenPos alloc] initWithX:[overlayBitmap getOrigWidth] y:0]];

// Create bitmap overlay raster tile data source
NTBitmapOverlayRasterTileDataSource* rasterDataSource = [[NTBitmapOverlayRasterTileDataSource alloc] initWithMinZoom:0 maxZoom:20 bitmap:overlayBitmap projection:proj mapPoses:mapPoses bitmapPoses:bitmapPoses];
NTRasterTileLayer* rasterLayer = [[NTRasterTileLayer alloc] initWithDataSource:rasterDataSource];
[[self.mapView getLayers] add:rasterLayer];

// Apply zoom level bias to the raster layer.
// By default, bitmaps are upsampled on high-DPI screens.
// We will correct this by applying appropriate bias
float zoomLevelBias = log([[self.mapView getOptions] getDPI] / 160.0f) / log(2);
[rasterLayer setZoomLevelBias:zoomLevelBias * 0.75f];
[rasterLayer setTileSubstitutionPolicy:NT_TILE_SUBSTITUTION_POLICY_VISIBLE];

[self.mapView setFocusPos:pos durationSeconds:0];
[self.mapView setZoom:15.5f durationSeconds:0];
</pre>
</div>
<div id="a1">
<pre class="brush: java">
Projection proj = mapView.getOptions().getBaseProjection();

// Load ground overlay bitmap
com.nutiteq.graphics.Bitmap overlayBitmap = new com.nutiteq.graphics.Bitmap(AssetUtils.loadBytes("jefferson-building-ground-floor.jpg"), false);

// Create two vector containing geographical positions and corresponding raster image pixel coordinates
MapPos pos = proj.fromWgs84(new MapPos(-77.004590, 38.888702));
double sizeNS = 110, sizeWE = 100;

MapPosVector mapPoses = new MapPosVector();
mapPoses.add(new MapPos(pos.getX()-sizeWE, pos.getY()+sizeNS));
mapPoses.add(new MapPos(pos.getX()+sizeWE, pos.getY()+sizeNS));
mapPoses.add(new MapPos(pos.getX()+sizeWE, pos.getY()-sizeNS));
mapPoses.add(new MapPos(pos.getX()-sizeWE, pos.getY()-sizeNS));

ScreenPosVector bitmapPoses = new ScreenPosVector();
bitmapPoses.add(new ScreenPos(0, 0));
bitmapPoses.add(new ScreenPos(0, overlayBitmap.getOrigHeight()));
bitmapPoses.add(new ScreenPos(overlayBitmap.getOrigWidth(), overlayBitmap.getOrigHeight()));
bitmapPoses.add(new ScreenPos(overlayBitmap.getOrigWidth(), 0));

// Create bitmap overlay raster tile data source
BitmapOverlayRasterTileDataSource rasterDataSource = new BitmapOverlayRasterTileDataSource(0, 20, overlayBitmap, proj, mapPoses, bitmapPoses);
RasterTileLayer rasterLayer = new RasterTileLayer(rasterDataSource);
mapView.getLayers().add(rasterLayer);

// Apply zoom level bias to the raster layer.
// By default, bitmaps are upsampled on high-DPI screens.
// We will correct this by applying appropriate bias
float zoomLevelBias = (float) (Math.log(mapView.getOptions().getDPI() / 160.0f) / Math.log(2));
rasterLayer.setZoomLevelBias(zoomLevelBias * 0.75f);
rasterLayer.setTileSubstitutionPolicy(TileSubstitutionPolicy.TILE_SUBSTITUTION_POLICY_VISIBLE);

mapView.setFocusPos(pos, 0);
mapView.setZoom(15.5f, 0);
</pre>
</div>
<div id="n1">
<pre class="brush: csharp">
// Load ground overlay bitmap
var overlayBitmap = new Bitmap (AssetUtils.LoadBytes("jefferson-building-ground-floor.jpg"), false);

// Create two vector containing geographical positions and corresponding raster image pixel coordinates
var pos = proj.FromWgs84(new MapPos(-77.004590, 38.888702));
var sizeNS = 110;
var sizeWE = 100;

var mapPoses = new MapPosVector();
mapPoses.Add(new MapPos(pos.X-sizeWE, pos.Y+sizeNS));
mapPoses.Add(new MapPos(pos.X+sizeWE, pos.Y+sizeNS));
mapPoses.Add(new MapPos(pos.X+sizeWE, pos.Y-sizeNS));
mapPoses.Add(new MapPos(pos.X-sizeWE, pos.Y-sizeNS));

var bitmapPoses = new ScreenPosVector();
bitmapPoses.Add(new ScreenPos(0, 0));
bitmapPoses.Add(new ScreenPos(0, overlayBitmap.OrigHeight));
bitmapPoses.Add(new ScreenPos(overlayBitmap.OrigWidth, overlayBitmap.OrigHeight));
bitmapPoses.Add(new ScreenPos(overlayBitmap.OrigWidth, 0));

// Create bitmap overlay raster tile data source
var rasterDataSource = new BitmapOverlayRasterTileDataSource(0, 20, overlayBitmap, proj, mapPoses, bitmapPoses);
var rasterLayer = new RasterTileLayer(rasterDataSource);
mapView.Layers.Add(rasterLayer);

// Apply zoom level bias to the raster layer.
// By default, bitmaps are upsampled on high-DPI screens.
// We will correct this by applying appropriate bias
float zoomLevelBias = (float) (Math.Log(mapView.Options.DPI / 160.0f) / Math.Log(2));
rasterLayer.ZoomLevelBias = zoomLevelBias * 0.75f;
rasterLayer.TileSubstitutionPolicy = TileSubstitutionPolicy.TileSubstitutionPolicyVisible;

mapView.SetFocusPos(pos, 0);
mapView.SetZoom(15.5f, 0);

</pre>
</div>
</div>

# Vector Styles

Usage of vector-based base map enables you to re-style map according to your needs: set colors, transparency, line styles (width, patterns, casings, endings), polygon patterns, icons, text placements, fonts and many other vector data parameters. Carto uses Mapnik (http://mapnik.org) XML style description language for customizing the visual style of vector tiles. Our styling is optimized for mobile, so we add some style parameters, and ignore some others, but generally you can mostly reuse your existing Mapnik XML or CartoCSS styling files and tools (like TileMill/Mapbox Studio).

Vector styling is applied in mobile client side, and the style files are usually bundled with application installer. Application can change the styling anytime without reloading vector map data, so you can download map data once, and change styling from "day mode" to "night mode" with no new downloads.

Carto map rendering implementation is intended for real-time rendering and, as a result, several limitations apply.

### Ready-made styles
See [Mobile Styles](/mobileStyles) page for ready-made styles by Carto. These are compatible with our vector tile sources.

### Mapnik style format
Mapnik map style definition is a common file format for map styles, it is based on XML. It is originally done for Mapnik, but is used by other softwares like our Mobile SDK. File format specification is in [XMLConfigReference](https://github.com/mapnik/mapnik/wiki/XMLConfigReference) document. There are several ways you can use the styles, from simplest to most advanced:

 1. Use Carto provided styles as they are
 1. Modify style.xml inside sample style to tweak it
 1. Create own style using some tool which edits Mapnik styles. Probably currently the best tool is free MapBox Studio, which uses CartoCSS as primary style definition. We don't use CartoCSS in Carto SDK, but Studio but can export also Mapnik XML styles. However, these style files needs a bit modification to be compatible with Carto SDK. 

### Creating style package

Carto vector styles are distributed as zip-archives. All style-related files/folders must be placed into a single zip file.
The most important part of the style is a style definition file, usually named _project.xml_. This file contains style descriptions of all layers and it usually references other files, like fonts, icons, pattern bitmaps which should be placed in various subfolders.

### Limitations/incompatibilities

* There are no built-in fonts, fonts must be explicitly added to the project

* Loading SVG icons is not supported, such icons should be converted to PNG format (Mapnik provides _svg2png_ utility)

* Original layer ordering is not always preserved, texts are always drawn on top of 2d geometry, 3d buildings are drawn on top of texts

* Layer opacity works per element, not per layer as in Mapnik. For non-overlapping elements, the result will be same. For overlapping elements there are likely artifacts.

* _comp-op_ feature is not supported (neither layer/symbolizer). Per symbolizer _comp-op_ support is under investigation

* _line-join_ parameter is ignored, only _miter_ line join is used. 

* _GroupSymbolizer_ and _RasterSymbolizer_ are not supported, support is under investigation

* Text characters are rendered one by one. If characters overlap, halo of one character may cover glyph of another character. Workaround is to increase spacing or decrease halo radius.

* This list is not final. Mapnik XML is not official standard, and mostly due to performance penalty on the mobile Carto SDK does not implement 100% of the tags and features of Mapnik. If you need some of the not implemented styling options, please contact our support. 

### Performance hints 

* **Multiple symbolizers per layer may have very large performance hit**. If possible, move each symbolizer into separate layer.

* _BuildingSymbolizer_ requires expensive OpenGL frame buffer read-back operation and may perform very poorly on some devices (original iPad Retina)

* It is best to keep all bitmaps with power-of-two dimensions, this increases performance.

### Carto-specific extension to Mapnik XML style files

#### _NutiParameters_

_NutiParameters_ describe additional parameters that can be used in styles and controlled in the code (from _MBVectorTileDecoder_).
Parameters are typed, have default values and can be used as variables within _nuti_ namespace in the style (for example, *[nuti::lang]*).
Some parameters may have _ prefix in their name. Such variables are reserved and should not be updated directly by the application.

Simple example of _NutiParameters_ section in style xml file:

`<NutiParameters>`
`  <NutiParameter name="lang" type="string" value="en" />`
`</NutiParameters>`

#### Metavariables

Metavariables add support for dynamic variable look up. Variable names may depend on other variables. For example, *[name_[nuti::lang]]*.
If the value of *nuti::lang* is 'en', this metavariable expression would be equal to *[name_en]* expression.

#### Conditional operator

Conditional operator ?: adds support for simple control flow management. It is similar to C language conditional operator and can be used in all 
expressions. For example:

`[nuti::lang] == 'en' ? 'English' : 'Other'`

#### 3D texts and markers

Mapnik _MarkersSymbolizer_ and _TextSymbolizer_ support additional value _nutibillboard_ for _placement_ parameter. This will make texts and
markers act as billboards (always facing the viewer) even when screen is tilted. This option can be used to give markers and texts more
'3d-like' look. For example,

`<MarkersSymbolizer placement="nutibillboard" fill="#666666" file="icon/[maki]-12.svg" />`

### Supported symbolizers and parameters

The following list contains all supported symbolizers and parameters, as of version 4.0.0:

* PointSymbolizer: file, opacity, allow-overlap, ignore-placement, transform

* BuildingSymbolizer: fill, fill-opacity, height, geometry-transform

* LineSymbolizer: stroke, stroke-width, stroke-opacity, stroke-linejoin, stroke-linecap, stroke-dasharray, geometry-transform

* LinePatternSymbolizer: file, fill, opacity, geometry-transform

* PolygonSymbolizer: fill, fill-opacity, geometry-transform

* PolygonPatternSymbolizer: file, fill, opacity, geometry-transform

* MarkersSymbolizer: file, placement, marker-type, fill, opacity, width, height, spacing, allow-overlap, ignore-placement, transform

* TextSymbolizer: name, face-name, fontset-name, placement, size, spacing, fill, opacity, halo-fill, halo-opacity, halo-radius, allow-overlap, minimum-distance, text-transform, orientation, dx, dy, wrap-width, wrap-before, character-spacing, line-spacing, horizontal-alignment, vertical-alignment

* ShieldSymbolizer: name, face-name, fontset-name, placement, size, spacing, fill, opacity, halo-fill, halo-opacity, halo-radius, allow-overlap, minimum-distance, text-transform, orientation, dx, dy, wrap-width, wrap-before, character-spacing, line-spacing, horizontal-alignment, vertical-alignment, file, shield-dx, shield-dy, unlock-image

### CartoCSS extensions

Same extensions: metavariables and nutiparameters are available for CartoCSS styles.

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

