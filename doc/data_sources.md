Data sources
============

Following DataSources are available for Nutiteq SDK 3.x on all supported
platforms (iOS, Android and others).

Some DataSources are more universal and could be used for different data
types and Layers: vector or raster, others can be used for specific
layer type only.

### 1. Offline maps from MBTiles {#offline-maps}

**MBTilesTileDataSource** is universal datasource for vector or raster
data, so it can be used two ways.

**a) MBtiles as RasterTileLayer**

For raster data it needs
[MBTiles](https://github.com/mapbox/mbtiles-spec) format files, see
[MBTiles guide](/guides/mbtiles) with list of some tools and sources to
generate them.

<div id="tabs1">
<ul>
<li>
<a href="#i1"><span>iOS ObjC</span></a></li>

<li>
<a href="#a1"><span>Android Java</span></a></li>

<li>
<a href="#n1"><span>.NET C\#</span></a></li>

</ul>
<div id="i1">
``` {.brush: .objc}
   // file-based local offline datasource
   NSString* fullpathVT = [[NSBundle mainBundle] pathForResource:@"MBTILESFILENAME" ofType:@"mbtiles"];
   NTTileDataSource* tileDataSource = [[NTMBTilesTileDataSource alloc] initWithPath: fullpathVT];

   // Initialize a raster layer with the previous data source
   NTRasterTileLayer* rasterLayer = [[NTRasterTileLayer alloc] initWithDataSource:tileDataSource];

   // Add the raster layer to the map
   [[mapView getLayers] add:rasterLayer];
```

</div>
<div id="a1">
``` {.brush: .java}

  // Create tile data source.
  MBTilesTileDataSource tileDataSource = new MBTilesTileDataSource(filePath);

  // Create layer and add to map
  TileLayer rasterLayer = new RasterTileLayer(tileDataSource);

  mapView.getLayers().add(rasterLayer);
```

</div>
<div id="n1">
``` {.brush: .csharp}

// NB!: the path to file is platfrom-specific.

var mbTilesLayer = new RasterTileLayer (new MBTilesTileDataSource (filePath));
mapView.Layers.Add (mbTilesLayer);

```

</div>
</div>
**b) MBtiles as VectorTileLayer**

For own packaged vector data you need Nutiteq specific vector files
packages (NTVT - *NutiTeq Vector Tile*) and styling files in Mapnik XML
format. Here is free sample packages with OpenStreetMap data:

