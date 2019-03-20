## Offline Maps


CARTO Mobile SDK supports several different types and levels of offline maps 

* **Offline country-wide basemap** packages managed by CARTO, consumed using *PackageManager*
* **Offline area basemap** packages (e.g. cities or other custom areas), also using *PackageManager*
* **Persistently cached online map** - connect to online map and SDK keeps offline cache
* **Tile pre-download** - download online tiles for given area and zoom range, and cache on device
* **MBtiles** file with your own data. You can bundle it with app, or download from your server
* **Bundled GeoJson** with your own data
* **Offline package from CARTO** account data

An offline map app setup could be composed as this for example:

![Offline map classes](../../img/mapview-offline-layers.png)


### Offline global basemaps

Creation and management of `PackageManager`, which handles offline package discovery, updates and downloads is covered in detail the next chapter. Once you have it, you should add `CartoOfflineVectorTileLayer` to the MapView, using PackageManager and map style for offline map layer as constructor parameters. 

**Note:** If map package is not yet downloaded, then this layer will have no map. So you may want to add another online tiled layer with same style, which will be replaced once offline map is downloaded

  {% highlight java linenos %}
  CartoOfflineVectorTileLayer layer = new CartoOfflineVectorTileLayer(cartoPackageManager, CartoBaseMapStyle.CARTO_BASEMAP_STYLE_VOYAGER);
  mapView.getLayers().add(layer);
  {% endhighlight %}

### Cached online tiled map

`PersistentCacheTileDataSource` caches HTTP tiles to a persistent sqlite database file. If the tile already exists in the database, the request to the original DataSource is ignored. This can be applied for **both raster and vector tiles**. The original DataSource's expired headers are taken into account as part of the processing.

This DataSource is used as "virtual datasource" - it you create it on top of another datasource, normally `HTTPTileDataSource`.

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

    // 1. Create a Bing raster data source. Note: tiles start from level 1
    String url = "http://ecn.t3.tiles.virtualearth.net/tiles/a{quadkey}.jpeg?g=471&mkt=en-US";
    TileDataSource baseRasterTileDataSource = new HTTPTileDataSource(1, 19, url);

    // 2. Add persistent caching datasource, tiles will be stored locally on persistent storage
    PersistentCacheTileDataSource cachedDataSource = new PersistentCacheTileDataSource(baseRasterTileDataSource, getExternalFilesDir(null) + "/mapcache.db");

    // 3. Create layer and add to map
    TileLayer baseLayer = new RasterTileLayer(cachedDataSource);
    mapView.getLayers().add(baseLayer);

    {% endhighlight %}
  </div>

  <div id="tab-csharp">
    {% highlight csharp linenos %}

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

  <div id="tab-objectivec">
    {% highlight objc linenos %}

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

  <div id="tab-swift">
    {% highlight swift linenos %}

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

  <div id="tab-kotlin">
    {% highlight kotlin linenos %}

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

### Tile pre-downloading

CARTO Mobile SDK also enables you to download needed map tiles from online to your device and keep them in your cache, indefinitely. The specified download location will always be available to you offline. Download is using persistent map cache `PersistentCacheTileDataSource`'s function `startDownloadArea` 

