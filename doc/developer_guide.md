# Developer Guide to SDK Features

The following advanced features can be customized by mobile app developers.

[Loading mobile data](#loading-mobile-data) | 
[Listening to events](#listening-to-events) | 
[Clusters to display multiple points of interest as objects](#clusters) |
[Ground overlays to project a bitmap image over a basemap](#ground-overlays) | 
[Vector styles for vector map tiles](#vector-styles)  |

## Loading Mobile Data

The following DataSources are available for the Mobile SDK, and are supported on all mobile platforms.

**Note:** Some DataSources are more generic and can be used for different data types and map layers (vector or raster). Others can only be applied for specific layer types.

## Offline Maps from MBTiles

**MBTilesTileDataSource** is universal datasource for raster, or vector, tile data.

### MBtiles as RasterTileLayer

For raster data, use [MBTiles](https://github.com/mapbox/mbtiles-spec) format files, and use the following code to generate them:

<div class="js-TabPanes">
  <ul class="Tabs">
    <li class="Tab js-Tabpanes-navItem is-active">
      <a href="#/0" class="js-Tabpanes-navLink">Java</a>
    </li>
    <li class="Tab js-Tabpanes-navItem">
      <a href="#/1" class="js-Tabpanes-navLink">C#</a>
    </li>
    <li class="Tab js-Tabpanes-navItem">
      <a href="#/2" class="js-Tabpanes-navLink">Objective-C</a>
    </li>
    <li class="Tab js-Tabpanes-navItem">
      <a href="#/3" class="js-Tabpanes-navLink">Swift</a>
    </li>
  </ul>

  <div class="Carousel-item js-Tabpanes-item is-active">
  {% highlight java %}MBTilesTileDataSource tileDataSource = new MBTilesTileDataSource(filePath);
    TileLayer rasterLayer = new RasterTileLayer(tileDataSource);

    mapView.getLayers().add(rasterLayer);

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item">
  {% highlight c# %}var mbTilesLayer = new RasterTileLayer(new MBTilesTileDataSource(filePath));
      MapView.Layers.Add(mbTilesLayer);

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item">
  {% highlight objc %}NSString* fullpathVT = [[NSBundle mainBundle] pathForResource:@"MBTILESFILENAME" ofType:@"mbtiles"];
   NTTileDataSource* tileDataSource = [[NTMBTilesTileDataSource alloc] initWithPath: fullpathVT];

// 1. Initialize a raster layer with the previous data source
      NTRasterTileLayer* rasterLayer = [[NTRasterTileLayer alloc] initWithDataSource:tileDataSource];

// 2. Add the raster layer to the map
      [[mapView getLayers] add:rasterLayer];

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item">
  {% highlight swift %}COMING SOON...

  {% endhighlight %}
  </div>
  
</div>

### MBtiles as VectorTileLayer

For packaged vector data, you will need CARTO specific vector files packages (NTVT - *NutiTeq Vector Tile*) and styling files must be in _Mapnik XML_ format. Download the following, free sample package using OpenStreetMap data:

-   [estonia\_ntvt.mbtiles](https://dl.dropboxusercontent.com/u/3573333/public_web/ntvt_packages/estonia_ntvt.mbtiles)

The Mobile SDK provides a built-in download service thorough the **Package Manager** to retrieve map packages for a country, or smaller region. For details, see [Offline Map Packages](/docs/carto-engine/mobile-sdk/offline-maps/#offline-map-packages).

**Note:** Vector maps always need proper **style definitions**. You can find compatible stylesheet files from our sample app projects (OSMBright with 3D buildings [osmbright.zip](/docs/carto-engine/mobile-sdk/getting-started/#sample-apps)), which contains **Mapnik XML styling** with resources (fonts, images) included. 

Use the following code to use MBtiles for a vector layer: 

<div class="js-TabPanes">
  <ul class="Tabs">
    <li class="Tab js-Tabpanes-navItem is-active">
      <a href="#/0" class="js-Tabpanes-navLink">Java</a>
    </li>
    <li class="Tab js-Tabpanes-navItem">
      <a href="#/1" class="js-Tabpanes-navLink">C#</a>
    </li>
    <li class="Tab js-Tabpanes-navItem">
      <a href="#/2" class="js-Tabpanes-navLink">Objective-C</a>
    </li>
    <li class="Tab js-Tabpanes-navItem">
      <a href="#/3" class="js-Tabpanes-navLink">Swift</a>
    </li>
  </ul>

  <div class="Carousel-item js-Tabpanes-item is-active">
  {% highlight java %}

// 1. Create tile data source from mbtiles file
      MBTilesTileDataSource tileDataSource = new MBTilesTileDataSource("/sdcard/estonia_ntvt.mbtiles");

// 2. Load vector tile styleset
      BinaryData styleBytes = AssetUtils.loadAsset("osmbright.zip");
      CompiledStyleSet vectorTileStyleSet = new CompiledStyleSet(new ZippedAssetPackage(styleBytes));

// 3. Create vector tile decoder using the styleset
      VectorTileDecoder vectorTileDecoder = new MBVectorTileDecoder(vectorTileStyleSet);

// 4. Create vector tile layer, using previously created data source and decoder
      TileLayer vectorTileLayer = new VectorTileLayer(tileDataSource, vectorTileDecoder);

// 5. Add vector tile layer
      mapView.getLayers().add(vectorTileLayer);

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item">
  {% highlight c# %}

// 1. Create tile data source from mbtiles file
      var tileDataSource = new MBTilesTileDataSource("world_zoom5.mbtiles");

// 2. Load vector tile styleset
      var styleBytes = AssetUtils.LoadAsset("nutiteq-dark.zip");
      var vectorTileStyleSet = new CompiledStyleSet(new ZippedAssetPackage(styleBytes));

// 3. Create vector tile decoder using the styleset
      var vectorTileDecoder = new MBVectorTileDecoder(vectorTileStyleSet);

// 4. Create vector tile layer, using previously created data source and decoder
      var vectorTileLayer = new VectorTileLayer(tileDataSource, vectorTileDecoder);

// 5. Add vector tile layer
      MapView.Layers.Add(vectorTileLayer);

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item">
  {% highlight objc %}NSString* fullpathVT = [[NSBundle mainBundle] pathForResource:@"estonia_ntvt" ofType:@"mbtiles"];
    NTTileDataSource* tileDataSource = [[NTMBTilesTileDataSource alloc] initWithMinZoom:0 maxZoom:4 path: fullpathVT];
    
// 1. Load vector tile styleset
      NTBinaryData *vectorTileStyleSetData = [NTAssetUtils loadAsset: @"osmbright.zip"];
      NTZippedAssetPackage *package = [[NTZippedAssetPackage alloc] initWithZipData:vectorTileStyleSetData];
      NTCompiledStyleSet *vectorTileStyleSet = [[NTCompiledStyleSet alloc] initWithAssetPackage:package];
    
// 2. Create vector tile decoder using the styleset
      NTMBVectorTileDecoder *vectorTileDecoder = [[NTMBVectorTileDecoder alloc] initWithCompiledStyleSet:vectorTileStyleSet];
    
// 3. Create vector tile layer, using previously created data source and decoder
      NTVectorTileLayer *layer = [[NTVectorTileLayer alloc] initWithDataSource:tileDataSource decoder:vectorTileDecoder];
    
// 4. Add vector tile layer
      [[mapView getLayers] add:layer];

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item">
  {% highlight swift %}COMING SOON...

  {% endhighlight %}
  </div>
  
</div>

## MBTiles for Map Data

MBTiles contain the TileJSON formats that include basic map styling for Mapbox. [MBTiles](http://mapbox.com/developers/mbtiles/) support is included in CARTO Mobile SDK. MBTiles uses the SQLite database format, which only requires one file to be downloaded and managed.

### Raster Maps

The following tools enable you to create MBTile packages:

- [MapTiler](http://www.maptiler.com/) is a utility to create MBTiles from raster geo files (GeoTIFF, JPG, ECW, and so on)

* [TileMill](http://mapbox.com/tilemill/) is an open source generator of map packages for vector geo files, such as Shapefile or PosgGIS geo data
- [MOBAC](http://mobac.sourceforge.net) is available to download from variety of free sources, such as Bing, OpenStreetMap, and so on. You can even load it from WMS with added configuration

- [MBUtil](https://github.com/mapbox/mbutil) enables you to create mbtiles from/to TMS-style tile folders, created with different utilities, such as GDAL utility

- [Portable Basemap Server](https://geopbs.codeplex.com/) is a free utility for Windows and loads data from various commercial servers and custom sources. It is also available in ESRI formats. It works mainly as WMTS server, but can create MBTiles as an extra feature

### Vector Maps

The following vector map tools enable you to create MBTiles:

- The Mobile SDK bundled [Package Manager API](/docs/carto-engine/mobile-sdk/offline-maps/) downloads country-wide, or bounding box based, map packages with **OpenStreetMap data**. These files are not directly visible, as Package Manager downloads the files, you only use the API

_**Note:** If you need to create vector map files **from different sources, or with your own data** (e.g. Shapefiles), please contact [CARTO support](mailto:carto@support.com)._ Our Mobile Developers will provide you with a free converter toolchain for custom conversions.

## Online Maps

**HTTPTileDataSource** is a general datasource that can be used for vector or raster tiles. You can also use CARTO or Web (HTTP) tiles for vector layers. Additionally, you can use Web (HTTP) raster tiles for map layers. 

### CARTO Online Vector Tile Layer

The Mobile SDK contains built-in vector tiles, as provided by CARTO maps as a service, and is avilale for all account plans. This is useful for basic maps. For vector styling, you can use the same assets provided for offline tiles (OSMBright with 3D buildings [osmbright.zip](/docs/carto-engine/mobile-sdk/getting-started/#sample-apps)).

<div class="js-TabPanes">
  <ul class="Tabs">
    <li class="Tab js-Tabpanes-navItem is-active">
      <a href="#/0" class="js-Tabpanes-navLink">Java</a>
    </li>
    <li class="Tab js-Tabpanes-navItem">
      <a href="#/1" class="js-Tabpanes-navLink">C#</a>
    </li>
    <li class="Tab js-Tabpanes-navItem">
      <a href="#/2" class="js-Tabpanes-navLink">Objective-C</a>
    </li>
    <li class="Tab js-Tabpanes-navItem">
      <a href="#/3" class="js-Tabpanes-navLink">Swift</a>
    </li>
  </ul>

  <div class="Carousel-item js-Tabpanes-item is-active">
  {% highlight java %}BinaryData styleAsset = AssetUtils.loadAsset("nutibright-v2a.zip");
        VectorTileLayer baseLayer = new CartoOnlineVectorTileLayer("nutiteq.osm", new ZippedAssetPackage(styleAsset));

        mapView.getLayers().add(baseLayer);

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item">
  {% highlight c# %}var styleAsset = AssetUtils.LoadAsset("nutibright-v2a.zip");
      var baseLayer = new CartoOnlineVectorTileLayer("nutiteq.osm", new ZippedAssetPackage(styleAsset));
      
      MapView.Layers.Add(baseLayer);

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item">
  {% highlight objc %}NTBinaryData* styleData = [NTAssetUtils loadAsset:@"nutibright-v3.zip"];
    NTAssetPackage* assetPackage = [[NTZippedAssetPackage alloc] initWithZipData:styleData];
    NTVectorTileLayer* vectorTileLayer = [[NTCartoOnlineVectorTileLayer alloc] initWithSource: @"nutiteq.osm" styleAssetPackage:assetPackage];

    [[mapView getLayers] add:vectorTileLayer];
  
  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item">
  {% highlight swift %}COMING SOON...

  {% endhighlight %}
  </div>
  
</div>

### Web (HTTP) Tiles as VectorTileLayer

Some Mobile SDK plans enable you to use other vector tile map DataSources. These are similar to custom raster map data sources, but a vector decoder is needed for correct implementation.

- Connect to MapBox vector tiles, which have a very similar (but not identical) data structure. You can use the same (OSMBright with 3D buildings [osmbright.zip](/docs/carto-engine/mobile-sdk/getting-started/#sample-apps)) asset for vector styling

  **Note:** Before using these assets in your app, confirm with MapBox if this is allowed and make sure you use your MapBox license key in the URL. You may need to use your custom stylying and vector tile server URL.

The following tags are supported in the URL definition: **zoom, x, y, xflipped, yflipped, quadkey**.

<div class="js-TabPanes">
  <ul class="Tabs">
    <li class="Tab js-Tabpanes-navItem is-active">
      <a href="#/0" class="js-Tabpanes-navLink">Java</a>
    </li>
    <li class="Tab js-Tabpanes-navItem">
      <a href="#/1" class="js-Tabpanes-navLink">C#</a>
    </li>
    <li class="Tab js-Tabpanes-navItem">
      <a href="#/2" class="js-Tabpanes-navLink">Objective-C</a>
    </li>
    <li class="Tab js-Tabpanes-navItem">
      <a href="#/3" class="js-Tabpanes-navLink">Swift</a>
    </li>
  </ul>

  <div class="Carousel-item js-Tabpanes-item is-active">
  {% highlight java %}

// 1. Define data source
      TileDataSource tileDataSource = new HTTPTileDataSource(0, 14, "http://a.tiles.mapbox.com/v4/mapbox.mapbox-streets-v5/{zoom}/{x}/{y}.vector.pbf?access_token=YOUR-MAPBOX-TOKEN");

// 2. Load vector tile styleset
      BinaryData styleBytes = AssetUtils.loadAsset("osmbright.zip");
      CompiledStyleSet vectorTileStyleSet = new CompiledStyleSet(new ZippedAssetPackage(styleBytes));

// 3. Create vector tile decoder using the styleset
      VectorTileDecoder vectorTileDecoder = new MBVectorTileDecoder(vectorTileStyleSet);

// 4. Create vector tile layer, using previously created data source and decoder
      TileLayer vectorTileLayer = new VectorTileLayer(tileDataSource, vectorTileDecoder);

// 5. Add vector tile layer
      mapView.getLayers().add(vectorTileLayer);

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item">
  {% highlight c# %}

// 1. Define data source
      var tileDataSource = new HTTPTileDataSource(0, 14, "http://a.tiles.mapbox.com/v4/mapbox.mapbox-streets-v5/{zoom}/{x}/{y}.vector.pbf?access_token=YOUR-MAPBOX-TOKEN");

// 2. Load vector tile styleset
      var styleBytes = AssetUtils.LoadAsset("nutiteq-dark.zip");
      var vectorTileStyleSet = new CompiledStyleSet(new ZippedAssetPackage(styleBytes));

// 3. Create vector tile decoder using the styleset
      var vectorTileDecoder = new MBVectorTileDecoder(vectorTileStyleSet);

// 4. Create vector tile layer, using previously created data source and decoder
      var vectorTileLayer = new VectorTileLayer(tileDataSource, vectorTileDecoder);

// 5. Add vector tile layer
      MapView.Layers.Add(vectorTileLayer);

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item">
  {% highlight objc %}

// 1. Define data source
      NTTileDataSource* tileDataSource = [[NTHTTPTileDataSource alloc] initWithMinZoom:0 maxZoom:14 baseURL:@"http://a.tiles.mapbox.com/v4/mapbox.mapbox-streets-v5/{zoom}/{x}/{y}.vector.pbf?access_token=YOUR-MAPBOX-TOKEN"];
    
// 2. Load vector tile styleset
      NTBinaryData *vectorTileStyleSetData = [NTAssetUtils loadAsset: @"osmbright.zip"];
      NTZippedAssetPackage *package = [[NTZippedAssetPackage alloc] initWithZipData:vectorTileStyleSetData];
      NTCompiledStyleSet *vectorTileStyleSet = [[NTCompiledStyleSet alloc] initWithAssetPackage:package];
    
// 3. Create vector tile decoder using the styleset
      NTMBVectorTileDecoder *vectorTileDecoder = [[NTMBVectorTileDecoder alloc] initWithCompiledStyleSet:vectorTileStyleSet];
    
// 4. Create vector tile layer, using previously created data source and decoder
      NTVectorTileLayer *vectorTileLayer = [[NTVectorTileLayer alloc] initWithDataSource:tileDataSource decoder:vectorTileDecoder];
    
// 5. Add vector tile layer
      [[mapView getLayers] add:vectorTileLayer];

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item">
  {% highlight swift %}COMING SOON...

  {% endhighlight %}
  </div>
  
</div>

### Web (HTTP) Tiles as RasterTileLayer

For online raster tiles you can use any common web tiles in PNG or JPG formats, in Spherical Mercator system. See [Raster Tile Sources](#raster-tile-sources).

The following tags are supported in the URL definition: **zoom, x, y, xflipped,
yflipped, quadkey**.

<div class="js-TabPanes">
  <ul class="Tabs">
    <li class="Tab js-Tabpanes-navItem is-active">
      <a href="#/0" class="js-Tabpanes-navLink">Java</a>
    </li>
    <li class="Tab js-Tabpanes-navItem">
      <a href="#/1" class="js-Tabpanes-navLink">C#</a>
    </li>
    <li class="Tab js-Tabpanes-navItem">
      <a href="#/2" class="js-Tabpanes-navLink">Objective-C</a>
    </li>
    <li class="Tab js-Tabpanes-navItem">
      <a href="#/3" class="js-Tabpanes-navLink">Swift</a>
    </li>
  </ul>

  <div class="Carousel-item js-Tabpanes-item is-active">
  {% highlight java %}TileDataSource baseRasterTileDataSource = new HTTPTileDataSource(1, 19, "http://ecn.t3.tiles.virtualearth.net/tiles/a{quadkey}.jpeg?g=471&mkt=en-US");

        TileLayer baseLayer = new RasterTileLayer(baseRasterTileDataSource);

        mapView.getLayers().add(baseLayer);

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item">
  {% highlight c# %}

// 1. Create a Bing raster data source. Note: tiles start from level 1, there is no single root tile!
      var baseRasterTileDataSource = new HTTPTileDataSource(1, 19, "http://ecn.t3.tiles.virtualearth.net/tiles/a{quadkey}.jpeg?g=471&mkt=en-US");

// 2. Create layer and add to map
      var baseLayer = new RasterTileLayer(baseRasterTileDataSource);
      MapView.Layers.Add(baseLayer);

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item">
  {% highlight objc %}NTHTTPTileDataSource* baseRasterTileDataSource = [[NTHTTPTileDataSource alloc] initWithMinZoom:0 maxZoom:19 baseURL:@"http://otile1.mqcdn.com/tiles/1.0.0/osm/{zoom}/{x}/{y}.png"];
    
// 1. Initialize a raster layer with the previous data source
      NTRasterTileLayer* rasterLayer = [[NTRasterTileLayer alloc] initWithDataSource:baseRasterTileDataSource];
    
// 2. Add the previous raster layer to the map
      [[mapView getLayers] add:rasterLayer];

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item">
  {% highlight swift %}COMING SOON...

  {% endhighlight %}
  </div>
  
</div>

## App-defined Vector Overlay

Mobile SDK contains in-memory DataSources where applications can add vector objects as the map overlay layer.

`LocalVectorDataSource` is a dynamic DataSource that is initially empty, and allows you to add or remove objects. Objects are cached and by default, all added objects are rendered. For detailed code samples, see [Basic Map Features](/docs/carto-engine/mobile-sdk/getting-started/#basic-map-features) for examples of adding Points, Markers, Texts, Lines and Polygons to map.

## App-defined Ground Overlays

`BitmapOverlayRasterTileDataSource` defines a bitmap image and the geographical coordinates of the bitmap. For details, see [Ground Overlays](#ground-overlays).

## Virtual DataSources

There are some DataSources which can use another DataSource as input, and apply internal processing and create a new DataSource as the result. This enables you to create a new DataSource based on a customized logic.

-   `PersistentCacheTileDataSource` caches HTTP tiles to a persistent sqlite database file. If the tile already exists in the database, the request to the original DataSource is ignored. This can be applied for both raster and vector tiles. The original DataSource's expired headers are taken into account as part of the processing.

<div class="js-TabPanes">
  <ul class="Tabs">
    <li class="Tab js-Tabpanes-navItem is-active">
      <a href="#/0" class="js-Tabpanes-navLink">Java</a>
    </li>
    <li class="Tab js-Tabpanes-navItem">
      <a href="#/1" class="js-Tabpanes-navLink">C#</a>
    </li>
    <li class="Tab js-Tabpanes-navItem">
      <a href="#/2" class="js-Tabpanes-navLink">Objective-C</a>
    </li>
    <li class="Tab js-Tabpanes-navItem">
      <a href="#/3" class="js-Tabpanes-navLink">Swift</a>
    </li>
  </ul>

  <div class="Carousel-item js-Tabpanes-item is-active">
  {% highlight java %}

// 1. Create a Bing raster data source. Note: tiles start from level 1, there is no single root tile!
      TileDataSource baseRasterTileDataSource = new HTTPTileDataSource(1, 19, "http://ecn.t3.tiles.virtualearth.net/tiles/a{quadkey}.jpeg?g=471&mkt=en-US");

// 2. Add persistent caching datasource, tiles will be stored locally on persistent storage
      PersistentCacheTileDataSource cachedDataSource = 
        new PersistentCacheTileDataSource(baseRasterTileDataSource, getExternalFilesDir(null)+"/mapcache.db");

// 3. Create layer and add to map
      TileLayer baseLayer = new RasterTileLayer(cachedDataSource);
      mapView.getLayers().add(baseLayer);

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item">
  {% highlight c# %}

// 1. Create a Bing raster data source. Note: tiles start from level 1, there is no single root tile!
      var baseRasterTileDataSource = new HTTPTileDataSource(1, 19, "http://ecn.t3.tiles.virtualearth.net/tiles/a{quadkey}.jpeg?g=471&mkt=en-US");

      // Add persistent caching datasource, tiles will be stored locally on persistent storage
      // fileDir must be a directory where files can be written - this is platform-specific
      var cachedDataSource = new PersistentCacheTileDataSource(baseRasterTileDataSource, fileDir+"/mapcache.db");

// 2. Create layer and add to map
      var baseLayer = new RasterTileLayer(cachedDataSource);
      MapView.Layers.Add(baseLayer);

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item">
  {% highlight objc %}

// 1. Initialize a OSM raster data source from MapQuest Open Tiles
      NTHTTPTileDataSource* baseRasterTileDataSource = [[NTHTTPTileDataSource alloc] initWithMinZoom:0 maxZoom:19 baseURL:@"http://otile1.mqcdn.com/tiles/1.0.0/osm/{zoom}/{x}/{y}.png"];

// 2. Create persistent cache for the given data source  
      NTPersistentCacheTileDataSource* cachedRasterTileDataSource = [[NTPersistentCacheTileDataSource alloc] initWithDataSource:baseRasterTileDataSource databasePath:[NTAssetUtils calculateWritablePath:@"mycache.db"]];

// 3. Initialize a raster layer with the previous data source
      NTRasterTileLayer* rasterLayer = [[NTRasterTileLayer alloc] initWithDataSource:cachedRasterTileDataSource];

// 4. Add the previous raster layer to the map
      [[self.mapView getLayers] add:rasterLayer];

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item">
  {% highlight swift %}COMING SOON...

  {% endhighlight %}
  </div>
  
</div>

-   **CompressedCacheTileDataSource**

An in-memory cache DataSource. Although layers also cache tiles, the tiles are uncompressed and usually take 2-5x more memory after being loaded. `CompressedCacheTileDataSource` keeps all tiles in a compressed format for better use of existing memory.

-   **CombinedTileDataSource**

A tile DataSource that combines two data sources (usually offline and online) and selects tiles based on the zoom level. All requests below a specified zoom level are directed to the first DataSource. All requests at, or above, a specified zoom level are directed to the second Datasource.

## Other Built-in DataSources

The following datasources are also built into the Mobile SDK packages.

-   **AssetTileDataSource** - loads offline tiles data from a folder (/sdcard or Application sandbox, depending on platform). For example, you can use zoom/x/y.png or zoom\_x\_y.png file naming structures for offline data

-   **NMLModelLODTreeOnlineDataSource** - uses NMLDB online API for “3D city” layer. 3D tools are provided as commercial service. [Contact us](mailto:sales@carto.com) for more information.

-   **NMLModelLODTreeSqliteDataSource** - An offline version of *NMLModelLODTreeOnlineDataSource*, model that is stored in a sqlite database file. Simple individual 3D objects (NML models) can also be added to the usual *LocalVectorDataSource*

## Shapefiles, GeoTIFF and other GIS DataSources

With the GIS extension we provide **OGRVectorDataSource** and **GDALRasterTileDataSource** which can open ESRI Shapefile, GeoTIFF, KML, GML, GPX, MapInfo and many other GIS vector and raster formats. The mobile codes just defines the file and layer to be used in your mobile app. CARTO provides a commercial package for these extensions. [Contact us](mailto@support.com) for a free evaluation demonstration.

## Custom DataSources

You can define a custom datasource for your mobile application. See *MyMergedRasterTileDataSource* from the [Sample Apps](/docs/carto-engine/mobile-sdk/getting-started/#sample-apps) for an example of using a virtual DataSource. This example uses raster data from two raster datasources and merges the map using pixel aggregations. Note that the second datasource contains transparent, or semitransparent pixels. Otherwise, the pixels from the first datasource are hidden.

Custom DataSourced should extend one of following abstract base classes:

-   **TileDataSource** - for tiles (raster or vector)
-   **VectorDataSource** - for vector objects
-   **CacheTileDataSource** - for custom tile caching (it is a *TileDataSource*)

## Raster Tile Sources

Raster tiles can produce a faster loading map by rendering the map based on pixels of small images. With certain account plans, you can use use external raster map tiles with the Mobile SDK. You can also create your own custom raster tile map source. This section describes the raster tile features available in the Mobile SDK.

In order to apply a raster tile source:

- Use the `HTTPTileDataSource` API function as the tile source

- Create `RasterTileLayer` to apply it

**Tip:** For general information about raster tiles and geospatial data, see the following Open Source Geospatial Foundation reference materials for [Tile Map Service](http://wiki.openstreetmap.org/wiki/TMS) and [Featured Tile Layers](http://wiki.openstreetmap.org/wiki/Featured_tiles).

### Code Sample

The DataSource constructor uses the following URL patterns. It requires a minimum zoom level above `0`, and a maximum zoom level between `14 and 19`

<div class="js-TabPanes">
  <ul class="Tabs">
    <li class="Tab js-Tabpanes-navItem is-active">
      <a href="#/0" class="js-Tabpanes-navLink">Java</a>
    </li>
    <li class="Tab js-Tabpanes-navItem">
      <a href="#/1" class="js-Tabpanes-navLink">C#</a>
    </li>
    <li class="Tab js-Tabpanes-navItem">
      <a href="#/2" class="js-Tabpanes-navLink">Objective-C</a>
    </li>
    <li class="Tab js-Tabpanes-navItem">
      <a href="#/3" class="js-Tabpanes-navLink">Swift</a>
    </li>
  </ul>

  <div class="Carousel-item js-Tabpanes-item is-active">
  {% highlight java %}String url = "http://your-url-with-placeholders-see-below";
      TileDataSource tileDataSource = new HTTPTileDataSource(0, 18, url);
      RasterTileLayer layer = new RasterTileLayer(tileDataSource);

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item">
  {% highlight c# %}string url = "http://your-url-with-placeholders-see-below";
    TileDataSource tileDataSource = new HTTPTileDataSource(0, 18, url);
    RasterTileLayer layer = new RasterTileLayer(tileDataSource);

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item">
  {% highlight objc %}NSString* url = @"http://your-url-with-placeholders-see-below";
    NTHTTPTileDataSource *source = [[NTHTTPTileDataSource alloc]initWithMinZoom:0 maxZoom:18 baseURL:url];
    NTRasterTileLayer *layer = [[NTRasterTileLayer alloc]initWithDataSource:source];

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item">
  {% highlight swift %}COMING SOON...

  {% endhighlight %}
  </div>
  
</div>

<br/><br/>
The following sections provide the code for rester tile basemap urls, and a sample image.

### CARTO Positron Tiles

[`http://{s}.basemaps.cartocdn.com/light_all/{z}/{x}/{y}.png`](https://carto.com/location-data-services/basemaps/)

<img src="http://a.basemaps.cartocdn.com/light_all/17/65490/43588.png"/>

### CARTO Dark Matter Tiles

[`http://a.basemaps.cartocdn.com/dark_all/{zoom}/{x}/{y}.png`](https://carto.com/location-data-services/basemaps/)

<img src="http://a.basemaps.cartocdn.com/dark_all/17/65490/43588.png"/>

### OpenStreetMap Mapnik Tiles

[`http://a.tile.openstreetmap.org/{zoom}/{x}/{y}.png`](http://www.osm.org)

<img src="http://b.tile.openstreetmap.org/15/5241/12661.png"/>

**Note:** OpenStreetMap Mapnik tiles are free for limited use. See a description of their [_Tile usage policy_](http://wiki.openstreetmap.org/wiki/Tile_usage_policy).

### Mapbox Raster Tiles

[`http://api.mapbox.com/v4/{mapid}/{zoom}/{x}/{y}.png?access_token=&lt;your access token&gt;`](http://www.mapbox.com)

<img src="http://api.tiles.mapbox.com/v3/nutiteq.map-f0sfyluv/17/65490/43588.png"/>

See <a href=" https://www.mapbox.com/developers/api/maps/">MapBox API spec</a> for details about how to create access tokens. <a href='http://mapbox.com/about/maps' target='_blank'>MapBox Attributions</a>. 

### Stamen Toner

[`http://a.tile.stamen.com/toner/{zoom}/{x}/{y}.png`](http://maps.stamen.com)

<img src="http://a.tile.stamen.com/toner/13/1310/3166.png"/>

Please contact [Stamen](http://www.stamen.com) directly.

### Stamen Watercolor

[`http://tile.stamen.com/watercolor/{zoom}/{x}/{y}.png`](http://maps.stamen.com/#watercolor/12/37.7706/-122.3782)

<img src="http://tile.stamen.com/watercolor/12/654/1583.jpg"/>

Please contact [Stamen](http://www.stamen.com) directly.

### Stamen Terrain - US Only

[`http://tile.stamen.com/terrain/{zoom}/{x}/{y}.png`](http://maps.stamen.com/#terrain/12/37.7706/-122.3782)

<img src="http://tile.stamen.com/terrain/12/654/1583.jpg"/>

Please contact [Stamen](http://www.stamen.com) directly.

### Microsoft Bing Maps

`http://ecn.t3.tiles.virtualearth.net/tiles/r{quadkey}.png?g=1&mkt=en-US&shading=hill&n=z`

<img width="256" height="256" src="http://ecn.t3.tiles.virtualearth.net/tiles/r1202.png?g=1&mkt=en-US&shading=hill&n=z" alt=""/>

<a href="http://www.microsoft.com/maps/product/terms.html">Commercial terms of Bing Maps API</a>. Must use <a href="http://msdn.microsoft.com/en-us/library/ff701716.aspx">Get Imagery Metadata Bing Service </a> to get supported URL parameters. 

### Microsoft Bing Aerials

`http://ecn.t3.tiles.virtualearth.net/tiles/a{quadkey}.jpeg?g=1&mkt=en-US`

<img width="256" height="256"  src="http://ecn.t3.tiles.virtualearth.net/tiles/a032010110123333.jpeg?g=1&mkt=en-US" alt=""/>

<a href="http://www.microsoft.com/maps/product/terms.html">See Bing's APIs Terms of Use</a>. You must use the <a href="http://msdn.microsoft.com/en-us/library/ff701716.aspx">Get Imagery Metadata</a> service to retrieve supported URL parameters.

### ESRI World Imagery (and other ESRI Basemaps)

`http://services.arcgisonline.com/arcgis/rest/services/World_Imagery/MapServer/tile/{zoom}/{y}/{x}`

<img width="256" height="256"  src="http://server.arcgisonline.com/ArcGIS/rest/services/World_Imagery/MapServer/tile/16/24351/35037" alt=""/>

<a href="http://www.arcgis.com/home/item.html?id=10df2279f9684e4a9f6a7f08febac2a9">See their Terms of Use</a>. Note that the ArcGIS&trade; Developer service subscription is required.

### OpenCycleMap

`http://a.tile.opencyclemap.org/cycle/{zoom}/{x}/{y}.png`

<img width="256" height="256"  src="http://b.tile.opencyclemap.org/cycle/13/4092/2719.png" alt=""/>

© <a href="http://www.opencyclemap.org">OpenCycleMap</a>

### OpenCycleMap Transport

`http://a.tile2.opencyclemap.org/transport/{zoom}/{x}/{y}.png`

<img width="256" height="256"  src="http://a.tile2.opencyclemap.org/transport/16/35209/21491.png" alt=""/>

Open source resource: <a href="http://www.thunderforest.com/transport/">Thunderforest</a>

### USGS Topo National Map (US Only)

`http://basemap.nationalmap.gov/arcgis/rest/services/USGSTopo/MapServer/WMTS/tile/1.0.0/USGSTopo/default/GoogleMapsCompatible/{zoom}/{y}/{x}`

<img width="256" height="256"  src="http://basemap.nationalmap.gov/arcgis/rest/services/USGSTopo/MapServer/WMTS/tile/1.0.0/USGSTopo/default/GoogleMapsCompatible/13/3162/1306" alt="USGS Topo"/>

© USGS The National Map. The National Boundaries Dataset, National Elevation Dataset, Geographic Names Information System, National Hydrography Dataset, National Land Cover Database, National Structures Dataset, and National Transportation Dataset; U.S. Census Bureau - TIGER/Line; HERE Road Data. <a href="http://basemap.nationalmap.gov/arcgis/rest/services/USGSTopo/MapServer">USGS MapServer</a>

## Listening to Events

Once you have implemented and set the *MapEventListener* interface for a MapView, the following listener events are generated as parameters:

* **onMapMoved** - map moving, zooming, tilting, rotating. This event is called for each pixel movement, so any changed to this parameter can make map panning animation visibly slower

* **onMapClicked(MapClickInfo)** - map clicked, provides info about click:

  * **ClickType** - results include `CLICK_TYPE_SINGLE`, `CLICK_TYPE_LONG`, `CLICK_TYPE_DOUBLE` or `CLICK_TYPE_DUAL`

  * **ClickPos** - geographical click position (in map projection)

* **onVectorElementClicked(VectorElementsClickInfo)** - any vector object on map was clicked (could be line,polygon,point,marker,balloon popup or a 3D model)

  * **ClickType** - same description as above, results include `CLICK_TYPE_SINGLE`, `CLICK_TYPE_LONG`, `CLICK_TYPE_DOUBLE` or `CLICK_TYPE_DUAL`

  * **VectorElementClickInfos** - sorted list of clicked objects, nearest is first. Each ClickInfo contains:

    * **ClickPos** - geographical click location

    * **ElementClickPos** - click locaton projected to element, different from ClickPos if click was not precisely on object (which it never really is)

    * **Distance** - distance from the vector element to the camera

    * **Order** - order of element within VectorElementClickInfos

    * **VectorElement** - the actual clicked element

### Implement MapEventListener

Create a new class called **MyMapEventListener** which implements MapEventListner interface.

<div class="js-TabPanes">
  <ul class="Tabs">
    <li class="Tab js-Tabpanes-navItem is-active">
      <a href="#/0" class="js-Tabpanes-navLink">Java</a>
    </li>
    <li class="Tab js-Tabpanes-navItem">
      <a href="#/1" class="js-Tabpanes-navLink">C#</a>
    </li>
    <li class="Tab js-Tabpanes-navItem">
      <a href="#/2" class="js-Tabpanes-navLink">Objective-C</a>
    </li>
    <li class="Tab js-Tabpanes-navItem">
      <a href="#/3" class="js-Tabpanes-navLink">Swift</a>
    </li>
  </ul>

  <div class="Carousel-item js-Tabpanes-item is-active">
  {% highlight java %}

  // imports omitted...
  
  /**
   * A custom map event listener that displays information about map events and creates pop-ups.
   */
  public class MyMapEventListener extends MapEventListener {
    private MapView mapView;
    private LocalVectorDataSource vectorDataSource;
    
    private BalloonPopup oldClickLabel;
    
    public MyMapEventListener(MapView mapView, LocalVectorDataSource vectorDataSource) {
      this.mapView = mapView;
      this.vectorDataSource = vectorDataSource;
    }

    @Override
    public void onMapMoved() {

          final MapPos topLeft = mapView.screenToMap(new ScreenPos(0, 0));
          final MapPos bottomRight = mapView.screenToMap(new ScreenPos(mapView.getWidth(), mapView.getHeight()));
          Log.d(Const.LOG_TAG, mapView.getOptions().getBaseProjection().toWgs84(topLeft)
                  + " " + mapView.getOptions().getBaseProjection().toWgs84(bottomRight));

    }

    @Override
    public void onMapClicked(MapClickInfo mapClickInfo) {
      Log.d(Const.LOG_TAG, "Map click!");
      
      // Remove old click label
      if (oldClickLabel != null) {
        vectorDataSource.remove(oldClickLabel);
        oldClickLabel = null;
      }
      
      BalloonPopupStyleBuilder styleBuilder = new BalloonPopupStyleBuilder();
        // Make sure this label is shown on top all other labels
        styleBuilder.setPlacementPriority(10);
      
      // Check the type of the click
      String clickMsg = null;
      if (mapClickInfo.getClickType() == ClickType.CLICK_TYPE_SINGLE) {
        clickMsg = "Single map click!";
      } else if (mapClickInfo.getClickType() == ClickType.CLICK_TYPE_LONG) {
        clickMsg = "Long map click!";
      } else if (mapClickInfo.getClickType() == ClickType.CLICK_TYPE_DOUBLE) {
        clickMsg = "Double map click!";
      } else if (mapClickInfo.getClickType() == ClickType.CLICK_TYPE_DUAL) {
        clickMsg ="Dual map click!";
      }
    
      MapPos clickPos = mapClickInfo.getClickPos();
      MapPos wgs84Clickpos = mapView.getOptions().getBaseProjection().toWgs84(clickPos);
      String msg = String.format(Locale.US, "%.4f, %.4f", wgs84Clickpos.getY(), wgs84Clickpos.getX());
      BalloonPopup clickPopup = new BalloonPopup(mapClickInfo.getClickPos(),
                             styleBuilder.buildStyle(),
                                     clickMsg,
                                     msg);
      vectorDataSource.add(clickPopup);
      oldClickLabel = clickPopup;
    }

    @Override
    public void onVectorElementClicked(VectorElementsClickInfo vectorElementsClickInfo) {
      Log.d(Const.LOG_TAG, "Vector element click!");
      
      // Remove old click label
      if (oldClickLabel != null) {
        vectorDataSource.remove(oldClickLabel);
        oldClickLabel = null;
      }
      
      // Multiple vector elements can be clicked at the same time, we only care about the one
      // closest to the camera
      VectorElementClickInfo clickInfo = vectorElementsClickInfo.getVectorElementClickInfos().get(0);
      
      // Check the type of vector element
      BalloonPopup clickPopup = null;
      BalloonPopupStyleBuilder styleBuilder = new BalloonPopupStyleBuilder();
        // Configure style
        styleBuilder.setLeftMargins(new BalloonPopupMargins(0, 0, 0, 0));
        styleBuilder.setTitleMargins(new BalloonPopupMargins(6, 3, 6, 3));
        // Make sure this label is shown on top all other labels
        styleBuilder.setPlacementPriority(10);

      VectorElement vectorElement = clickInfo.getVectorElement();
      String clickText = vectorElement.getMetaDataElement("ClickText");
      if (clickText == null || clickText.length() == 0) {
        return;
      }

      if (vectorElement instanceof Billboard) {
        // If the element is billboard, attach the click label to the billboard element
        Billboard billboard = (Billboard) vectorElement;
        clickPopup = new BalloonPopup(billboard, 
                        styleBuilder.buildStyle(),
                                  clickText, 
                                  "");
      } else {
        // for lines and polygons set label to click location
        clickPopup = new BalloonPopup(clickInfo.getElementClickPos(),
                        styleBuilder.buildStyle(),
                                  clickText,
                                  "");
      }
      vectorDataSource.add(clickPopup);
      oldClickLabel = clickPopup;
    }
  }

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item">
  {% highlight c# %}

  public class MapListener : MapEventListener
  {
    private LocalVectorDataSource _dataSource;
    private BalloonPopup _oldClickLabel;

    public MapListener(LocalVectorDataSource dataSource)
    {
      _dataSource = dataSource;
    }

    public override void OnMapClicked (MapClickInfo mapClickInfo)
    {
      // Remove old click label
      if (_oldClickLabel != null) {
        _dataSource.Remove(_oldClickLabel);
        _oldClickLabel = null;
      }
    }

    public override void OnMapMoved()
    {
    }

    public override void OnVectorElementClicked(VectorElementsClickInfo vectorElementsClickInfo)
    {
      // A note about iOS: DISABLE 'Optimize PNG files for iOS' option in iOS build settings,
      // otherwise icons can not be loaded using AssetUtils/Bitmap constructor as Xamarin converts
      // PNGs to unsupported custom format.

      // Remove old click label
      if (_oldClickLabel != null) {
        _dataSource.Remove(_oldClickLabel);
        _oldClickLabel = null;
      }

      var clickInfo = vectorElementsClickInfo.VectorElementClickInfos[0];

      var styleBuilder = new BalloonPopupStyleBuilder();
      // Configure simple style
      styleBuilder.LeftMargins = new BalloonPopupMargins (0, 3, 0, 6);
      styleBuilder.RightMargins = new BalloonPopupMargins (0, 3, 0, 6);

      // Make sure this label is shown on top all other labels
      styleBuilder.PlacementPriority = 10;

      var vectorElement = clickInfo.VectorElement;
      var clickText = vectorElement.GetMetaDataElement("ClickText");

      var clickPopup = new BalloonPopup(clickInfo.ElementClickPos, 
        styleBuilder.BuildStyle(),
        clickText, 
        "");

      _dataSource.Add(clickPopup);
      _oldClickLabel = clickPopup;

    }
  }

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item">
  {% highlight objc %}

  #import &lt;CartoMobileSDK/CartoMobileSDK.h&lt;

  /*
   * A custom map event listener that displays information about map events and creates pop-ups.
   */
  @interface  MyMapEventListener : NTMapEventListener

  -(void)setMapView:(NTMapView*)mapView vectorDataSource:(NTLocalVectorDataSource*)vectorDataSource;
  -(void)onMapMoved;
  -(void)onMapClicked:(NTMapClickInfo*)mapClickInfo;

  @end

  // MyMapEventListener.mm:

  #import "MyMapEventListener.h"

  @interface  MyMapEventListener() {
  }
  @end;

  @implementation MyMapEventListener

  -(void)onMapMoved
  {
   // called very often, even just console logging can lag map movement animation
   // NSLog(@"Map moved!");
  }

  -(void)onMapClicked:(NTMapClickInfo*)mapClickInfo
  {
      // Check the type of the click
      NSString* clickMsg;
      if ([mapClickInfo getClickType] == NT_CLICK_TYPE_SINGLE)
      {
          clickMsg = @"Single map click!";
      }
      else if ([mapClickInfo getClickType] == NT_CLICK_TYPE_LONG)
      {
          clickMsg = @"Long map click!";
      }
      else if ([mapClickInfo getClickType] == NT_CLICK_TYPE_DOUBLE)
      {
          clickMsg = @"Double map click!";
      }
      else if ([mapClickInfo getClickType] == NT_CLICK_TYPE_DUAL)
      {
          clickMsg = @"Dual map click!";
      }
      
      NTMapPos* clickPos = [mapClickInfo getClickPos];
      NTMapPos* wgs84Clickpos = [[[_mapView getOptions] getBaseProjection] toWgs84:clickPos];
      NSLog(@"%@ Location: %@", clickMsg, [NSString stringWithFormat:@"%f, %f", [wgs84Clickpos getY], [wgs84Clickpos getX]]);
  }

  -(void)onVectorElementClicked:(NTVectorElementsClickInfo*)vectorElementsClickInfo
  {
      NSLog(@"Vector element click!");
      
      // Multiple vector elements can be clicked at the same time, we only care about the one
      // closest to the camera
      NTVectorElementClickInfo* clickInfo = [[vectorElementsClickInfo getVectorElementClickInfos] get:0];    
      
      // Load metadata from the object
      NTVectorElement* vectorElement = [clickInfo getVectorElement];
    NSString* clickText = [vectorElement getMetaDataElement:@"ClickText"];
    if (clickText == nil || [clickText length] == 0) {
      return;
    }
    
    NSLog(@"Vector element clicked, metadata : '%@'", clickText);
  }

  @end

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item">
  {% highlight swift %}COMING SOON...

  {% endhighlight %}
  </div>
  
</div>

### Initialize Listener

Apply the following code to initialize listener events for map clicks.

<div class="js-TabPanes">
  <ul class="Tabs">
    <li class="Tab js-Tabpanes-navItem is-active">
      <a href="#/0" class="js-Tabpanes-navLink">Java</a>
    </li>
    <li class="Tab js-Tabpanes-navItem">
      <a href="#/1" class="js-Tabpanes-navLink">C#</a>
    </li>
    <li class="Tab js-Tabpanes-navItem">
      <a href="#/2" class="js-Tabpanes-navLink">Objective-C</a>
    </li>
    <li class="Tab js-Tabpanes-navItem">
      <a href="#/3" class="js-Tabpanes-navLink">Swift</a>
    </li>
  </ul>

  <div class="Carousel-item js-Tabpanes-item is-active">
  {% highlight java %}mapView.setMapEventListener(new MyMapEventListener(mapView, vectorDataSource));

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item">
  {% highlight c# %}MapView.MapEventListener = new MapListener (dataSource);

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item">
  {% highlight objc %}MyMapEventListener* mapListener = [[MyMapEventListener alloc] init];
  [self setMapEventListener:mapListener];

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item">
  {% highlight swift %}COMING SOON...

  {% endhighlight %}
  </div>
  
</div>

### Show/Hide Labels for Map Clicks

MapEventListener contains special functionality that shows, or hides, a small label text over a clicked object. The following label click event rules apply:

- If you click on an object, it creates another map object as a **BalloonPopup**. This contains the text from the metadata of the clicked object

  For this reason, our samples apps include a _Metadata field value_. You can use the metadata value directly, or use the **object unique ID** as the metadata, to query details about the clicked object from the database.

- When a viewer clicks a new location on the map (or click another map object), the original BalloonPopup is deleted, and new one appears at the click location

- The BalloonPopup is added to the DataSource, which is linked existing map layers. The same DataSource is used for other vector elements on a map

## Clusters

When you have too many Points of Interest (POIs) that cannot be displayed as individual markers, it results in a cluttered map. It also effects the rendering performance of your map, and may even result in running out of memory. As an alternative, replace individual objects with *clusters*.

Clusters are markers which display a location of several objects, and typically indicate the number of markers within each object.

![Cluster sample](http://share.gifyoutube.com/vMPDzX.gif)

CARTO Mobile SDK has built-in cluster feature, which is highly customizable. You can define the following options in your app code:

- Styling the cluster objects
- Dynamically generate cluster object styles. For example, automatically display the number of objects in each cluster
- Define the minimum zoom level for clusters
- Set the minimum distance between objects, before it becomes a cluster
- Indicate the action when clicking on marker. For example, zoom in, or expand the cluster without zooming

  **Tip:** The cluster expand feature is useful for small clusters (containing up to four objects inside)

### API Methods for Clusters

Clusters are generated dynamically, based on `VectorDataSource` data that loads the map layer. If using an API, it works as a unique layer with the `ClusteredVectorLayer` method, and includes the following parameters in the a hierarchal order:

1. Select the layer `DataSource`

    In most cases,  the `LocalVectorDataSource` function contains all the elements to request the data. It is important that the DataSource displays all elements in a layer, and does not limit it to the current map visualization bbox (bounding box)

2. `ClusterElementBuilder` defines a single method `buildClusterElement`

### Define Cluster Data and Layer

<div class="js-TabPanes">
  <ul class="Tabs">
    <li class="Tab js-Tabpanes-navItem is-active">
      <a href="#/0" class="js-Tabpanes-navLink">Java</a>
    </li>
    <li class="Tab js-Tabpanes-navItem">
      <a href="#/1" class="js-Tabpanes-navLink">C#</a>
    </li>
    <li class="Tab js-Tabpanes-navItem">
      <a href="#/2" class="js-Tabpanes-navLink">Objective-C</a>
    </li>
    <li class="Tab js-Tabpanes-navItem">
      <a href="#/3" class="js-Tabpanes-navLink">Swift</a>
    </li>
  </ul>

  <div class="Carousel-item js-Tabpanes-item is-active">
  {% highlight java %}

// 1. Initialize a local vector data source
      LocalVectorDataSource vectorDataSource1 = new LocalVectorDataSource(baseProjection);

// 2. Create Marker objects and add them to vectorDataSource
      // **Note:** This depends on the _app type_ of your mobile app settings. See AdvancedMap for samples with JSON loading and random point generation

// 3. Initialize a vector layer with the previous data source
      ClusteredVectorLayer vectorLayer1 = new ClusteredVectorLayer(vectorDataSource1, new MyClusterElementBuilder(this.getApplication()));
        vectorLayer1.setMinimumClusterDistance(20);

// 4. Add the previous vector layer to the map
      mapView.getLayers().add(vectorLayer1);

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item">
  {% highlight c# %}var proj = new EPSG3857();

// 1. Create overlay layer for markers
      var dataSource = new LocalVectorDataSource(proj);

// 2. Create Marker objects and add them to vectorDataSource.
      // **Note:** This depends on the _app type_ of your mobile app settings. See samples with JSON loading

      var layer = new ClusteredVectorLayer(dataSource, new MyClusterElementBuilder());
      layer.MinimumClusterDistance = 20; // in pixels

      MapView.Layers.Add(layer);

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item">
  {% highlight objc %}

// 1. Initialize a local vector data source
      NTProjection* proj = [[mapView getOptions] getBaseProjection];
      NTLocalVectorDataSource* vectorDataSource = [[NTLocalVectorDataSource alloc] initWithProjection:proj];
    
// 2. Create Marker objects and add them to vectorDataSource.
      // **Note:** This depends on the _app type_ of your mobile app settings. See AdvancedMap for samples with JSON loading and random point generation
    
// 3. Create element builder
      MyMarkerClusterElementBuilder* clusterElementBuilder = [[MyMarkerClusterElementBuilder alloc] init];
    
// 4. Initialize a vector layer with the previous data source
      NTClusteredVectorLayer* vectorLayer = [[NTClusteredVectorLayer alloc] initWithDataSource:vectorDataSource clusterElementBuilder:clusterElementBuilder];
    
// 5. Add the previous vector layer to the map
      [[mapView getLayers] add:vectorLayer];

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item">
  {% highlight swift %}COMING SOON...

  {% endhighlight %}
  </div>
  
</div>

### Define ClusterElementBuilder

The Cluster Element Builder takes set of original markers (map objects) as input, and returns one object (or another `VectorElement`, such as a Point or BalloonPopup) which dynamically replaces the original marker.

**Note:** It is highly recommended to reuse and cache styles to reduce memory usage. For example, a marker style with a specific number is only created once. Android and iOS samples use platform-specific graphic APIs to generate the bitmap for the marker. .NET example only uses BalloonPopup, which is slower but works the same across all platforms.

<div class="js-TabPanes">
  <ul class="Tabs">
    <li class="Tab js-Tabpanes-navItem is-active">
      <a href="#/0" class="js-Tabpanes-navLink">Java</a>
    </li>
    <li class="Tab js-Tabpanes-navItem">
      <a href="#/1" class="js-Tabpanes-navLink">C#</a>
    </li>
    <li class="Tab js-Tabpanes-navItem">
      <a href="#/2" class="js-Tabpanes-navLink">Objective-C</a>
    </li>
    <li class="Tab js-Tabpanes-navItem">
      <a href="#/3" class="js-Tabpanes-navLink">Swift</a>
    </li>
  </ul>

  <div class="Carousel-item js-Tabpanes-item is-active">
  {% highlight java %}private class MyClusterElementBuilder extends ClusterElementBuilder {

        @SuppressLint("UseSparseArrays")
        private Map< Integer, MarkerStyle > markerStyles = new HashMap< Integer, MarkerStyle >();
        private android.graphics.Bitmap markerBitmap;

        MyClusterElementBuilder(Application context) {
            markerBitmap = android.graphics.Bitmap.createBitmap(BitmapFactory.decodeResource(context.getResources(), R.drawable.marker_black));
        }

        @Override
        public VectorElement buildClusterElement(MapPos pos, VectorElementVector elements) {
            // 1. Reuse existing marker styles
            MarkerStyle style = markerStyles.get((int) elements.size());
            
            if (elements.size() == 1) {
                style = ((Marker) elements.get(0)).getStyle();
            }

            if (style == null) {
                android.graphics.Bitmap canvasBitmap = markerBitmap.copy(android.graphics.Bitmap.Config.ARGB_8888, true);
                android.graphics.Canvas canvas = new android.graphics.Canvas(canvasBitmap);
                android.graphics.Paint paint = new android.graphics.Paint(android.graphics.Paint.ANTI_ALIAS_FLAG);
                
                paint.setTextAlign(Paint.Align.CENTER);
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

            // 2. Create marker for the cluster
            Marker marker = new Marker(pos, style);
            return marker;
        }
    }

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item">
  {% highlight c# %}public class MyClusterElementBuilder : ClusterElementBuilder
  {
    BalloonPopupStyleBuilder balloonPopupStyleBuilder;

    public MyClusterElementBuilder()
    {
      balloonPopupStyleBuilder = new BalloonPopupStyleBuilder();
      balloonPopupStyleBuilder.CornerRadius = 3;
      balloonPopupStyleBuilder.TitleMargins = new BalloonPopupMargins(6, 6, 6, 6);
      balloonPopupStyleBuilder.LeftColor = new Color(240, 230, 140, 255);
    }

    public override VectorElement BuildClusterElement(MapPos pos, VectorElementVector elements)
    {
      BalloonPopupStyle style = balloonPopupStyleBuilder.BuildStyle();
      var popup = new BalloonPopup(pos, style, elements.Count.ToString(), "");

      return popup;
    }
  }

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item">
  {% highlight objc %}

    // .h

    @interface MyMarkerClusterElementBuilder : NTClusterElementBuilder

  @property NSMutableDictionary* markerStyles;

  @end
  
  // .m

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
          
          [markerStyleBuilder setPlacementPriority:-(int)[elements size]];
          markerStyle = [markerStyleBuilder buildStyle];
          
          [self.markerStyles setValue:markerStyle forKey:styleKey];
      }
      
      NTMarker* marker = [[NTMarker alloc] initWithPos:mapPos style:markerStyle];
      
      NTVariant* variant = [[NTVariant alloc] initWithString:[@([elements size]) stringValue]];
      [marker setMetaDataElement:@"elements" element:variant];
      
      return marker;
  }

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item">
  {% highlight swift %}COMING SOON...

  {% endhighlight %}
  </div>
  
</div>

## Ground Overlays

Ground overlays projects a bitmap (PNG, JPG) image of a defined coordinate over a basemap. For example, a ground overlay bitmap may be used to show an indoor floorplan over a building map.

<img src = "/images/ground_overlay.png" alt = "Sample bitmap" width="550">

**Note:** Your bitmap must define **geographical map coordinates** with `Ground Control Points`. Currently, Mobile SDK supports three or four points on a bitmap image.

### Ground Overlay Requirements

The following requirements allow you to ground overlays with the Mobile SDK.

- `linear affine transformation` enables you to set three control points to set the location, size, and rotation of the bitmap

- `perspective transformation` enables you to control four control points for the bitmap

-  If you have more control points in your data, it is suggested to choose three or four of your best ones and select those as your ground control settings

- Control points must be defined in the app code. Mobile SDK does not automatically gather control points from the source file metadata

  For example, if you need GeoTIFF, ESRI World File, MapInfo TAB, Geospatial PDF, or other referenced data, you will receive the following you will get this from GIS Extension (see below).

- The **entire bitmap must fit to device memory** (RAM). Depending on the target device, the maximum size could vary. For example, the target device maximum size might be 2000x2000 pixels. 

  **Tip:** For larger rasters, the SDK Extensions allow you to display any size bitmap, up to hundreds of megabytes. These bitmaps are read directly from common GIS raster formats (such as GeoTIFF, BSB, ECW, MrSID, JPEG2000, and so on). Additionally, the source data can be entered using different coordinate systems. Please [contact us](mailto:sales@carto.com) if you are interested in enabling SDK Extensions as part of your account.

### Code Samples

This example uses only one geographical coordinate. The building size is known, and the building direction is facing north. This allows us to calculate other ground points with the code. Four ground control points are set to the corners of the bitmap, which typically returns the most accurate result. 

**Tip:** The following sample code assumes that you have the **jefferson-building-ground-floor.jpg** bitmap file as part of your application project.

- For Android, this image is located under *assets*
- In iOS, it can be located anywhere in your project

<div class="js-TabPanes">
  <ul class="Tabs">
    <li class="Tab js-Tabpanes-navItem is-active">
      <a href="#/0" class="js-Tabpanes-navLink">Java</a>
    </li>
    <li class="Tab js-Tabpanes-navItem">
      <a href="#/1" class="js-Tabpanes-navLink">C#</a>
    </li>
    <li class="Tab js-Tabpanes-navItem">
      <a href="#/2" class="js-Tabpanes-navLink">Objective-C</a>
    </li>
    <li class="Tab js-Tabpanes-navItem">
      <a href="#/3" class="js-Tabpanes-navLink">Swift</a>
    </li>
  </ul>

  <div class="Carousel-item js-Tabpanes-item is-active">
  {% highlight java %}com.carto.graphics.Bitmap overlayBitmap = BitmapUtils.loadBitmapFromAssets("jefferson-building-ground-floor.jpg");

// 1. Create two vector containing geographical positions and corresponding raster image pixel coordinates
    MapPos pos = proj.fromWgs84(new MapPos(-77.004590, 38.888702));
    double sizeNS = 110, sizeWE = 100;

    MapPosVector mapPoses = new MapPosVector();
    mapPoses.add(new MapPos(pos.getX()-sizeWE, pos.getY()+sizeNS));
    mapPoses.add(new MapPos(pos.getX()+sizeWE, pos.getY()+sizeNS));
    mapPoses.add(new MapPos(pos.getX()+sizeWE, pos.getY()-sizeNS));
    mapPoses.add(new MapPos(pos.getX()-sizeWE, pos.getY()-sizeNS));

    ScreenPosVector bitmapPoses = new ScreenPosVector();
    bitmapPoses.add(new ScreenPos(0, 0));
    bitmapPoses.add(new ScreenPos(0, overlayBitmap.getHeight()));
    bitmapPoses.add(new ScreenPos(overlayBitmap.getWidth(), overlayBitmap.getHeight()));
    bitmapPoses.add(new ScreenPos(overlayBitmap.getWidth(), 0));

// 2. Create bitmap overlay raster tile data source
    BitmapOverlayRasterTileDataSource rasterDataSource = new BitmapOverlayRasterTileDataSource(0, 20, overlayBitmap, proj, mapPoses, bitmapPoses);
    RasterTileLayer rasterLayer = new RasterTileLayer(rasterDataSource);
    mapView.getLayers().add(rasterLayer);

// 3. Apply zoom level bias to the raster layer
    // - By default, bitmaps are upsampled on high-DPI screens
// 4. Correct this by applying appropriate bias
    float zoomLevelBias = (float) (Math.log(mapView.getOptions().getDPI() / 160.0f) / Math.log(2));
    rasterLayer.setZoomLevelBias(zoomLevelBias * 0.75f);
    rasterLayer.setTileSubstitutionPolicy(TileSubstitutionPolicy.TILE_SUBSTITUTION_POLICY_VISIBLE);

    mapView.setFocusPos(pos, 0);
    mapView.setZoom(16f, 0);

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item">
  {% highlight c# %}var overlayBitmap = BitmapUtils.LoadBitmapFromAssets("jefferson-building-ground-floor.jpg");

// 1. Create two vector geographical positions, and corresponding raster image pixel coordinates
      var pos = proj.FromWgs84(new MapPos(-77.004590, 38.888702));
      var sizeNS = 110;
      var sizeWE = 100;

      var mapPoses = new MapPosVector();
      mapPoses.Add(new MapPos(pos.X - sizeWE, pos.Y + sizeNS));
      mapPoses.Add(new MapPos(pos.X + sizeWE, pos.Y + sizeNS));
      mapPoses.Add(new MapPos(pos.X + sizeWE, pos.Y - sizeNS));
      mapPoses.Add(new MapPos(pos.X - sizeWE, pos.Y - sizeNS));

      var bitmapPoses = new ScreenPosVector();
      bitmapPoses.Add(new ScreenPos(0, 0));
      bitmapPoses.Add(new ScreenPos(0, overlayBitmap.Height));
      bitmapPoses.Add(new ScreenPos(overlayBitmap.Width, overlayBitmap.Height));
      bitmapPoses.Add(new ScreenPos(overlayBitmap.Width, 0));

// 2. Create bitmap overlay raster tile data source
      var rasterDataSource = new BitmapOverlayRasterTileDataSource(0, 20, overlayBitmap, proj, mapPoses, bitmapPoses);
      var rasterLayer = new RasterTileLayer(rasterDataSource);
      MapView.Layers.Add(rasterLayer);

// 3. Apply zoom level bias to the raster layer
      // - By default, bitmaps are upsampled on high-DPI screens

// 4. Correct this by applying appropriate bias
      float zoomLevelBias = (float)(Math.Log(MapView.Options.DPI / 160.0f) / Math.Log(2));
      rasterLayer.ZoomLevelBias = zoomLevelBias * 0.75f;
      rasterLayer.TileSubstitutionPolicy = TileSubstitutionPolicy.TileSubstitutionPolicyVisible;

      MapView.SetFocusPos(pos, 0);
      MapView.SetZoom(16f, 0);

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item">
  {% highlight objc %}

// 1. Load ground overlay bitmap
      NTBitmap *overlayBitmap = [NTBitmapUtils loadBitmapFromAssets:@"jefferson-building-ground-floor.jpg"];
    
// 2. Create two vector geographical positions, and corresponding raster image pixel coordinates
      NTMapPos* pos = [proj fromWgs84:[[NTMapPos alloc] initWithX:-77.004590 y:38.888702]];
      double sizeNS = 110, sizeWE = 100;
    
      NTMapPosVector* mapPoses = [[NTMapPosVector alloc] init];
      [mapPoses add:[[NTMapPos alloc] initWithX:[pos getX]-sizeWE y:[pos getY]+sizeNS]];
      [mapPoses add:[[NTMapPos alloc] initWithX:[pos getX]+sizeWE y:[pos getY]+sizeNS]];
      [mapPoses add:[[NTMapPos alloc] initWithX:[pos getX]+sizeWE y:[pos getY]-sizeNS]];
      [mapPoses add:[[NTMapPos alloc] initWithX:[pos getX]-sizeWE y:[pos getY]-sizeNS]];
      
      NTScreenPosVector* bitmapPoses = [[NTScreenPosVector alloc] init];
      [bitmapPoses add:[[NTScreenPos alloc] initWithX:0 y:0]];
      [bitmapPoses add:[[NTScreenPos alloc] initWithX:0 y:[overlayBitmap getHeight]]];
      [bitmapPoses add:[[NTScreenPos alloc] initWithX:[overlayBitmap getWidth] y:[overlayBitmap getHeight]]];
      [bitmapPoses add:[[NTScreenPos alloc] initWithX:[overlayBitmap getWidth] y:0]];
    
// 3. Create bitmap overlay raster tile data source
      NTBitmapOverlayRasterTileDataSource* rasterDataSource = [[NTBitmapOverlayRasterTileDataSource alloc] initWithMinZoom:0 maxZoom:20 bitmap:overlayBitmap projection:proj mapPoses:mapPoses bitmapPoses:bitmapPoses];
      NTRasterTileLayer* rasterLayer = [[NTRasterTileLayer alloc] initWithDataSource:rasterDataSource];
      [[mapView getLayers] add:rasterLayer];
    
// 4. Apply zoom level bias to the raster layer
      // - By default, bitmaps are upsampled on high-DPI screens

// 5. Correct this by applying appropriate bias
      float zoomLevelBias = log([[mapView getOptions] getDPI] / 160.0f) / log(2);
      [rasterLayer setZoomLevelBias:zoomLevelBias * 0.75f];
      [rasterLayer setTileSubstitutionPolicy:NT_TILE_SUBSTITUTION_POLICY_VISIBLE];
      
      [mapView setFocusPos:pos durationSeconds:0];
      [mapView setZoom:16f durationSeconds:0];

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item">
  {% highlight swift %}COMING SOON...

  {% endhighlight %}
  </div>
  
</div>

## Vector Styles

Usage of vector-based base maps enables you to re-style maps according to your needs. You can set the colors, transparency, line styles (width, patterns, casings, endings), polygon patterns, icons, text placements, fonts and many other vector data parameters. 

CARTO uses [Mapnik](http://mapnik.org) XML style description language for customizing the visual style of vector tiles. Our styling is optimized for mobile and contain some unique style parameters, while ignoring some others. In general, you can reuse your existing Mapnik XML, or CartoCSS, styling files and tools (such as TileMill/Mapbox Studio).

Vector styling is applied in the mobile client, where the style files are bundled with the application installer. The application can change the styling anytime, without reloading vector map data. This enables your to download map data once, and change styling from "day mode" to "night mode" with no new downloads.

CARTO map rendering implementation is intended for real-time rendering. As a result, several limitations apply.

### Prepackaged Vector Styles

Prepackaged mobile styles used by CARTO. These are compatible with our vector tile sources.

### Mapnik Style Format

Mapnik style definitions are a common file format for map styles, based on XML. It was originally created for Mapnik, but is used by other software, such as our Mobile SDK. The file format specifications are located in this [XMLConfigReference](https://github.com/mapnik/mapnik/wiki/XMLConfigReference) document. There are several ways you can apply these styles:

 1. Use CARTO provided styles

 2. Modify the `style.xml`, located inside the sample style, with your own edits

 3. Create your own styles using a Mapnik editor tool

    **Note:** It is recommended to use the free MapBox Studio tool, which uses CartoCSS for primary style definitions. While the Mobile SDK does not use CartoCSS styles, you can modify and export Mapnik XML styles to be compatible with the Mobile SDK. 

### Mapnik Limitations

Please note the following limitations with Mapnik style formats.

- There are no built-in fonts, fonts must be explicitly added to the project

- Loading SVG icons is not supported, such icons should be converted to PNG format (Mapnik provides a _svg2png_ utility)

- Original layer ordering is not always preserved, texts are always drawn on top of 2D geometry. 3D buildings are drawn on top of texts

- Layer opacity works per element, not per layer as in Mapnik. For non-overlapping elements, the result will be same. For overlapping elements, there are likely artifacts

- `comp-op` feature is not supported (neither is `layer/symbolizer`)

- `line-join` parameter is ignored, only `miter line join` is used 

- `GroupSymbolizer` and `RasterSymbolizer` are currently not supported

- Text characters are rendered individually. If characters overlap, the halo of one character may cover glyph of another character. The suggested workaround is to increase spacing, or decrease the halo radius

**Note:** The Mobile SDK does not implement 100% of the tags and features of Mapnik. This lists just a few of the Mapnik limitations, there are additional Mapnik XML features that are not standard. If you need some an unimplemented styling option, please [contact us](mailto:support@carto.com). 

### Creating Vector Style Packages

CARTO vector styles are distributed as zip-archives. All style-related files and folders must be placed into a single zip file.

The most important part of the style is the style definition file, typically named _project.xml_. This file contains style descriptions for all layers, and usually references other files, such as fonts, icons, and pattern bitmaps (which should be placed in various subfolders).

### Vector Style Performance 

When using vector styles, the following recommendations are suggested for optimal performance:

- **Multiple symbolizers per layer may have very large performance hits**. If possible, move each symbolizer into separate layer

- `BuildingSymbolizer` requires an expensive OpenGL frame buffer read-back operation, and may perform very poorly on some devices (such as the original iPad Retina)

- To increase performance, it is suggested to use power-of-two dimensions for bitmaps

### SDK Extensions for Mapnik XML style files

The following CARTO specific extensions are specific to Mapnik XML style files.

#### NutiParameters

_NutiParameters_ describe additional parameters that can be used in styles and controlled in the code (from `MBVectorTileDecoder`).

- Parameters are typed, have default values and can be used as variables within _nuti_ namespace in the style (for example, *[nuti::lang]*)

- Some parameters may have _ prefix in their name. Such variables are reserved and should not be updated directly by the application

The following is a simple example of _NutiParameters_ section, located in the style xml file:

`<NutiParameters>`
`  <NutiParameter name="lang" type="string" value="en" />`
`</NutiParameters>`

#### Metavariables

Metavariables add support for dynamic variable look-up. Variable names may depend on other variables. For example, *[name_[nuti::lang]]*.

If the value of *nuti::lang* is 'en', this metavariable expression would be equal to *[name_en]* expression.

#### Conditional Operator

Conditional operator ?: adds support for simple control flow management. It is similar to C language conditional operator and can be used in all expressions. For example:

`[nuti::lang] == 'en' ? 'English' : 'Other'`

#### 3D Texts and Markers

Mapnik `MarkersSymbolizer` and `TextSymbolizer` support additional values _nutibillboard_ for _placement_ parameter. This enables texts and markers to act as billboards (always facing the viewer), even when screen is tilted. This option can be used to give markers and texts more '3D-like' appearance. For example,

`<MarkersSymbolizer placement="nutibillboard" fill="#666666" file="icon/[maki]-12.svg" />`

### Supported Symbolizers and Parameters

The following lists contains all supported symbolizers and parameters for vector styles:

- `PointSymbolizer`: file, opacity, allow-overlap, ignore-placement, transform

- `BuildingSymbolizer`: fill, fill-opacity, height, geometry-transform

- `LineSymbolizer`: stroke, stroke-width, stroke-opacity, stroke-linejoin, stroke-linecap, stroke-dasharray, geometry-transform

- `LinePatternSymbolizer`: file, fill, opacity, geometry-transform

- `PolygonSymbolizer`: fill, fill-opacity, geometry-transform

- `PolygonPatternSymbolizer`: file, fill, opacity, geometry-transform

- `MarkersSymbolizer`: file, placement, marker-type, fill, opacity, width, height, spacing, allow-overlap, ignore-placement, transform

- `TextSymbolizer`: name, face-name, fontset-name, placement, size, spacing, fill, opacity, halo-fill, halo-opacity, halo-radius, allow-overlap, minimum-distance, text-transform, orientation, dx, dy, wrap-width, wrap-before, character-spacing, line-spacing, horizontal-alignment, vertical-alignment

- `ShieldSymbolizer`: name, face-name, fontset-name, placement, size, spacing, fill, opacity, halo-fill, halo-opacity, halo-radius, allow-overlap, minimum-distance, text-transform, orientation, dx, dy, wrap-width, wrap-before, character-spacing, line-spacing, horizontal-alignment, vertical-alignment, file, shield-dx, shield-dy, unlock-image

### CartoCSS Extensions for Vector Styles

`Metavariables` and `nutiparameters` are also available as CartoCSS style extensions.