-   [estonia\_ntvt.mbtiles](https://dl.dropboxusercontent.com/u/3573333/public_web/ntvt_packages/estonia_ntvt.mbtiles)

Nutiteq SDK provides built-in download service called **Package
Manager** to get the map packages for a country or smaller region
easily, this is described in separate page: [learn
more](/guides/offline-maps) . Sample here is for case when you have
**own generated MBTiles file with vector data**.

Vector maps needs always proper **style definition**. You can find
compatible stylesheet files from sample code, these are zip files which
have **Mapnik XML styling** with resources (fonts, images) inside. You
can use ones with sample projects, e.g. OSMBright with 3D buildings
[osmbright.zip](https://github.com/nutiteq/hellomap3d-android/blob/master/com.nutiteq.advancedmap3/assets/osmbright.zip).
See [Vector Tile Styling](/guides/vector-styles) for details about
styling.

<div id="tabs2">
<ul>
<li>
<a href="#i2"><span>iOS ObjC</span></a></li>

<li>
<a href="#a2"><span>Android Java</span></a></li>

<li>
<a href="#n2"><span>.NET C\#</span></a></li>

</ul>
<div id="i2">
``` {.brush: .objc}
  // 1. define file-based local offline datasource
  NSString* fullpathVT = [[NSBundle mainBundle] pathForResource:@"estonia_ntvt" ofType:@"mbtiles"];
  NTTileDataSource* tileDataSource = [[NTMBTilesTileDataSource alloc] initWithMinZoom:0 maxZoom:4 path: fullpathVT];

  // 2. Load vector tile styleset
  UnsignedCharVector *vectorTileStyleSetData = [NTAssetUtils loadBytes: @"osmbright.zip"];
  NTMBVectorTileStyleSet *vectorTileStyleSet = [[NTMBVectorTileStyleSet alloc] initWithData:vectorTileStyleSetData];

  // 3. Create vector tile decoder using the styleset
  NTMBVectorTileDecoder *vectorTileDecoder = [[NTMBVectorTileDecoder alloc] initWithStyleSet:vectorTileStyleSet];

  // 4. Create vector tile layer, using previously created data source and decoder
  NTVectorTileLayer *vectorTileLayer = [[NTVectorTileLayer alloc] initWithDataSource:vectorTileDataSource decoder:vectorTileDecoder];

  // 5. Add vector tile layer
  [[mapView getLayers] add:vectorTileLayer];
```

</div>
<div id="a2">
``` {.brush: .java}
  // 1. Create tile data source from mbtiles file
  MBTilesTileDataSource tileDataSource = new MBTilesTileDataSource("/sdcard/estonia_ntvt.mbtiles");

  // 2. Load vector tile styleset
  UnsignedCharVector styleBytes = AssetUtils.loadBytes("osmbright.zip");
  MBVectorTileStyleSet vectorTileStyleSet = new MBVectorTileStyleSet(styleBytes);

  // 3. Create vector tile decoder using the styleset
  VectorTileDecoder vectorTileDecoder = new MBVectorTileDecoder(vectorTileStyleSet);

  // 4. Create vector tile layer, using previously created data source and decoder
  TileLayer vectorTileLayer = new VectorTileLayer(tileDataSource, vectorTileDecoder);

  // 5. Add vector tile layer
  mapView.getLayers().add(vectorTileLayer);
```

</div>
<div id="n2">
``` {.brush: .csharp}
// 1. Create tile data source from mbtiles file
var tileDataSource = new MBTilesTileDataSource("/sdcard/mapxt/estonia_ntvt.mbtiles");

// 2. Load vector tile styleset
var styleBytes = AssetUtils.LoadBytes("osmbright.zip");
var vectorTileStyleSet = new MBVectorTileStyleSet(styleBytes);

// 3. Create vector tile decoder using the styleset
var vectorTileDecoder = new MBVectorTileDecoder(vectorTileStyleSet);

// 4. Create vector tile layer, using previously created data source and decoder
var vectorTileLayer = new VectorTileLayer(tileDataSource, vectorTileDecoder);

// 5. Add vector tile layer
mapView.Layers.Add(vectorTileLayer);
```

</div>
</div>
### 2. Online maps

**HTTPTileDataSource** is also universal datasource for vector or raster
data.

**a) Nutiteq online Vector Tile Layer**

Nutiteq SDK provides support for built-in vector tiles, as provided by
Nutiteq Maps as a Service packages and it is available for all plans.
This is suitable as universial base map.

For vector styling you use exactly same osmbright.zip file as for
offline tiles, see above.

<div id="tabs3">
<ul>
<li>
<a href="#i3"><span>iOS ObjC</span></a></li>

<li>
<a href="#a3"><span>Android Java</span></a></li>

<li>
<a href="#n3"><span>.NET C\#</span></a></li>

</ul>
<div id="i3">
``` {.brush: .objc}
NTVectorTileLayer* vectorTileLayer = [[NTNutiteqOnlineVectorTileLayer alloc] initWithStyleAssetName:@"osmbright.zip"];
[[mapView getLayers] add:vectorTileLayer];
```

</div>
<div id="a3">
``` {.brush: .java}
// Create base layer. Use vector style from assets (osmbright.zip)
VectorTileLayer baseLayer = new NutiteqOnlineVectorTileLayer("osmbright.zip");
mapView.getLayers().add(baseLayer);
```

</div>
<div id="n3">
``` {.brush: .csharp}
// Create base layer. Use vector style from assets (osmbright.zip)
var baseLayer = new NutiteqOnlineVectorTileLayer("osmbright.zip");
mapView.Layers.Add(baseLayer);
```

</div>
</div>
**b) Web (HTTP) tiles as VectorTileLayer**

With Nutiteq Starter, Pro or Enterprise plans you can use also other
vector tile map data sources. It is similar to custom raster map data
sources, only adding vector decoder is needed.

