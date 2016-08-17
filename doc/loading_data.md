# Loading Mobile Data

The following DataSources are available for the Mobile SDK, and are supported on all mobile platforms.

**Note:** Some DataSources are more generic and can be used for different data types and map layers (vector or raster). Others can only be applied for specific layer types. Please [contact us](mailto:mobile-support@carto.com) if you have questions about which DataSources are available for your account plan.

## Offline Maps from MBTiles

**MBTilesTileDataSource** is universal datasource for raster, or vector, tile data.

### MBtiles as RasterTileLayer

For raster data, use [MBTiles](https://github.com/mapbox/mbtiles-spec) format files, and use the following code to generate them:

<div class="js-TabPanes">
  <ul class="Tabs">
    <li class="Tab js-Tabpanes-navItem is-active">
      <a href="#/0" class="js-Tabpanes-navLink--lang js-Tabpanes-navLink--lang--java">Java</a>
    </li>
    <li class="Tab js-Tabpanes-navItem">
      <a href="#/1" class="js-Tabpanes-navLink--lang js-Tabpanes-navLink--lang--csharp">C#</a>
    </li>
    <li class="Tab js-Tabpanes-navItem">
      <a href="#/2" class="js-Tabpanes-navLink--lang js-Tabpanes-navLink--lang--objective-c">Objective-C</a>
    </li>
    <li class="Tab js-Tabpanes-navItem">
      <a href="#/3" class="js-Tabpanes-navLink--lang js-Tabpanes-navLink--lang--swift">Swift</a>
    </li>
  </ul>

  <div class="Carousel-item js-Tabpanes-item is-active">
  {% highlight java %}MBTilesTileDataSource tileDataSource = new MBTilesTileDataSource(filePath);
    TileLayer rasterLayer = new RasterTileLayer(tileDataSource);

    mapView.getLayers().add(rasterLayer);

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item">
  {% highlight csharp %}var mbTilesLayer = new RasterTileLayer(new MBTilesTileDataSource(filePath));
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
      <a href="#/0" class="js-Tabpanes-navLink--lang js-Tabpanes-navLink--lang--java">Java</a>
    </li>
    <li class="Tab js-Tabpanes-navItem">
      <a href="#/1" class="js-Tabpanes-navLink--lang js-Tabpanes-navLink--lang--csharp">C#</a>
    </li>
    <li class="Tab js-Tabpanes-navItem">
      <a href="#/2" class="js-Tabpanes-navLink--lang js-Tabpanes-navLink--lang--objective-c">Objective-C</a>
    </li>
    <li class="Tab js-Tabpanes-navItem">
      <a href="#/3" class="js-Tabpanes-navLink--lang js-Tabpanes-navLink--lang--swift">Swift</a>
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
  {% highlight csharp %}

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
      <a href="#/0" class="js-Tabpanes-navLink--lang js-Tabpanes-navLink--lang--java">Java</a>
    </li>
    <li class="Tab js-Tabpanes-navItem">
      <a href="#/1" class="js-Tabpanes-navLink--lang js-Tabpanes-navLink--lang--csharp">C#</a>
    </li>
    <li class="Tab js-Tabpanes-navItem">
      <a href="#/2" class="js-Tabpanes-navLink--lang js-Tabpanes-navLink--lang--objective-c">Objective-C</a>
    </li>
    <li class="Tab js-Tabpanes-navItem">
      <a href="#/3" class="js-Tabpanes-navLink--lang js-Tabpanes-navLink--lang--swift">Swift</a>
    </li>
  </ul>

  <div class="Carousel-item js-Tabpanes-item is-active">
  {% highlight java %}BinaryData styleAsset = AssetUtils.loadAsset("nutibright-v2a.zip");
        VectorTileLayer baseLayer = new CartoOnlineVectorTileLayer("nutiteq.osm", new ZippedAssetPackage(styleAsset));

        mapView.getLayers().add(baseLayer);

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item">
  {% highlight csharp %}var styleAsset = AssetUtils.LoadAsset("nutibright-v2a.zip");
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
      <a href="#/0" class="js-Tabpanes-navLink--lang js-Tabpanes-navLink--lang--java">Java</a>
    </li>
    <li class="Tab js-Tabpanes-navItem">
      <a href="#/1" class="js-Tabpanes-navLink--lang js-Tabpanes-navLink--lang--csharp">C#</a>
    </li>
    <li class="Tab js-Tabpanes-navItem">
      <a href="#/2" class="js-Tabpanes-navLink--lang js-Tabpanes-navLink--lang--objective-c">Objective-C</a>
    </li>
    <li class="Tab js-Tabpanes-navItem">
      <a href="#/3" class="js-Tabpanes-navLink--lang js-Tabpanes-navLink--lang--swift">Swift</a>
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
  {% highlight csharp %}

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
      <a href="#/0" class="js-Tabpanes-navLink--lang js-Tabpanes-navLink--lang--java">Java</a>
    </li>
    <li class="Tab js-Tabpanes-navItem">
      <a href="#/1" class="js-Tabpanes-navLink--lang js-Tabpanes-navLink--lang--csharp">C#</a>
    </li>
    <li class="Tab js-Tabpanes-navItem">
      <a href="#/2" class="js-Tabpanes-navLink--lang js-Tabpanes-navLink--lang--objective-c">Objective-C</a>
    </li>
    <li class="Tab js-Tabpanes-navItem">
      <a href="#/3" class="js-Tabpanes-navLink--lang js-Tabpanes-navLink--lang--swift">Swift</a>
    </li>
  </ul>

  <div class="Carousel-item js-Tabpanes-item is-active">
  {% highlight java %}TileDataSource baseRasterTileDataSource = new HTTPTileDataSource(1, 19, "http://ecn.t3.tiles.virtualearth.net/tiles/a{quadkey}.jpeg?g=471&mkt=en-US");

        TileLayer baseLayer = new RasterTileLayer(baseRasterTileDataSource);

        mapView.getLayers().add(baseLayer);

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item">
  {% highlight csharp %}

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
      <a href="#/0" class="js-Tabpanes-navLink--lang js-Tabpanes-navLink--lang--java">Java</a>
    </li>
    <li class="Tab js-Tabpanes-navItem">
      <a href="#/1" class="js-Tabpanes-navLink--lang js-Tabpanes-navLink--lang--csharp">C#</a>
    </li>
    <li class="Tab js-Tabpanes-navItem">
      <a href="#/2" class="js-Tabpanes-navLink--lang js-Tabpanes-navLink--lang--objective-c">Objective-C</a>
    </li>
    <li class="Tab js-Tabpanes-navItem">
      <a href="#/3" class="js-Tabpanes-navLink--lang js-Tabpanes-navLink--lang--swift">Swift</a>
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
  {% highlight csharp %}

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
      <a href="#/0" class="js-Tabpanes-navLink--lang js-Tabpanes-navLink--lang--java">Java</a>
    </li>
    <li class="Tab js-Tabpanes-navItem">
      <a href="#/1" class="js-Tabpanes-navLink--lang js-Tabpanes-navLink--lang--csharp">C#</a>
    </li>
    <li class="Tab js-Tabpanes-navItem">
      <a href="#/2" class="js-Tabpanes-navLink--lang js-Tabpanes-navLink--lang--objective-c">Objective-C</a>
    </li>
    <li class="Tab js-Tabpanes-navItem">
      <a href="#/3" class="js-Tabpanes-navLink--lang js-Tabpanes-navLink--lang--swift">Swift</a>
    </li>
  </ul>

  <div class="Carousel-item js-Tabpanes-item is-active">
  {% highlight java %}String url = "http://your-url-with-placeholders-see-below";
      TileDataSource tileDataSource = new HTTPTileDataSource(0, 18, url);
      RasterTileLayer layer = new RasterTileLayer(tileDataSource);

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item">
  {% highlight csharp %}string url = "http://your-url-with-placeholders-see-below";
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

{% comment %}
Mapbox Raster Tiles

[`http://api.mapbox.com/v4/{mapid}/{zoom}/{x}/{y}.png?access_token=&lt;your access token&gt;`](http://www.mapbox.com)

<img src="http://api.tiles.mapbox.com/v3/nutiteq.map-f0sfyluv/17/65490/43588.png"/>

See <a href=" https://www.mapbox.com/developers/api/maps/">MapBox API spec</a> for details about how to create access tokens. <a href='http://mapbox.com/about/maps' target='_blank'>MapBox Attributions</a>. 
{% endcomment %}

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