**Note:** Most public map tile servers **do not allow** such bulk tile downloading, so this feature is meant to be used with your own server. Also check that the download would not be too big. You can check number of downloaded tiles with [tile count estimator](https://www.mapbox.com/help/offline-estimator/). We do not suggest let users to download larger areas than few thousand tiles.

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

    Projection projection = contentView.mapView.getOptions().getBaseProjection();

    String url = "http://YOUR-SERVER/{z}/{x}/{y}.png";
    String path = getExternalFilesDir(null).getAbsolutePath() + "/cache.db";

    // Approximately downtown Washington DC
    MapPos min = projection.fromWgs84(new MapPos(-77.08, 38.85));
    MapPos max = projection.fromWgs84(new MapPos(-76.94, 38.93));
    MapBounds bounds = new MapBounds(min, max);

    // This source can be anything, even aero picture etc.,
    // using the most basic variant for this example
    HTTPTileDataSource source = new HTTPTileDataSource(0, 18, url);
    PersistentCacheTileDataSource cache = new PersistentCacheTileDataSource(source, path);

    // Only uses cached tiles, does not download any new tiles during zoom
    cache.setCacheOnlyMode(true);

    cache.startDownloadArea(bounds, 0, 10, new TileDownloadListener() {
        @Override
        public void onDownloadProgress(float progress) {

        }

        @Override
        public void onDownloadCompleted() {

        }
    });

    RasterTileLayer layer = new RasterTileLayer(cache);

    contentView.mapView.getLayers().add(layer);

    {% endhighlight %}
  </div>

  <div id="tab-csharp">
    {% highlight csharp linenos %}
      
    Projection projection = ContentView.MapView.Options.BaseProjection;

    string url = "http://YOUR-SERVER/{z}/{x}/{y}.png";
    string path = GetExternalFilesDir(null).AbsolutePath + "/cache.db";

    // Approximately downtown Washington DC
    MapPos min = projection.FromWgs84(new MapPos(-77.08, 38.85));
    MapPos max = projection.FromWgs84(new MapPos(-76.94, 38.93));
    var bounds = new MapBounds(min, max);

    // This source can be anything, even aero picture etc.,
    // using the most basic variant for this example
    var source = new HTTPTileDataSource(0, 18, url);
    var cache = new PersistentCacheTileDataSource(source, path);

    // Only uses cached tiles, does not download any new tiles during zoom
    cache.CacheOnlyMode = true;

    var listener = new DownloadListener();
    cache.StartDownloadArea(bounds, 0, 10, listener);

    var layer = new RasterTileLayer(cache);

    ContentView.MapView.Layers.Add(layer);

    /*
     * DownloadListener implementation
     */
    public class DownloadListener : TileDownloadListener
    {
        public override void OnDownloadProgress(float progress)
        {
            
        }

        public override void OnDownloadCompleted()
        {
            
        }
    }

    {% endhighlight %}
  </div>

  <div id="tab-objectivec">
    {% highlight objc linenos %}

    NTProjection* projection = [[mapView getOptions] getBaseProjection];
        
    NSString* url = @"http://YOUR-SERVER/{z}/{x}/{y}.png";
        
    // Create folder for the database file
    NSArray* paths = NSSearchPathForDirectoriesInDomains(NSApplicationSupportDirectory, NSUserDomainMask,YES);
    NSString* appSupportDir = [paths objectAtIndex: 0];
    NSString* path = [appSupportDir stringByAppendingString:@"/cache.db"];
        
    // Approximately downtown Washington DC
    NTMapPos* min = [projection fromWgs84:[[NTMapPos alloc] initWithX:-77.08 y:38.85]];
    NTMapPos* max = [projection fromWgs84:[[NTMapPos alloc] initWithX:-76.948 y:38.93]];
    NTMapBounds* bounds = [[NTMapBounds alloc] initWithMin:min max:max];
        
    // This source can be anything, even aero picture etc.,
    // using the most basic variant for this example
    NTHTTPTileDataSource* source = [[NTHTTPTileDataSource alloc] initWithMinZoom:0 maxZoom:18 baseURL:url];
    NTPersistentCacheTileDataSource* cache = [[NTPersistentCacheTileDataSource alloc] initWithDataSource:source databasePath:path];
        
    // Only uses cached tiles, does not download any new tiles during zoom
    [cache setCacheOnlyMode:YES];
        
    DownloadListener* listener = [[DownloadListener alloc] init];
    [cache startDownloadArea:bounds minZoom:0 maxZoom:10 tileDownloadListener:listener];
        
    NTRasterTileLayer* layer = [[NTRasterTileLayer alloc] initWithDataSource:cache];
    [[mapView getLayers] add:layer];

    /*
     * DownloadListener implementation
     */
    @interface DownloadListener : NTTileDownloadListener

    @end

    @implementation DownloadListener

    -(void)onDownloadProgress:(float)progress {
        
    }

    -(void)onDownloadCompleted {
        
    }

    @end

    {% endhighlight %}
  </div>

  <div id="tab-swift">
    {% highlight swift linenos %}
      
    let projection = contentView.map.getOptions().getBaseProjection()
        
    let url = "http://YOUR-SERVER{z}/{x}/{y}.png"
        
    let documentDir = NSSearchPathForDirectoriesInDomains(.documentDirectory, .userDomainMask, true)[0]
    let path = documentDir + "/cache.db"
        
    // Approximately downtown Washington DC
    let min = projection?.fromWgs84(NTMapPos(x: -77.08, y: 38.85))
    let max = projection?.fromWgs84(NTMapPos(x: -76.94, y: 38.93))
    let bounds = NTMapBounds(min: min, max: max)
        
    // This source can be anything, even aero picture etc.,
    // using the most basic variant for this example
    let source = NTHTTPTileDataSource(minZoom: 0, maxZoom: 18, baseURL: url)
    let cache = NTPersistentCacheTileDataSource(dataSource: source, databasePath: path)
        
    // Only uses cached tiles, does not download any new tiles during zoom
    cache?.setCacheOnlyMode(true)
        
    let listener = DownloadListener()
    cache?.startDownloadArea(bounds, minZoom: 0, maxZoom: 10, tileDownloadListener: listener)
        
    let layer = NTRasterTileLayer(dataSource: cache)
        
    contentView.map.getLayers().add(layer)

    /*
     * DownloadListener implementation
     */
    class DownloadListener : NTTileDownloadListener {
        
        override func onDownloadProgress(_ progress: Float) {
        
        }
        
        override func onDownloadCompleted() {
            
        }
    }

    {% endhighlight %}
  </div>

  <div id="tab-kotlin">
    {% highlight kotlin linenos %}
      
    val projection = contentView!!.map.options.baseProjection

    val url = "http://YOUR-SERVER/{z}/{x}/{y}.png"
    val path = getExternalFilesDir(null).absolutePath + "/cache.db"

    // Approximately downtown Washington DC
    val min = projection.fromWgs84(MapPos(-77.08, 38.85))
    val max = projection.fromWgs84(MapPos(-76.94, 38.93))
    val bounds = MapBounds(min, max)

    // This source can be anything, even aero picture etc.,
    // using the most basic variant for this example
    val source = HTTPTileDataSource(0, 18, url)
    val cache = PersistentCacheTileDataSource(source, path)

    // Only uses cached tiles, does not download any new tiles during zoom
    cache.isCacheOnlyMode = true

    cache.startDownloadArea(bounds, 0, 10, object: TileDownloadListener() {
        override fun onDownloadProgress(progress: Float) {
            print("Download progress: " + progress.toString())
        }

        override fun onDownloadCompleted() {
            print("Download complete")
        }
    })

    val layer = RasterTileLayer(cache)

    contentView!!.map.layers.add(layer)

    {% endhighlight %}
  </div>
</div>

### MBTiles files

Following code samples assume that you have already mbtiles file in device file system, and know the file full path. How you exactly get it, from app bundle or download it from your server, depends on app. 

You can use your favorite MBTiles file creator go convert your data to MBTiles, e.g. MBTiler. 

**Note:** You cannot open mbtiles file directly from Android assets (i.e. APK file), instead you need to copy the file to file storage, and open it from there. On iOS this is not needed.

#### Raster MBTiles

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

    MBTilesTileDataSource tileDataSource = new MBTilesTileDataSource("MBTILES_FILE_FULL_PATH");
    RasterTileLayer mbTilesLayer = new RasterTileLayer(tileDataSource);
    
    mapView.getLayers().add(mbTilesLayer);

    {% endhighlight %}
  </div>

  <div id="tab-csharp">
    {% highlight csharp linenos %}
  
    var tileDataSource = new MBTilesTileDataSource(filePath);
    var mbTilesLayer = new RasterTileLayer(tileDataSource);

    MapView.Layers.Add(mbTilesLayer);

    {% endhighlight %}
  </div>

  <div id="tab-objectivec">
    {% highlight objc linenos %}
  
    NTTileDataSource* tileDataSource = [[NTMBTilesTileDataSource alloc] initWithPath: @"MBTILES_FILE_FULL_PATH"];
    NTRasterTileLayer* mbTilesLayer = [[NTRasterTileLayer alloc] initWithDataSource:tileDataSource];
    
    [[mapView getLayers] add:mbTilesLayer];

    {% endhighlight %}
  </div>

  <div id="tab-swift">
    {% highlight swift linenos %}
  
    let tileDataSource = NTMBTilesTileDataSource(path: "MBTILES_FILE_FULL_PATH")
    let mbTilesLayer = NTRasterTileLayer(dataSource: tileDataSource)
        
    mapView?.getLayers()?.add(mbTilesLayer)

    {% endhighlight %}
  </div>

  <div id="tab-kotlin">
    {% highlight kotlin linenos %}
  
    val tileDataSource = MBTilesTileDataSource("MBTILES_FILE_FULL_PATH")
    val mbTilesLayer = RasterTileLayer(tileDataSource)
    
    mapView?.layers?.add(mbTilesLayer)

    {% endhighlight %}
  </div>
    
</div>

#### MBTiles as VectorTileLayer

Vector maps always need proper **style definitions**. Style definition must match with specific data schema (i.e. layers and tags there), and if you have your custom data inside vector tiles, then the style must be compatible with that. Building suitable style asset is described for the case of customized basemap styles in SDK wiki ["Creating custom basemap styles"](https://github.com/CartoDB/mobile-sdk/wiki/Creating-custom-basemap-styles). Basemap styling and overlay styling with vector tiles is technically same, so same style package format is used there.

You can also use **CARTO Builder** to define map styling using web interface , see chapter **Offline CARTO map** below how to get data to mobile offline from this.

In this sample we use mbtiles downloaded from openmaptiles.org, these are in [openmaptiles schema](http://openmaptiles.org/schema). As also CARTO bundled vector maps use same schema, then the built-in *CARTO_BASEMAP_STYLE_VOYAGER* and other styles happen to be compatible with this, and can be used to render the data.

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

    MBTilesTileDataSource tileDataSource = new MBTilesTileDataSource("estonia.mbtiles");

    // Create tile decoder based on Voyager style and VectorTileLayer
    VectorTileDecoder tileDecoder = CartoVectorTileLayer.createTileDecoder(CartoBaseMapStyle.CARTO_BASEMAP_STYLE_VOYAGER);
    VectorTileLayer offlineLayer = new VectorTileLayer(tileDataSource, tileDecoder);

    mapView.getLayers().add(offlineLayer);

    {% endhighlight %}
  </div>

  <div id="tab-csharp">
    {% highlight csharp linenos %}

    var tileDataSource = new MBTilesTileDataSource("estonia.mbtiles");

    // Create tile decoder based on Voyager style and VectorTileLayer
    var tileDecoder = CartoVectorTileLayer.CreateTileDecoder(CartoBaseMapStyle.CartoBasemapStyleVoyager);
    var offlineLayer = new VectorTileLayer(tileDataSource, tileDecoder);

    mapView.Layers.Add(offlineLayer);

    {% endhighlight %}
  </div>

  <div id="tab-objectivec">
    {% highlight objc linenos %}
  
    NSString* path = [[NSBundle mainBundle] pathForResource:@"estonia" ofType:@"mbtiles"];
    NTMBTilesTileDataSource* tileDataSource = [[NTMBTilesTileDataSource alloc] initWithPath:path];
    
    // Create tile decoder based on Voyager style and VectorTileLayer
    NTMBVectorTileDecoder* tileDecoder = [NTCartoVectorTileLayer createTileDecoder: NT_CARTO_BASEMAP_STYLE_VOYAGER];
    NTVectorTileLayer* offlineLayer = [[NTVectorTileLayer alloc] initWithDataSource:tileDataSource decoder:tileDecoder];
    
    [[mapView getLayers] add:offlineLayer];

    {% endhighlight %}
  </div>

  <div id="tab-swift">
    {% highlight swift linenos %}

    let tileDataSource = NTMBTilesTileDataSource(path: Bundle.main.path(forResource: "estonia", ofType: "mbtiles"))
    
    // Create tile decoder based on Voyager style and VectorTileLayer
    let tileDecoder = NTCartoVectorTileLayer.createTileDecoder(CartoBaseMapStyle.CARTO_BASEMAP_STYLE_VOYAGER)
    let offlineLayer = NTVectorTileLayer(tileDataSource, tileDecoder)
    
    mapView?.layers?.add(offlineLayer)

    {% endhighlight %}
  </div>

  <div id="tab-kotlin">
    {% highlight kotlin linenos %}
  
    val tileDataSource = MBTilesTileDataSource("estonia.mbtiles")
    
    // Create tile decoder based on Voyager style and VectorTileLayer
    val tileDecoder = CartoVectorTileLayer.createTileDecoder(CartoBaseMapStyle.CARTO_BASEMAP_STYLE_VOYAGER)
    val offlineLayer = VectorTileLayer(tileDataSource, tileDecoder)

    mapView?.layers?.add(offlineLayer)

    {% endhighlight %}
  </div>
    
</div>


### GeoJson

[GeoJSON](http://geojson.org/) is a format for encoding a variety of geographic data structures. GeoJSON supports the following geometry types: Point, LineString, Polygon, MultiPoint, MultiLineString, and MultiPolygon. Geometric objects with additional properties are Feature objects. Sets of features are contained by FeatureCollection objects.

The following example displays how you can load GeoJSON from bundled assets and render it on the map. Make sure you have the loaded .geojson file as a bundled Asset (Android) or Resource (iOS), you can get a copy from [here](https://github.com/CartoDB/mobile-dotnet-samples/blob/master/AdvancedMap.Droid/Assets/cities15000.geojson)

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

    // Initialize a local vector data source
    final Projection projection = mapView.getOptions().getBaseProjection();
    final LocalVectorDataSource source = new LocalVectorDataSource(projection);
    VectorLayer layer = new VectorLayer(source);
    mapView.getLayers().add(layer);

    // As the file to load is rather large, we do not want to block our main thread
    thread = new Thread(new Runnable() {
        @Override
        public void run() {

            // Create a basic style
            MarkerStyle style = new MarkerStyleBuilder().buildStyle();

            // Read GeoJSON, parse it using SDK GeoJSON parser
            GeoJSONGeometryReader reader = new GeoJSONGeometryReader();

            // Set target projection to base (mercator)
            reader.setTargetProjection(projection);

            String fileName = "cities15000.geojson";
            String json;

            try {
                InputStream is = getAssets().open(fileName);
                int size = is.available();
                byte[] buffer = new byte[size];
                is.read(buffer);
                is.close();

                json = new String(buffer, "UTF-8");

            } catch (IOException ex) {
                return;
            }

            // Read features from local asset
            FeatureCollection features = reader.readFeatureCollection(json);

            VectorElementVector elements = new VectorElementVector();

            for (int i = 0; i < features.getFeatureCount(); i++) {
                // This data set features point geometry,
                // however, it can also be LineGeometry or PolygonGeometry
                PointGeometry geometry = (PointGeometry) features.getFeature(i).getGeometry();
                elements.add(new Marker(geometry, style));
            }

            source.addAll(elements);
        }
    });

    thread.start();

    {% endhighlight %}
  </div>

  <div id="tab-csharp">
    {% highlight csharp linenos %}
      
    // Read json from assets and add to map
    string json;

    using (System.IO.StreamReader sr = new System.IO.StreamReader (Assets.Open ("cities15000.geojson")))
    {
        json = sr.ReadToEnd ();
    }

    // Initialize a local vector data source
    var projection = MapView.Options.BaseProjection;
    var source = new LocalVectorDataSource(projection);

    VectorLayer layer = new VectorLayer(source);

    new System.Threading.Thread((obj) =>
    {
        // Create basic style. Use the MarkerStyleBuilder to modify your markers
        MarkerStyle style = new MarkerStyleBuilder().BuildStyle();

        // Read GeoJSON, parse it using SDK GeoJSON parser
        GeoJSONGeometryReader reader = new GeoJSONGeometryReader();

        // Set target projection to base (mercator)
        reader.TargetProjection = projection;
        
        // Read features from local asset
        FeatureCollection features = reader.ReadFeatureCollection(json);
        
        VectorElementVector elements = new VectorElementVector();

        for (int i = 0; i < features.FeatureCount; i++)
        {
            // This data set features point geometry,
            // however, it can also be LineGeometry or PolygonGeometry
            PointGeometry geometry = (PointGeometry)features.GetFeature(i).Geometry;
            elements.Add(new Marker(geometry, style));
        }

        source.AddAll(elements);
        
        // Add the clustered vector layer to the map
        MapView.Layers.Add(layer);

    }).Start();

    {% endhighlight %}
  </div>

  <div id="tab-objectivec">
    {% highlight objc linenos %}

    // Initialize a local vector data source
    NTProjection* projection = [[self.mapView getOptions] getBaseProjection];
    NTLocalVectorDataSource* source = [[NTLocalVectorDataSource alloc] initWithProjection:projection];
    NTVectorLayer* layer = [[NTVectorLayer alloc] initWithDataSource:source];
    [[[self mapView] getLayers] add:layer];
        
    dispatch_queue_t queue = dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0ul);
    dispatch_async(queue, ^{
        // Read .geojson
        NSString* fullpath = [[NSBundle mainBundle] pathForResource:@"cities15000" ofType:@"geojson"];
        NSString* json = [NSString stringWithContentsOfFile:fullpath encoding:NSUTF8StringEncoding error:nil];
        
        // .geojson parsing
        NTGeoJSONGeometryReader* geoJsonReader = [[NTGeoJSONGeometryReader alloc] init];
        [geoJsonReader setTargetProjection:projection];
        
        NTFeatureCollection* features = [geoJsonReader readFeatureCollection:json];
        
        // Initialize basic style, as it will later be overridden
        NTMarkerStyle* style = [[[NTMarkerStyleBuilder alloc] init] buildStyle];
        
        NTVectorElementVector* elements = [[NTVectorElementVector alloc] init];
        
        for (int i = 0; i < [features getFeatureCount]; i++) {
            NTPointGeometry* geometry = (NTPointGeometry *)[[features getFeature:i] getGeometry];
        
            NTMarker* marker = [[NTMarker alloc] initWithGeometry:geometry style:style];
            [elements add:marker];
        }
        
        // To avoid flickering, add all the sources
        [source addAll:elements];
    });

    {% endhighlight %}
  </div>

  <div id="tab-swift">
    {% highlight swift linenos %}

    let projection = contentView.map.getOptions().getBaseProjection()
    let source = NTLocalVectorDataSource(projection: projection)
    let layer = NTVectorLayer(dataSource: source)
        
    contentView.map.getLayers().add(layer)
        
    DispatchQueue.global().async {

        let path = Bundle.main.path(forResource: "cities15000", ofType: "geojson")
        
        guard let json = try? NSString(contentsOfFile: path!, encoding: String.Encoding.utf8.rawValue) else {
            return
        }
        
        // Create default style
        let mBuilder = NTMarkerStyleBuilder()
        let style = mBuilder?.buildStyle()
        
        // Read GeoJSON, parse it using SDK GeoJSON parser
        let reader = NTGeoJSONGeometryReader()
        reader?.setTargetProjection(self.contentView.map.getOptions().getBaseProjection())
        
        let features = reader?.readFeatureCollection(json as String!)
        
        let elements = NTVectorElementVector()
        let total = Int((features?.getFeatureCount())!)
        
        
        for i in stride(from: 0, to: total, by: 1) {
            // This data set features point geometry, however, it can also be LineGeometry or PolygonGeometry
            let geometry = features?.getFeature(Int32(i)).getGeometry() as? NTPointGeometry
            elements?.add(NTMarker(geometry: geometry, style: style))
        }
        
        DispatchQueue.main.async(execute: {
            source?.addAll(elements)
        })
    }

    {% endhighlight %}
  </div>

  <div id="tab-kotlin">
    {% highlight kotlin linenos %}
      
    val source = LocalVectorDataSource(contentView!!.map.options.baseProjection)
    val layer = VectorLayer(source)
    contentView!!.map.layers.add(layer)

    // Kotlin Anko library: https://github.com/Kotlin/anko
    doAsync {

        val filename = "cities15000.geojson"

        val stream = assets.open(filename)
        val size = stream.available()
        val buffer = ByteArray(size)

        stream.read(buffer)
        stream.close()

        val json = String(buffer, charset("UTF-8"))

        // Basic style builder
        val mBuilder = MarkerStyleBuilder()
        val style = mBuilder.buildStyle()

        // Read GeoJSON, parse it using SDK GeoJSON parser
        val reader = GeoJSONGeometryReader()
        reader.targetProjection = contentView?.projection

        val features = reader.readFeatureCollection(json)

        val elements = VectorElementVector()
        val total = features.featureCount

        for(i in 0..total - 1) {
            // This data set features point geometry, however, it can also be LineGeometry or PolygonGeometry
            val geometry = features.getFeature(i).geometry as PointGeometry
            elements.add(Marker(geometry, style))
        }

        source.addAll(elements)
    }

    {% endhighlight %}
  </div>
</div>


### Offline CARTO map

You can create offline map package is via CARTO platform, from a dataset in BUILDER:

- Upload your data to CARTO, create a new Map with CARTO Builder, define map  layer styling
- Use the [Mobile Tile Packager](https://github.com/CartoDB/mobile-tile-packager) tool to create the offline map data package from the map, as MBTiles file.
- Add the package file to the mobile app - you can have your app download it from your server, or add it as bundled asset to your app.
- Add the map to MapView, as a `VectorTileLayer`, from `MBTilesTileDataSource` and apply CartoCSS as String for styling. You can take and copy-paste CartoCSS from BUILDER's Layer Styling tab and "CARTOCSS" view: check the toggle in bottom of the screen.

This method enables you to create two in one: optimized vector tiles and suitable CartoCSS styling for your map.

**Note:** If you use text labels, then you need to bundle and add Font package .zip file to the app and TileDecoder, as in sample below. You can get one from [here](https://github.com/CartoDB/mobile-dotnet-samples/blob/master/AdvancedMap.Droid/Assets/carto-fonts.zip), copy it to your app bundled assets.

  {% highlight java linenos %}
  
        // Below "text-placement: nutibillboard;" does not work in BUILDER web, it is special placement for 3D mobile maps
        
        String cartoCss =
                "#offlinepackages {\n" +
                "  polygon-fill: #374C70;\n" +
                "  polygon-opacity: 0.9;\n" +
                "  polygon-gamma: 0.5;\n" +
                        " ::outline {" +
                        " line-color: #FFF;\n" +
                        "}" +
                "#offlinepackages::labels {\n" +
                "  text-name: [package_id];\n" +
                "  text-face-name: 'DejaVu Sans Book';\n" +
                "  text-size: 10;\n" +
                "  text-fill: #130505;\n" +
                "  text-label-position-tolerance: 0;\n" +
                "  text-halo-radius: 1;\n" +
                "  text-halo-fill: #dee3e7;\n" +
                "  text-dy: -10;\n" +
                "  text-allow-overlap: false;\n" +
                "  text-placement: nutibillboard;\n" +
                "  text-placement-type: dummy;\n" +
                "}";

        BinaryData styleAsset = AssetUtils.loadAsset("carto-fonts.zip");
        ZippedAssetPackage assetPackage = new ZippedAssetPackage(styleAsset);

        MBVectorTileDecoder decoder = new MBVectorTileDecoder(new CartoCSSStyleSet(cartoCss, assetPackage));

        MBTilesTileDataSource mbDataSource = createMbtilesDataSourceFromAsset(mbTileFile);
        VectorTileLayer mbLayer = new VectorTileLayer(mbDataSource,decoder);
        mapView.getLayers().add(mbLayer);

 {% endhighlight %}