Here we connect to MapBox vector tiles, which have very similar (but not
identical) data structure, so we can use same osmbright.zip vector
styling. Before using this in your app please check also from MapBox if
they allow this, and make sure you use your MapBox key in URL. You may
need to use your own vector tile server URL, and your suitable styling
instead.

Following tags are supported in URL definition: **zoom, x, y, xflipped,
yflipped, quadkey**.

<div id="tabs4">
<ul>
<li>
<a href="#i4"><span>iOS ObjC</span></a></li>

<li>
<a href="#a4"><span>Android Java</span></a></li>

<li>
<a href="#n4"><span>.NET C\#</span></a></li>

</ul>
<div id="i4">
``` {.brush: .objc}

// 1. define data source
NTTileDataSource* tileDataSource = [[NTHTTPTileDataSource alloc] initWithMinZoom:0 maxZoom:14 baseURL:@"http://a.tiles.mapbox.com/v4/mapbox.mapbox-streets-v5/{zoom}/{x}/{y}.vector.pbf?access_token=YOUR-MAPBOX-TOKEN"];

// 2. Load vector tile styleset
NTUnsignedCharVector *vectorTileStyleSetData = [NTAssetUtils loadBytes: @"osmbright.zip"];
NTMBVectorTileStyleSet *vectorTileStyleSet = [[NTMBVectorTileStyleSet alloc] initWithData:vectorTileStyleSetData];

// 3. Create vector tile decoder using the styleset
NTMBVectorTileDecoder *vectorTileDecoder = [[NTMBVectorTileDecoder alloc] initWithStyleSet:vectorTileStyleSet];

// 4. Create vector tile layer, using previously created data source and decoder
NTVectorTileLayer *vectorTileLayer = [[NTVectorTileLayer alloc] initWithDataSource:tileDataSource decoder:vectorTileDecoder];

// 5. Add vector tile layer
[[mapView getLayers] add:vectorTileLayer];
```

</div>
<div id="a4">
``` {.brush: .java}
// 1. define data source
TileDataSource vectorTileDataSource = new HTTPTileDataSource(0, 14, "http://a.tiles.mapbox.com/v4/mapbox.mapbox-streets-v5/{zoom}/{x}/{y}.vector.pbf?access_token=YOUR-MAPBOX-TOKEN");

// 2. Create style set
MBVectorTileStyleSet vectorTileStyleSet = new MBVectorTileStyleSet(styleBytes);
MBVectorTileDecoder vectorTileDecoder = new MBVectorTileDecoder(vectorTileStyleSet);

// 3. Create layer and add to map

TileLayer baseLayer = new VectorTileLayer(vectorTileDataSource, vectorTileDecoder);
mapView.getLayers().add(baseLayer);
```

</div>
<div id="n4">
``` {.brush: .csharp}
// 1. define data source
var vectorTileDataSource = new HTTPTileDataSource(0, 14, "http://a.tiles.mapbox.com/v4/mapbox.mapbox-streets-v5/{zoom}/{x}/{y}.vector.pbf?access_token=YOUR-MAPBOX-TOKEN");

// 2. Create style set
var styleBytes = AssetUtils.LoadBytes("osmbright.zip");
var vectorTileStyleSet = new MBVectorTileStyleSet(styleBytes);
var vectorTileDecoder = new MBVectorTileDecoder(vectorTileStyleSet);

// 3. Create layer and add to map

var baseLayer = new VectorTileLayer(vectorTileDataSource, vectorTileDecoder);
mapView.Layers.Add(baseLayer);
```

</div>
</div>
**b) Web (HTTP) tiles as RasterTileLayer**

For online raster tiles you can use any common web tiles in PNG or JPG
formats, in Spherical Mercator system. See our [Raster tile sources
list](/guides/raster-tile-sources) .

Following tags are supported in URL definition: **zoom, x, y, xflipped,
yflipped, quadkey**.

<div id="tabs5">
<ul>
<li>
<a href="#i5"><span>iOS ObjC</span></a></li>

<li>
<a href="#a5"><span>Android Java</span></a></li>

<li>
<a href="#n5"><span>.NET C\#</span></a></li>

