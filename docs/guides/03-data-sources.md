## Data Sources

Maps in CARTO Mobile SDK are displayed as list of layers. Each map layer is 
connected to a **DataSource**, which defines the data the layer displays.

The following DataSources are available for the Mobile SDK, and are supported on all mobile platforms.

**Note:** Some DataSources are more generic and can be used for different data types and map layers (vector or raster). Others can only be applied for specific layer types. Please [contact us](mailto:mobile-support@carto.com) if you have questions about which DataSources are available for your account plan.

### Offline Maps from custom MBTiles

**MBTilesTileDataSource** is universal datasource for raster, or vector, tile data.

#### MBTiles as RasterTileLayer

For raster data, use [MBTiles](https://github.com/mapbox/mbtiles-spec) format files, and use the following code to generate them:

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

    MBTilesTileDataSource tileDataSource = new MBTilesTileDataSource("MBTILES_FILENAME");
    RasterTileLayer mbTilesLayer = new RasterTileLayer(tileDataSource);
    
    mapView.getLayers().add(mbTilesLayer);

    {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--csharp">
    {% highlight csharp %}
  
    var tileDataSource = new MBTilesTileDataSource(filePath);
    var mbTilesLayer = new RasterTileLayer(tileDataSource);

    MapView.Layers.Add(mbTilesLayer);

    {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--objective-c">
    {% highlight objc %}
  
    NTTileDataSource* tileDataSource = [[NTMBTilesTileDataSource alloc] initWithPath: @"MBTILES_FILENAME"];
    NTRasterTileLayer* mbTilesLayer = [[NTRasterTileLayer alloc] initWithDataSource:tileDataSource];
    
    [[mapView getLayers] add:mbTilesLayer];

    {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--swift">
    {% highlight swift %}
  
    let tileDataSource = NTMBTilesTileDataSource(path: "MBTILES_FILENAME")
    let mbTilesLayer = NTRasterTileLayer(dataSource: tileDataSource)
        
    mapView?.getLayers()?.add(mbTilesLayer)

    {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--kotlin">
    {% highlight swift %}
  
    val tileDataSource = MBTilesTileDataSource("MBTILES_FILENAME")
    val mbTilesLayer = RasterTileLayer(tileDataSource)
    
    mapView?.layers?.add(mbTilesLayer)

    {% endhighlight %}
  </div>
    
</div>

#### MBTiles as VectorTileLayer

For packaged vector data, you will need CARTO specific vector files packages (NTVT - *NutiTeq Vector Tile*) and styling files must be in _Mapnik XML_ format. Download the following, free sample package using OpenStreetMap data:

-   [estonia\_ntvt.mbtiles](https://dl.dropboxusercontent.com/u/3573333/public_web/ntvt_packages/estonia_ntvt.mbtiles)

The Mobile SDK provides a built-in download service thorough the **Package Manager** to retrieve map packages for a country, or smaller region. For details, see [Package Manager](/docs/carto-engine/mobile-sdk/05-package-manager/) section.

**Note:** Vector maps always need proper **style definitions**. Fortunately, we bundle three basic styles with the SDK (bright, default, gray).
These styles assume *OpenMapTiles* data structure are not generally applicable to other data structures (*Mapbox*, for example).

Use the following code to use MBTiles for a vector layer: 

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

    MBTilesTileDataSource tileDataSource = new MBTilesTileDataSource("estonia_ntvt.mbtiles");

    // Create tile decoder based on Voyager style and VectorTileLayer
    VectorTileDecoder tileDecoder = CartoVectorTileLayer.createTileDecoder(CartoBaseMapStyle.CARTO_BASEMAP_STYLE_VOYAGER);
    VectorTileLayer offlineLayer = new VectorTileLayer(tileDataSource, tileDecoder);

    mapView.getLayers().add(offlineLayer);

    {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--csharp">
    {% highlight csharp %}

    var tileDataSource = new MBTilesTileDataSource("estonia_ntvt.mbtiles");

    // Create tile decoder based on Voyager style and VectorTileLayer
    var tileDecoder = CartoVectorTileLayer.CreateTileDecoder(CartoBaseMapStyle.CartoBasemapStyleVoyager);
    var offlineLayer = new VectorTileLayer(tileDataSource, tileDecoder);

    mapView.Layers.Add(offlineLayer);

    {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--objective-c">
    {% highlight objc %}
  
    NSString* path = [[NSBundle mainBundle] pathForResource:@"estonia_ntvt" ofType:@"mbtiles"];
    NTMBTilesTileDataSource* tileDataSource = [[NTMBTilesTileDataSource alloc] initWithPath:path];
    
    // Create tile decoder based on Voyager style and VectorTileLayer
    NTMBVectorTileDecoder* tileDecoder = [NTCartoVectorTileLayer createTileDecoder: NT_CARTO_BASEMAP_STYLE_VOYAGER];
    NTVectorTileLayer* offlineLayer = [[NTVectorTileLayer alloc] initWithDataSource:tileDataSource decoder:tileDecoder];
    
    [[mapView getLayers] add:offlineLayer];

    {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--swift">
    {% highlight swift %}

    let tileDataSource = NTMBTilesTileDataSource(path: Bundle.main.path(forResource: "estonia_ntvt", ofType: "mbtiles"))
    
    // Create tile decoder based on Voyager style and VectorTileLayer
    let tileDecoder = NTCartoVectorTileLayer.createTileDecoder(CartoBaseMapStyle.CARTO_BASEMAP_STYLE_VOYAGER)
    let offlineLayer = NTVectorTileLayer(tileDataSource, tileDecoder)
    
    mapView?.layers?.add(offlineLayer)

    {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--kotlin">
    {% highlight kotlin %}
  
    val tileDataSource = MBTilesTileDataSource("estonia_ntvt.mbtiles")
    
    // Create tile decoder based on Voyager style and VectorTileLayer
    val tileDecoder = CartoVectorTileLayer.createTileDecoder(CartoBaseMapStyle.CARTO_BASEMAP_STYLE_VOYAGER)
    val offlineLayer = VectorTileLayer(tileDataSource, tileDecoder)

    mapView?.layers?.add(offlineLayer)

    {% endhighlight %}
  </div>
    
</div>

#### Tools for Raster Maps

The following tools enable you to create MBTiles packages:

- [MapTiler](http://www.maptiler.com/) is a utility to create MBTiles from raster geo files (GeoTIFF, JPG, ECW, and so on)

- [TileMill](http://mapbox.com/tilemill/) is an open source generator of map packages for vector geo files, such as Shapefile or PosgGIS geo data

- [MOBAC](http://mobac.sourceforge.net) is available to download from variety of free sources, such as Bing, OpenStreetMap, and so on. You can even load it from WMS with added configuration

- [MBUtil](https://github.com/mapbox/mbutil) enables you to create mbtiles from/to TMS-style tile folders, created with different utilities, such as GDAL utility

- [Portable Basemap Server](https://geopbs.codeplex.com/) is a free utility for Windows and loads data from various commercial servers and custom sources. It is also available in ESRI formats. It works mainly as WMTS server, but can create MBTiles as an extra feature

#### Tools for Vector Maps

If you need to create vector map files **from different sources, or with your own data** (e.g. Shapefiles), please contact [CARTO support](mailto:carto@support.com)._ Our Mobile Developers will provide you with a free converter toolchain for custom conversions.

### Online Maps

**HTTPTileDataSource** is a general datasource that can be used for vector or raster tiles. You can also use CARTO or Web (HTTP) tiles for vector layers. Additionally, you can use Web (HTTP) raster tiles for map layers. 

#### CARTO Online Vector Tile Layer

The Mobile SDK contains built-in vector tiles, as provided by CARTO maps as a service, and is available for all account plans. This is useful for basic maps.

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
  
    CartoOnlineVectorTileLayer layer = new CartoOnlineVectorTileLayer(CartoBaseMapStyle.CARTO_BASEMAP_STYLE_DEFAULT);
    mapView.getLayers().add(layer);

    {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--csharp">
    {% highlight csharp %}
  
    var layer = new CartoOnlineVectorTileLayer(CartoBaseMapStyle.CartoBasemapStyleDefault);
    MapView.Layers.Add(layer);

    {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--objective-c">
    {% highlight objc %}
  
    NTCartoOnlineVectorTileLayer* layer = [[NTCartoOnlineVectorTileLayer alloc] initWithStyle:NT_CARTO_BASEMAP_STYLE_DEFAULT];
    [[self.mapView getLayers] add:layer];
  
    {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--swift">
    {% highlight swift %}

    let layer = NTCartoOnlineVectorTileLayer(style: NTCartoBaseMapStyle.CARTO_BASEMAP_STYLE_DEFAULT)
    mapView?.getLayers()?.add(layer)

    {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--kotlin">
    {% highlight kotlin %}

    val layer = CartoOnlineVectorTileLayer(CartoBaseMapStyle.CARTO_BASEMAP_STYLE_DEFAULT)
    mapView?.layers?.add(layer)

    {% endhighlight %}
  </div>
    
</div>

#### Web (HTTP) Tiles as VectorTileLayer

Some Mobile SDK plans enable you to use other vector tile map DataSources. These are similar to custom raster map data sources, but a vector decoder is needed for correct implementation.

- Connect to Mapbox vector tiles, which have a different data structure. You need your own StyleSet asset.

  **Note:** Before using these assets in your app, confirm with Mapbox if this is allowed and make sure you use your Mapbox license key in the URL. You may need to use your custom stylying and vector tile server URL.

The following tags are supported in the URL definition: **zoom, x, y, xflipped, yflipped, quadkey**.

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

    // 1. Define data source
    TileDataSource tileDataSource = new HTTPTileDataSource(0, 14, "http://a.tiles.mapbox.com/v4/mapbox.mapbox-streets-v5/{zoom}/{x}/{y}.vector.pbf?access_token=YOUR-MAPBOX-TOKEN");

    // 2. Load vector tile style set
    BinaryData styleBytes = AssetUtils.loadAsset("CUSTOM_STYLESET.zip");
    CompiledStyleSet vectorTileStyleSet = new CompiledStyleSet(new ZippedAssetPackage(styleBytes));

    // 3. Create vector tile decoder using the style set
    VectorTileDecoder vectorTileDecoder = new MBVectorTileDecoder(vectorTileStyleSet);

    // 4. Create vector tile layer, using previously created data source and decoder
    TileLayer vectorTileLayer = new VectorTileLayer(tileDataSource, vectorTileDecoder);

    // 5. Add vector tile layer
    mapView.getLayers().add(vectorTileLayer);

    {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--csharp">
    {% highlight csharp %}

    // 1. Define data source
    var tileDataSource = new HTTPTileDataSource(0, 14, "http://a.tiles.mapbox.com/v4/mapbox.mapbox-streets-v5/{zoom}/{x}/{y}.vector.pbf?access_token=YOUR-MAPBOX-TOKEN");

    // 2. Load vector tile style set
    var styleBytes = AssetUtils.LoadAsset("CUSTOM_STYLESET.zip");
    var vectorTileStyleSet = new CompiledStyleSet(new ZippedAssetPackage(styleBytes));

    // 3. Create vector tile decoder using the style set
    var vectorTileDecoder = new MBVectorTileDecoder(vectorTileStyleSet);

    // 4. Create vector tile layer, using previously created data source and decoder
    var vectorTileLayer = new VectorTileLayer(tileDataSource, vectorTileDecoder);

    // 5. Add vector tile layer
    MapView.Layers.Add(vectorTileLayer);

    {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--objective-c">
    {% highlight objc %}

    // 1. Define data source
    NTTileDataSource* tileDataSource = [[NTHTTPTileDataSource alloc] initWithMinZoom:0 maxZoom:14 baseURL:@"http://a.tiles.mapbox.com/v4/mapbox.mapbox-streets-v5/{zoom}/{x}/{y}.vector.pbf?access_token=YOUR-MAPBOX-TOKEN"];

    // 2. Load vector tile style set
    NTBinaryData* styleBytes = [NTAssetUtils loadAsset: @"CUSTOM_STYLESET.zip"];
    NTCompiledStyleSet* vectorTileStyleSet = [[NTCompiledStyleSet alloc] initWithAssetPackage: [[NTZippedAssetPackage alloc] initWithZipData: styleBytes]];

    // 3. Create vector tile decoder using the style set
    NTMBVectorTileDecoder* vectorTileDecoder = [[NTMBVectorTileDecoder alloc] initWithCompiledStyleSet: vectorTileStyleSet];

    // 4. Create vector tile layer, using previously created data source and decoder
    NTVectorTileLayer* vectorTileLayer = [[NTVectorTileLayer alloc] initWithDataSource:tileDataSource decoder: vectorTileDecoder];

    // 5. Add vector tile layer
    [[mapView getLayers] add:vectorTileLayer];

    {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--swift">
    {% highlight swift %}

    // 1. Define data source
    let url = "http://a.tiles.mapbox.com/v4/mapbox.mapbox-streets-v5/{zoom}/{x}/{y}.vector.pbf?access_token=YOUR-MAPBOX-TOKEN"
    let tileDataSource = NTHTTPTileDataSource(minZoom: 0, maxZoom: 14, baseURL: url)

    // 2. Load vector tile style set
    let styleBytes = NTAssetUtils.loadAsset("CUSTOM_STYLESET.zip")
    let vectorTileStyleSet = NTCompiledStyleSet(assetPackage: NTZippedAssetPackage(zip: styleBytes))

    // 3. Create vector tile decoder using the style set
    let vectorTileDecoder = NTMBVectorTileDecoder(compiledStyleSet: vectorTileStyleSet)

    // 4. Create vector tile layer, using previously created data source and decoder
    let vectorTileLayer = NTVectorTileLayer(dataSource: tileDataSource, decoder: vectorTileDecoder)

    // 5. Add vector tile layer
    mapView?.getLayers()?.add(vectorTileLayer)

    {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--kotlin">
    {% highlight kotlin %}

    // 1. Define data source
    val url = "http://a.tiles.mapbox.com/v4/mapbox.mapbox-streets-v5/{zoom}/{x}/{y}.vector.pbf?access_token=YOUR-MAPBOX-TOKEN"
    val tileDataSource = HTTPTileDataSource(0, 14, url)

    // 2. Load vector tile style set
    val styleBytes = AssetUtils.loadAsset("CUSTOM_STYLESET.zip")
    val vectorTileStyleSet = CompiledStyleSet(ZippedAssetPackage(styleBytes))

    // 3. Create vector tile decoder using the style set
    val vectorTileDecoder = MBVectorTileDecoder(vectorTileStyleSet)

    // 4. Create vector tile layer, using previously created data source and decoder
    val vectorTileLayer = VectorTileLayer(tileDataSource, vectorTileDecoder)

    // 5. Add vector tile layer
    mapView?.layers?.add(vectorTileLayer)

    {% endhighlight %}
  </div>
    
</div>

#### Web (HTTP) Tiles as RasterTileLayer

For online raster tiles you can use any common web tiles in PNG or JPG formats, in Spherical Mercator system. See [Raster Tile Sources](#raster-tile-sources).

The following tags are supported in the URL definition: **zoom, x, y, xflipped,
yflipped, quadkey**.

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

    String url = "http://ecn.t3.tiles.virtualearth.net/tiles/a{quadkey}.jpeg?g=471&mkt=en-US";
    TileDataSource baseRasterTileDataSource = new HTTPTileDataSource(1, 19, url);

    TileLayer baseLayer = new RasterTileLayer(baseRasterTileDataSource);
    mapView.getLayers().add(baseLayer);

    {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--csharp">
    {% highlight csharp %}

    string url = "http://ecn.t3.tiles.virtualearth.net/tiles/a{quadkey}.jpeg?g=471&mkt=en-US";
    var baseRasterTileDataSource = new HTTPTileDataSource(1, 19, url);

    var baseLayer = new RasterTileLayer(baseRasterTileDataSource);
    MapView.Layers.Add(baseLayer);

    {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item">
    {% highlight objc %}

    NSString* url = @"http://ecn.t3.tiles.virtualearth.net/tiles/a{quadkey}.jpeg?g=471&mkt=en-US";
    NTHTTPTileDataSource* baseRasterTileDataSource = [[NTHTTPTileDataSource alloc] initWithMinZoom:1 maxZoom:19 baseURL:url];
      
    NTRasterTileLayer* baseLayer = [[NTRasterTileLayer alloc] initWithDataSource:baseRasterTileDataSource];
    [[mapView getLayers] add:baseLayer];

    {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--swift">
    {% highlight swift %}

    let url = "http://ecn.t3.tiles.virtualearth.net/tiles/a{quadkey}.jpeg?g=471&mkt=en-US"
    let baseRasterTileDataSource = NTHTTPTileDataSource(minZoom: 1, maxZoom: 19, baseURL: url)

    let baseLayer = NTRasterTileLayer(dataSource: baseRasterTileDataSource)
    mapView?.getLayers()?.add(baseLayer)

    {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--kotlin">
    {% highlight kotlin %}

    val url = "http://ecn.t3.tiles.virtualearth.net/tiles/a{quadkey}.jpeg?g=471&mkt=en-US"
    val baseRasterTileDataSource = HTTPTileDataSource(1, 19, url)

    val baseLayer = RasterTileLayer(baseRasterTileDataSource)
    mapView?.layers?.add(baseLayer)

    {% endhighlight %}
  </div>
    
</div>

### App-defined Vector Overlay

Mobile SDK contains in-memory DataSources where applications can add vector objects as the map overlay layer.

`LocalVectorDataSource` is a dynamic DataSource that is initially empty, and allows you to add or remove objects. Objects are cached and by default, all added objects are rendered. For detailed code samples, see [Vector Elements](/docs/carto-engine/mobile-sdk/03-vector-elements/) for examples of adding Points, Markers, Texts, Lines and Polygons to map.

### App-defined Ground Overlays

`BitmapOverlayRasterTileDataSource` defines a bitmap image and the geographical coordinates of the bitmap. For details, see [Ground Overlays](/docs/carto-engine/mobile-sdk/11-ground-overlays/).

### Virtual DataSources

There are some DataSources which can use another DataSource as input, and apply internal processing and create a new DataSource as the result. This enables you to create a new DataSource based on a customized logic.

-   `PersistentCacheTileDataSource` caches HTTP tiles to a persistent sqlite database file. If the tile already exists in the database, the request to the original DataSource is ignored. This can be applied for both raster and vector tiles. The original DataSource's expired headers are taken into account as part of the processing.

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

    // 1. Create a Bing raster data source. Note: tiles start from level 1, there is no single root tile!
    String url = "http://ecn.t3.tiles.virtualearth.net/tiles/a{quadkey}.jpeg?g=471&mkt=en-US";
    TileDataSource baseRasterTileDataSource = new HTTPTileDataSource(1, 19, url);

    // 2. Add persistent caching datasource, tiles will be stored locally on persistent storage
    PersistentCacheTileDataSource cachedDataSource = new PersistentCacheTileDataSource(baseRasterTileDataSource, getExternalFilesDir(null) + "/mapcache.db");

    // 3. Create layer and add to map
    TileLayer baseLayer = new RasterTileLayer(cachedDataSource);
    mapView.getLayers().add(baseLayer);

    {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--csharp">
    {% highlight csharp %}

    // 1. Create a Bing raster data source. Note: tiles start from level 1, there is no single root tile!
    var url = "http://ecn.t3.tiles.virtualearth.net/tiles/a{quadkey}.jpeg?g=471&mkt=en-US";
    var baseRasterTileDataSource = new HTTPTileDataSource(1, 19, url);

    // Add persistent caching datasource, tiles will be stored locally on persistent storage
    // fileDir must be a directory where files can be written - this is platform-specific
    var cachedDataSource = new PersistentCacheTileDataSource(baseRasterTileDataSource, fileDir + "/mapcache.db");

    // 2. Create layer and add to map
    var baseLayer = new RasterTileLayer(cachedDataSource);
    MapView.Layers.Add(baseLayer);

    {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--objective-c">
    {% highlight objc %}

    // 1. Initialize a OSM raster data source from MapQuest Open Tiles
    NSString* url = @"http://ecn.t3.tiles.virtualearth.net/tiles/a{quadkey}.jpeg?g=471&mkt=en-US";
    NTHTTPTileDataSource* baseRasterTileDataSource = [[NTHTTPTileDataSource alloc] initWithMinZoom:1 maxZoom:19 baseURL: url];

    // 2. Create persistent cache for the given data source  
    NTPersistentCacheTileDataSource* cachedDataSource = [[NTPersistentCacheTileDataSource alloc] initWithDataSource:baseRasterTileDataSource databasePath:[NTAssetUtils calculateWritablePath:@"mycache.db"]];

    // 3. Initialize a raster layer with the previous data source
    NTRasterTileLayer* baseLayer = [[NTRasterTileLayer alloc] initWithDataSource:cachedDataSource];

    // 4. Add the previous raster layer to the map
    [[self.mapView getLayers] add:baseLayer];

    {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--swift">
    {% highlight swift %}

    // 1. Create a Bing raster data source. Note: tiles start from level 1, there is no single root tile!
    let url = "http://ecn.t3.tiles.virtualearth.net/tiles/a{quadkey}.jpeg?g=471&mkt=en-US"
    let baseRasterTileDataSource = NTHTTPTileDataSource(minZoom: 1, maxZoom: 19, baseURL: url)

    // 2. Add persistent caching datasource, tiles will be stored locally on persistent storage
    let path = NTAssetUtils.calculateWritablePath("mapcache.db")
    let cachedDataSource = NTPersistentCacheTileDataSource(dataSource: baseRasterTileDataSource, databasePath: path)

    // 3. Create layer and add to map
    let baseLayer = NTRasterTileLayer(dataSource: cachedDataSource)

    // 4. Add the previous raster layer to the map
    mapView?.getLayers()?.add(baseLayer)

    {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--kotlin">
    {% highlight kotlin %}

    // 1. Create a Bing raster data source. Note: tiles start from level 1, there is no single root tile!
    val url = "http://ecn.t3.tiles.virtualearth.net/tiles/a{quadkey}.jpeg?g=471&mkt=en-US"
    val baseRasterTileDataSource = HTTPTileDataSource(1, 19, url)

    // 2. Add persistent caching datasource, tiles will be stored locally on persistent storage
    val path = getExternalFilesDir(null).path + "/mapcache.db"
    val cachedDataSource = PersistentCacheTileDataSource(baseRasterTileDataSource, path)

    // 3. Create layer and add to map
    val baseLayer = RasterTileLayer(cachedDataSource)

    // 4. Add the previous raster layer to the map
    mapView?.layers?.add(baseLayer)

    {% endhighlight %}
  </div>
    
</div>

-   **CompressedCacheTileDataSource**

An in-memory cache tile DataSource. Although layers also cache tiles, the tiles are uncompressed and usually take 2-5x more memory after being loaded. `CompressedCacheTileDataSource` keeps all tiles in an original compressed format for better use of existing memory.

-   **CombinedTileDataSource**

A tile DataSource that combines two data sources (usually offline and online) and selects tiles based on the zoom level. All requests below a specified zoom level are directed to the first DataSource. All requests at, or above, a specified zoom level are directed to the second Datasource.

### Other Built-in DataSources

The following datasources are also built into the Mobile SDK packages.

-   **AssetTileDataSource** - loads offline tiles data from a folder (/sdcard or Application sandbox, depending on platform). For example, you can use zoom/x/y.png or zoom\_x\_y.png file naming structures for offline data

-   **NMLModelLODTreeOnlineDataSource** - uses NMLDB online API for “3D city” layer. 3D tools are provided as commercial service. [Contact us](mailto:sales@carto.com) for more information.

-   **NMLModelLODTreeSqliteDataSource** - An offline version of *NMLModelLODTreeOnlineDataSource*, model that is stored in a sqlite database file. Simple individual 3D objects (NML models) can also be added to the usual *LocalVectorDataSource*

### Shapefiles, GeoTIFF and other GIS DataSources

With the GIS extension we provide **OGRVectorDataSource** and **GDALRasterTileDataSource** which can open ESRI Shapefile, GeoTIFF, KML, GML, GPX, MapInfo and many other GIS vector and raster formats. The mobile codes just defines the file and layer to be used in your mobile app. CARTO provides a commercial package for these extensions. [Contact us](mailto@support.com) for a free evaluation demonstration.

### Custom DataSources

You can define a custom datasource for your mobile application. 
See *MyMergedRasterTileDataSource* from the [Sample Apps](/docs/carto-engine/mobile-sdk/01-getting-started/#sample-apps) for an example of using a virtual DataSource. This example uses raster data from two raster datasources and merges the map using pixel aggregations. Note that the second datasource contains transparent, or semitransparent pixels. Otherwise, the pixels from the first datasource are hidden.

Custom DataSourced should extend one of following abstract base classes:

-   **TileDataSource** - for tiles (raster or vector)
-   **VectorDataSource** - for vector objects
-   **CacheTileDataSource** - for custom tile caching (it is a *TileDataSource*)

### Raster Tile Sources

Raster tiles can produce a faster loading map by rendering the map based on pixels of small images. With certain account plans, you can use use external raster map tiles with the Mobile SDK. You can also create your own custom raster tile map source. This section describes the raster tile features available in the Mobile SDK.

In order to apply a raster tile source:

- Use the `HTTPTileDataSource` API function as the tile source

- Create `RasterTileLayer` to apply it

**Tip:** For general information about raster tiles and geospatial data, see the following Open Source Geospatial Foundation reference materials for [Tile Map Service](http://wiki.openstreetmap.org/wiki/TMS) and [Featured Tile Layers](http://wiki.openstreetmap.org/wiki/Featured_tiles).

#### Code Sample

The DataSource constructor uses the following URL patterns. It requires a minimum zoom level above `0`, and a maximum zoom level between `14 and 19`

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

    String url = "http://your-url-with-placeholders-see-below";
    TileDataSource tileDataSource = new HTTPTileDataSource(0, 18, url);
    RasterTileLayer layer = new RasterTileLayer(tileDataSource);

    {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--csharp">
    {% highlight csharp %}

    string url = "http://your-url-with-placeholders-see-below";
    TileDataSource tileDataSource = new HTTPTileDataSource(0, 18, url);
    RasterTileLayer layer = new RasterTileLayer(tileDataSource);

    {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--objective-c">
    {% highlight objc %}

    NSString* url = @"http://your-url-with-placeholders-see-below";
    NTHTTPTileDataSource* tileDataSource = [[NTHTTPTileDataSource alloc] initWithMinZoom:0 maxZoom:18 baseURL:url];
    NTRasterTileLayer* layer = [[NTRasterTileLayer alloc] initWithDataSource:tileDataSource];

    {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--swift">
    {% highlight swift %}

    let url = "http://your-url-with-placeholders-see-below"
    let tileDataSource = NTHTTPTileDataSource(minZoom: 0, maxZoom: 18, baseURL: url)
    let layer = NTRasterTileLayer(dataSource: tileDataSource)

    {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--kotlin">
    {% highlight kotlin %}

    val url = "http://your-url-with-placeholders-see-below"
    val tileDataSource = HTTPTileDataSource(0, 18, url)
    val layer = RasterTileLayer(tileDataSource)

    {% endhighlight %}
  </div>
    
</div>

<br/><br/>
The following sections provide the code for raster tile basemap urls, and a sample image.

#### HERE Satellite Tiles

`RasterTileLayer hereRasterLayer = new RasterTileLayer(new CartoOnlineTileDataSource("here.satellite.day@2x"));`

<img src="https://1.aerial.maps.api.here.com/maptile/2.1/maptile/newest/satellite.day/17/65490/43588/256/jpg?lg=eng&token=A7tBPacePg9Mj_zghvKt9Q&app_id=KuYppsdXZznpffJsKT24"/>

#### HERE Map Tiles

`RasterTileLayer hereRasterLayer = new RasterTileLayer(new CartoOnlineTileDataSource("here.normal.day@2x"));`

<img src="https://1.base.maps.api.here.com/maptile/2.1/maptile/newest/normal.day/17/65490/43588/256/png8?lg=eng&token=A7tBPacePg9Mj_zghvKt9Q&app_id=KuYppsdXZznpffJsKT24"/>

#### CARTO Positron Tiles in raster

`http://{s}.basemaps.cartocdn.com/light_all/{z}/{x}/{y}.png`

<img src="http://a.basemaps.cartocdn.com/light_all/17/65490/43588.png"/>

#### CARTO Dark Matter Tiles in raster

`http://a.basemaps.cartocdn.com/dark_all/{zoom}/{x}/{y}.png`

<img src="http://a.basemaps.cartocdn.com/dark_all/17/65490/43588.png"/>

#### OpenStreetMap Mapnik Tiles

`http://a.tile.openstreetmap.org/{zoom}/{x}/{y}.png`

<img src="http://b.tile.openstreetmap.org/15/5241/12661.png"/>

**Note:** OpenStreetMap Mapnik tiles are free for limited use. See a description of their [_Tile usage policy_](http://wiki.openstreetmap.org/wiki/Tile_usage_policy).


#### Stamen Toner

`http://a.tile.stamen.com/toner/{zoom}/{x}/{y}.png`

<img src="http://a.tile.stamen.com/toner/13/1310/3166.png"/>

Please contact [Stamen](http://www.stamen.com) directly.

#### Stamen Watercolor

`http://tile.stamen.com/watercolor/{zoom}/{x}/{y}.png`

<img src="http://tile.stamen.com/watercolor/12/654/1583.jpg"/>

Please contact [Stamen](http://www.stamen.com) directly.

#### Stamen Terrain - US Only

`http://tile.stamen.com/terrain/{zoom}/{x}/{y}.png`

<img src="http://tile.stamen.com/terrain/12/654/1583.jpg"/>

Please contact [Stamen](http://www.stamen.com) directly.

#### OpenCycleMap

`http://a.tile.opencyclemap.org/cycle/{zoom}/{x}/{y}.png`

<img width="256" height="256"  src="http://b.tile.opencyclemap.org/cycle/13/4092/2719.png" alt=""/>

© <a href="http://www.opencyclemap.org">OpenCycleMap</a>

#### OpenCycleMap Transport

`http://a.tile2.opencyclemap.org/transport/{zoom}/{x}/{y}.png`

<img width="256" height="256"  src="http://a.tile2.opencyclemap.org/transport/16/35209/21491.png" alt=""/>

Open source resource: <a href="http://www.thunderforest.com/transport/">Thunderforest</a>

#### USGS Topo National Map (US Only)

`http://basemap.nationalmap.gov/arcgis/rest/services/USGSTopo/MapServer/WMTS/tile/1.0.0/USGSTopo/default/GoogleMapsCompatible/{zoom}/{y}/{x}`

<img width="256" height="256"  src="http://basemap.nationalmap.gov/arcgis/rest/services/USGSTopo/MapServer/WMTS/tile/1.0.0/USGSTopo/default/GoogleMapsCompatible/13/3162/1306" alt="USGS Topo"/>

© USGS The National Map. The National Boundaries Dataset, National Elevation Dataset, Geographic Names Information System, National Hydrography Dataset, National Land Cover Database, National Structures Dataset, and National Transportation Dataset; U.S. Census Bureau - TIGER/Line; HERE Road Data. <a href="http://basemap.nationalmap.gov/arcgis/rest/services/USGSTopo/MapServer">USGS MapServer</a>