</ul>
<div id="i5">
``` {.brush: .objc}
// Initialize a OSM raster data source from MapQuest Open Tiles
NTHTTPTileDataSource* baseRasterTileDataSource = [[NTHTTPTileDataSource alloc] initWithMinZoom:0 maxZoom:19 baseURL:@"http://otile1.mqcdn.com/tiles/1.0.0/osm/{zoom}/{x}/{y}.png"];

// Initialize a raster layer with the previous data source
NTRasterTileLayer* rasterLayer = [[NTRasterTileLayer alloc] initWithDataSource:baseRasterTileDataSource];

// Add the previous raster layer to the map
[[mapView getLayers] add:rasterLayer];
```

</div>
<div id="a5">
``` {.brush: .java}

// Create a Bing raster data source. Note: tiles start from level 1, there is no single root tile!
TileDataSource baseRasterTileDataSource = new HTTPTileDataSource(1, 19, "http://ecn.t3.tiles.virtualearth.net/tiles/a{quadkey}.jpeg?g=471&mkt=en-US");

TileLayer baseLayer = new RasterTileLayer(baseRasterTileDataSource);

mapView.getLayers().add(baseLayer);
```

</div>
<div id="n5">
``` {.brush: .csharp}

// Create a Bing raster data source. Note: tiles start from level 1, there is no single root tile!
var baseRasterTileDataSource = new HTTPTileDataSource(1, 19, "http://ecn.t3.tiles.virtualearth.net/tiles/a{quadkey}.jpeg?g=471&mkt=en-US");

// create layer and add to map
var baseLayer = new RasterTileLayer(baseRasterTileDataSource);
mapView.Layers.Add(baseLayer);
```

</div>
</div>
### 3. Application-defined vector overlay {#application-defined-vector-data}

Nutiteq SDK has in-memory datasources where application can add vector
objects which are shown on map as overlay layer.

**LocalVectorDataSource** is a dynamic data source that is empty
initially and supports adding/removing objects. Objects are kept in
local memory and the state is not persistent. By default, all added
objects are always rendered. For detailed code samples see [map data
overlay](/guides/overlays) page, this has samples how to add Points,
Markers, Texts, Lines and Polygons to map.

### 4. Application-defined ground overlays {#ground-overlay}

Nutiteq SDK supports **Ground Overlays** - bitmaps (PNG, JPG etc) which
are put to map to your defined location. Most commonly this is used for
**indoor floorplans**, but it can be used also for other cases.

**BitmapOverlayRasterTileDataSource** defines bitmap image, and
geographical coordinates of the bitmap. See [Ground Overlay
guide](/guides/ground-overlays) for more info.

### 5. Virtual data sources

There are some DataSources which take another DataSource as input, do
internal logic or processing and output DataSource itself. This way you
can pipe datasources to have customized logic.

-   **PersistentCacheTileDataSource**

Caches HTTP tiles to a persistent sqlite database file. If tile exists
in the database, then request to original data source is skipped. Can be
used for both raster and vector tiles. The data source takes original
source’s expires headers into account.

<div id="tabs6">
<ul>
<li>
<a href="#i6"><span>iOS ObjC</span></a></li>

<li>
<a href="#a6"><span>Android Java</span></a></li>

<li>
<a href="#n6"><span>.NET C\#</span></a></li>

</ul>
<div id="i6">
``` {.brush: .objc}
// Initialize a OSM raster data source from MapQuest Open Tiles
NTHTTPTileDataSource* baseRasterTileDataSource = [[NTHTTPTileDataSource alloc] initWithMinZoom:0 maxZoom:19 baseURL:@"http://otile1.mqcdn.com/tiles/1.0.0/osm/{zoom}/{x}/{y}.png"];

// Create persistent cache for the given data source  
NTPersistentCacheTileDataSource* cachedRasterTileDataSource = [[NTPersistentCacheTileDataSource alloc] initWithDataSource:baseRasterTileDataSource databasePath:[NTAssetUtils calculateWritablePath:@"mycache.db"]];

// Initialize a raster layer with the previous data source
NTRasterTileLayer* rasterLayer = [[NTRasterTileLayer alloc] initWithDataSource:cachedRasterTileDataSource];

// Add the previous raster layer to the map
[[self.mapView getLayers] add:rasterLayer];
```

</div>
<div id="a6">
``` {.brush: .java}

// Create a Bing raster data source. Note: tiles start from level 1, there is no single root tile!
TileDataSource baseRasterTileDataSource = new HTTPTileDataSource(1, 19, "http://ecn.t3.tiles.virtualearth.net/tiles/a{quadkey}.jpeg?g=471&mkt=en-US");

// Add persistent caching datasource, tiles will be stored locally on persistent storage
PersistentCacheTileDataSource cachedDataSource = 
  new PersistentCacheTileDataSource(baseRasterTileDataSource, getExternalFilesDir(null)+"/mapcache.db");

// Create layer and add to map
TileLayer baseLayer = new RasterTileLayer(cachedDataSource);
mapView.getLayers().add(baseLayer);
```

</div>
<div id="n6">
``` {.brush: .csharp}

// Create a Bing raster data source. Note: tiles start from level 1, there is no single root tile!
var baseRasterTileDataSource = new HTTPTileDataSource(1, 19, "http://ecn.t3.tiles.virtualearth.net/tiles/a{quadkey}.jpeg?g=471&mkt=en-US");

// Add persistent caching datasource, tiles will be stored locally on persistent storage
// fileDir must be a directory where files can be written - this is platform-specific
var cachedDataSource = 
  new PersistentCacheTileDataSource(baseRasterTileDataSource, fileDir+"/mapcache.db");


// create layer and add to map
var baseLayer = new RasterTileLayer(cachedDataSource);
mapView.Layers.Add(baseLayer);
```

</div>
</div>
-   **CompressedCacheTileDataSource**

An in-memory cache data source. Though layers also cache tiles, the
tiles are uncompressed and usually take 2-5x more memory after being
loaded. CompressedCacheTileDataSource keeps all tiles in compressed
format and thus makes better use of existing memory.

-   \***CombinedTileDataSource**

A tile data source that combines two data sources (usually offline and
online) and selects tiles based on zoom level. All requests below
specified zoom level are directed to the first data source and all
requests at or above specified zoom level are directed to the second
data source.

### 6. Other built-in data sources

-   **AssetTileDataSource** - loads offline tiles data from a folder
    (/sdcard or Application sandbox, depends on platform). You can use
    for example zoom/x/y.png or zoom\_x\_y.png file naming structures
    for offline data.
-   **NMLModelLODTreeOnlineDataSource** - uses NMLDB online API for “3D
    city” layer. 3D tools are provided as commercial service, contact
    Nutiteq to get more info about it.
-   **NMLModelLODTreeSqliteDataSource** - An offline version of
    *NMLModelLODTreeOnlineDataSource*, model info is kept in sqlite
    database file. Note that simple individual 3D objects (NML models)
    can be added to usual *LocalVectorDataSource*.

### 7. Shapefiles, GeoTIFF an other GIS data dources {#shapefile}

With the GIS extension we provide **OGRVectorDataSource** and
**GDALRasterTileDataSource** which can open ESRI Shapefile, GeoTIFF,
KML, GML, GPX, MapInfo and many other GIS vector and raster formats
directly. Your code just defines file and layer to be used. Nutiteq
provides it as commercial package, also free evaluation is avaliable,
email sales@nutiteq.com to get it.

### 8. Custom data sources

Last but not least - your application can define own Data Sources.

See *MyMergedRasterTileDataSource* in samples to get example for virtual
datasource which takes raster data from two raster datasources and
merges the map in pixel level. Note that the second datasources should
have a lot of transparent or semitransparent pixels, or the pixels from
the first datasource will be hidden.

Custom DataSource should extend one of following abstract base classes:

-   **TileDataSource** - for tiles (raster or vector)
-   **VectorDataSource** - for vector objects
-   **CacheTileDataSource** - for custom tile caching (it is a
    *TileDataSource*)

<script>
\$( “\#tabs1” ).tabs();\
 \$( “\#tabs2” ).tabs();\
 \$( “\#tabs3” ).tabs();\
 \$( “\#tabs4” ).tabs();\
 \$( “\#tabs5” ).tabs();\
 \$( “\#tabs6” ).tabs();

</script>

