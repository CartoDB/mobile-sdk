# Map Listener Events

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

## Implement MapEventListener

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
  {% highlight html %}

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
  {% highlight html %}

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
  {% highlight html %}

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
  {% highlight html %}

  COMING SOON...

  {% endhighlight %}
  </div>
  
</div>

## Initialize Listener

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
  {% highlight html %}mapView.setMapEventListener(new MyMapEventListener(mapView, vectorDataSource));

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item">
  {% highlight html %}MapView.MapEventListener = new MapListener (dataSource);

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item">
  {% highlight html %}MyMapEventListener* mapListener = [[MyMapEventListener alloc] init];
  [self setMapEventListener:mapListener];

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item">
  {% highlight html %}COMING SOON...

  {% endhighlight %}
  </div>
  
</div>

## Show/Hide Labels for Map Clicks

MapEventListener contains special functionality that shows, or hides, a small label text over a clicked object. The following label click event rules apply:

- If you click on an object, it creates another map object as a **BalloonPopup**. This contains the text from the metadata of the clicked object

  For this reason, our samples apps include a _Metadata field value_. You can use the metadata value directly, or use the **object unique ID** as the metadata, to query details about the clicked object from the database.

- When a viewer clicks a new location on the map (or click another map object), the original BalloonPopup is deleted, and new one appears at the click location

- The BalloonPopup is added to the DataSource, which is linked existing map layers. The same DataSource is used for other vector elements on a map

**Tip:** See [Event Listener guide](/guides/events) for sample code.

## DataSources

The following DataSources are available for the Mobile SDK, and are supported on all mobile platforms.

**Note:** Some DataSources are more generic and can be used for different data types and map layers (vector or raster). Others can only be applied for specific layer types.

### Offline Maps from MBTiles

**MBTilesTileDataSource** is universal datasource for raster, or vector, tile data.

#### MBtiles as RasterTileLayer

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
  {% highlight html %}MBTilesTileDataSource tileDataSource = new MBTilesTileDataSource(filePath);
    TileLayer rasterLayer = new RasterTileLayer(tileDataSource);

    mapView.getLayers().add(rasterLayer);

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item">
  {% highlight html %}var mbTilesLayer = new RasterTileLayer(new MBTilesTileDataSource(filePath));
      MapView.Layers.Add(mbTilesLayer);

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item">
  {% highlight html %}NSString* fullpathVT = [[NSBundle mainBundle] pathForResource:@"MBTILESFILENAME" ofType:@"mbtiles"];
   NTTileDataSource* tileDataSource = [[NTMBTilesTileDataSource alloc] initWithPath: fullpathVT];

// 1. Initialize a raster layer with the previous data source
      NTRasterTileLayer* rasterLayer = [[NTRasterTileLayer alloc] initWithDataSource:tileDataSource];

// 2. Add the raster layer to the map
      [[mapView getLayers] add:rasterLayer];

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item">
  {% highlight html %}COMING SOON...

  {% endhighlight %}
  </div>
  
</div>

#### MBtiles as VectorTileLayer

For packaged vector data, you will need CARTO specific vector files packages (NTVT - *NutiTeq Vector Tile*) and styling files must be in _Mapnik XML_ format. Download the following, free sample package using OpenStreetMap data:

-   [estonia\_ntvt.mbtiles](https://dl.dropboxusercontent.com/u/3573333/public_web/ntvt_packages/estonia_ntvt.mbtiles)

The Mobile SDK provides a built-in download service thorough the **Package Manager** to retrieve map packages for a country, or smaller region. For details, see [Offline Map Packages](/docs/carto-engine/mobile-sdk/offline-map-packages/#offline-map-packages).

**Note:** Vector maps always need proper **style definitions**. You can find compatible stylesheet files from this sample project (OSMBright with 3D buildings [osmbright.zip](https://github.com/nutiteq/hellomap3d-android/blob/master/com.nutiteq.advancedmap3/assets/osmbright.zip)), which contains **Mapnik XML styling** with resources (fonts, images) included. 

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
  {% highlight html %}

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
  {% highlight html %}

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
  {% highlight html %}NSString* fullpathVT = [[NSBundle mainBundle] pathForResource:@"estonia_ntvt" ofType:@"mbtiles"];
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
  {% highlight html %}COMING SOON...

  {% endhighlight %}
  </div>
  
</div>

### Online Maps

**HTTPTileDataSource** is a general datasource that can be used for vector or raster tiles. You can also use CARTO or Web (HTTP) tiles for vector layers. Additionally, you can use Web (HTTP) raster tiles for map layers. 

#### CARTO Online Vector Tile Layer

The Mobile SDK contains built-in vector tiles, as provided by CARTO maps as a service, and is avilale for all account plans. This is useful for basic maps. For vector styling, you can use the same assets provided for offline tiles (OSMBright with 3D buildings [osmbright.zip](https://github.com/nutiteq/hellomap3d-android/blob/master/com.nutiteq.advancedmap3/assets/osmbright.zip)).

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
  {% highlight html %}BinaryData styleAsset = AssetUtils.loadAsset("nutibright-v2a.zip");
        VectorTileLayer baseLayer = new CartoOnlineVectorTileLayer("nutiteq.osm", new ZippedAssetPackage(styleAsset));

        mapView.getLayers().add(baseLayer);

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item">
  {% highlight html %}var styleAsset = AssetUtils.LoadAsset("nutibright-v2a.zip");
      var baseLayer = new CartoOnlineVectorTileLayer("nutiteq.osm", new ZippedAssetPackage(styleAsset));
      
      MapView.Layers.Add(baseLayer);

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item">
  {% highlight html %}NTBinaryData* styleData = [NTAssetUtils loadAsset:@"nutibright-v3.zip"];
    NTAssetPackage* assetPackage = [[NTZippedAssetPackage alloc] initWithZipData:styleData];
    NTVectorTileLayer* vectorTileLayer = [[NTCartoOnlineVectorTileLayer alloc] initWithSource: @"nutiteq.osm" styleAssetPackage:assetPackage];

    [[mapView getLayers] add:vectorTileLayer];
  
  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item">
  {% highlight html %}COMING SOON...

  {% endhighlight %}
  </div>
  
</div>

#### Web (HTTP) Tiles as VectorTileLayer

Some Mobile SDK plans enable you to use other vector tile map DataSources. These are similar to custom raster map data sources, but a vector decoder is needed for correct implementation.

- Connect to MapBox vector tiles, which have a very similar (but not identical) data structure. You can use the same (OSMBright with 3D buildings [osmbright.zip](https://github.com/nutiteq/hellomap3d-android/blob/master/com.nutiteq.advancedmap3/assets/osmbright.zip)) asset for vector styling

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
  {% highlight html %}

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
  {% highlight html %}

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
  {% highlight html %}

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
  {% highlight html %}COMING SOON...

  {% endhighlight %}
  </div>
  
</div>

#### Web (HTTP) Tiles as RasterTileLayer

For online raster tiles you can use any common web tiles in PNG or JPG formats, in Spherical Mercator system. See our [Raster tile sources ist](/guides/raster-tile-sources).

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
  {% highlight html %}TileDataSource baseRasterTileDataSource = new HTTPTileDataSource(1, 19, "http://ecn.t3.tiles.virtualearth.net/tiles/a{quadkey}.jpeg?g=471&mkt=en-US");

        TileLayer baseLayer = new RasterTileLayer(baseRasterTileDataSource);

        mapView.getLayers().add(baseLayer);

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item">
  {% highlight html %}

// 1. Create a Bing raster data source. Note: tiles start from level 1, there is no single root tile!
      var baseRasterTileDataSource = new HTTPTileDataSource(1, 19, "http://ecn.t3.tiles.virtualearth.net/tiles/a{quadkey}.jpeg?g=471&mkt=en-US");

// 2. Create layer and add to map
      var baseLayer = new RasterTileLayer(baseRasterTileDataSource);
      MapView.Layers.Add(baseLayer);

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item">
  {% highlight html %}NTHTTPTileDataSource* baseRasterTileDataSource = [[NTHTTPTileDataSource alloc] initWithMinZoom:0 maxZoom:19 baseURL:@"http://otile1.mqcdn.com/tiles/1.0.0/osm/{zoom}/{x}/{y}.png"];
    
// 1. Initialize a raster layer with the previous data source
      NTRasterTileLayer* rasterLayer = [[NTRasterTileLayer alloc] initWithDataSource:baseRasterTileDataSource];
    
// 2. Add the previous raster layer to the map
      [[mapView getLayers] add:rasterLayer];

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item">
  {% highlight html %}COMING SOON...

  {% endhighlight %}
  </div>
  
</div>

### App-defined Vector Overlay

Mobile SDK contains in-memory DataSources where applications can add vector objects as the map overlay layer.

`LocalVectorDataSource` is a dynamic DataSource that is initially empty, and allows you to add or remove objects. Objects are cached and by default, all added objects are rendered. For detailed code samples, see [Basic Map Components](/docs/carto-engine/mobile-sdk/basic-map-components/) for examples of adding Points, Markers, Texts, Lines and Polygons to map.

### App-defined Ground Overlays

`BitmapOverlayRasterTileDataSource` defines a bitmap image and the geographical coordinates of the bitmap. For details, see [Ground Overlays](/docs/carto-engine/mobile-sdk/advanced-map-features/#ground-overlays).

### Virtual DataSources

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
  {% highlight html %}

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
  {% highlight html %}

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
  {% highlight html %}

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
  {% highlight html %}COMING SOON...

  {% endhighlight %}
  </div>
  
</div>

-   **CompressedCacheTileDataSource**

An in-memory cache DataSource. Although layers also cache tiles, the tiles are uncompressed and usually take 2-5x more memory after being loaded. `CompressedCacheTileDataSource` keeps all tiles in a compressed format for better use of existing memory.

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

You can define a custom datasource for your mobile application. See *MyMergedRasterTileDataSource* from the [Sample Apps](/docs/carto-engine/mobile-sdk/sample-apps/#sample-mobile-apps) for an example of using a virtual DataSource. This example uses raster data from two raster datasources and merges the map using pixel aggregations. Note that the second datasource contains transparent, or semitransparent pixels. Otherwise, the pixels from the first datasource are hidden.

Custom DataSourced should extend one of following abstract base classes:

-   **TileDataSource** - for tiles (raster or vector)
-   **VectorDataSource** - for vector objects
-   **CacheTileDataSource** - for custom tile caching (it is a *TileDataSource*)


## Offline Map Service for Listener Events

CARTO provides a service to download map packages in special vector formats for offline use. There is no direct URL to get these packages, instead we provide cross-platform API from within SDK: **Package Manager API**. Your application should use this API to download packages. The API provides the following features:

* Listing of available packages (countries), get IDs and metadata of specific packages
* Start package download
* Pause and resume download
* Delete downloaded package
* Get status report of package state and download status (% downloaded)

Usage of the Package Manager API requires the registration of a license code.

### Typical Use Case

<img src = "http://share.gifyoutube.com/yaNw0r.gif" alt="Offline browser" align="right">

Currently we support the following use cases:

1. Provide **interactive list of all packages** to the user, who selects a country and downloads predefined map package. From app development point of view this is most complex, as you need to create a nice UX

2. You define **one specific country** of what the viewer wants to download. Your application provides UI button to start download; or even starts download automatically when user first starts your app. This is much simpler to implement, as there is no need for list and status handling

3. Download **bounding box** map area. For example, this is suitable for city guide apps where the whole country is not needed. You can even let the viewer define their own area of interest

### Available Packages

* [List of available packages](/docs/carto-engine/mobile-sdk/offline-map-packages/#offline-map-packages) - see available package IDs for country package downloads

### Example code - complex scenario

The Mobile SDK provides pre-packaged advanced map examples: [**PackageManagerActivity**](https://github.com/nutiteq/hellomap3d-android/blob/master/com.nutiteq.advancedmap3/src/com/nutiteq/advancedmap3/PackageManagerActivity.java) for Android, and [**PackageMapController**](https://github.com/nutiteq/hellomap3d-ios/blob/master/advancedmap3/advancedmap3/PackageManagerController.mm) for iOS. You can list all packages and select the example to view the map.

### Event Diagram

The following flow diagram displays key events and messages between the app, Package Manager, and online service.

![packagemanager flow](/images/pm_flow.png)

### Additional Events

Applications must be able to properly handle any possible asynchronous events:

- *onPackageFailed* - package download failed, e.g. network connection issue

- *onPackageCancelled* - download canceled by application

- *onPackageListFailed* - package list download failed, probably network issue


### Single Package Download

Use the following steps to implement **single map package** downloading:

* **Initialize CartoPackageManager**

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
  {% highlight html %}

// 1. Register license, this must be done before PackageManager can be used!
        MapView.registerLicense("YOUR_LICENSE_HERE", getApplicationContext());

// 2. Create package manager
        File packageFolder = new File(getApplicationContext().getExternalFilesDir(null), "mappackages");
        if (!(packageFolder.mkdirs() || packageFolder.isDirectory())) {
            Log.e(Const.LOG_TAG, "Could not create package folder!");
        }
        CartoPackageManager packageManager = null;
        try {
            packageManager = new CartoPackageManager("nutiteq.mbstreets", packageFolder.getAbsolutePath());
        } catch (IOException e) {
            e.printStackTrace();
        }
        packageManager.start();

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item">
  {% highlight html %}

// Android: Register license, this must be done before PackageManager can be used!
      Carto.Ui.MapView.RegisterLicense("YOUR-NUTITEQ-LICENSE", ApplicationContext);

      // Create/find folder for packages
      var packageFolder = new File(GetExternalFilesDir(null), "packages");
      if (!(packageFolder.Mkdirs() || packageFolder.IsDirectory))
      {
        Log.Fatal("Could not create package folder!");
      }

// iOS: Register license, this must be done before PackageManager can be used!
      Carto.Ui.MapView.RegisterLicense("YOUR-NUTITEQ-LICENSE");

      // Find folder for packages
      var paths = NSSearchPath.GetDirectories(NSSearchPathDirectory.ApplicationSupportDirectory, NSSearchPathDomain.User);
      var packageFolder = paths[0] + "packages";
      NSFileManager.DefaultManager.CreateDirectory(packageFolder, true, null);

// Following code is identical for all platforms        

      // Create package manager

      // Define PackageManager listener, definition is in same class above
      var packageManager = new CartoPackageManager("nutiteq.mbstreets", packageFolder);
      var downloadedPackage = "";
      packageManager.PackageManagerListener = new PackageListener(packageManager, downloadedPackage);

      // Download new package list only if it is older than 24h
      // Note: this is only needed if pre-made packages are used
      if (packageManager.ServerPackageListAge > 24 * 60 * 60)
      {
        packageManager.StartPackageListDownload();
      }

      // Start manager - mandatory
      packageManager.Start();

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item">
  {% highlight html %}

// Create folder for package manager files. Package manager needs persistent writable folder.
    NSArray* paths = NSSearchPathForDirectoriesInDomains(NSApplicationSupportDirectory, NSUserDomainMask,YES);
    NSString* appSupportDir = [paths objectAtIndex: 0];
    NSString* packagesDir = [appSupportDir stringByAppendingString:@"packages"];
    NSError *error;
    [[NSFileManager defaultManager] createDirectoryAtPath:packagesDir withIntermediateDirectories:NO attributes:nil error:&error];
    
// Create package manager and package manager listener
    // we had defined packageManager and packageManagerListener already, as properties
    // currently the only package data source is nutiteq.mbstreets, it has OpenStreetMap global data
    NTCartoPackageManager* packageManager = [[NTCartoPackageManager alloc] initWithSource:@"nutiteq.mbstreets" dataFolder:packagesDir];
    [packageManager start];

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item">
  {% highlight html %}COMING SOON...

  {% endhighlight %}
  </div>
  
</div>

* **Implement and set PackageManagerListener** 

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
  {% highlight html %}
   
// PackageListener.java :
   /**
   * Listener for package manager events.
   */
  class PackageListener extends PackageManagerListener {
    @Override
    public void onPackageListUpdated() {
      // called when package list is downloaded
      // now you can start downloading packages
    }

    @Override
    public void onPackageListFailed() {
      // Failed to download package list
    }

    @Override
    public void onPackageStatusChanged(String id, int version, PackageStatus status) {
      // a portion of package is downloaded. Update your progress bar here.
    }

    @Override
    public void onPackageCancelled(String id, int version) {
      // called when you called cancel package download
    }

    @Override
    public void onPackageUpdated(String id, int version) {
      // called when package is updated
    }

    @Override
    public void onPackageFailed(String id, int version, PackageErrorType errorType) {
      // Failed to download package " + id + "/" + version
    }
  }

// Add the following to your activity:

  packageManager.setPackageManagerListener(new PackageListener());
  packageManager.startPackageListDownload();
   
  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item">
  {% highlight html %}

  public class PackageListener : PackageManagerListener
  {
    private PackageManager _packageManager;

    public PackageListener (PackageManager packageManager)
    {
      _packageManager = packageManager;
    }

    public override void OnPackageListUpdated ()
    {
      // called when package list is downloaded
      // now you can start downloading packages
      Log.Debug ("OnPackageListUpdated");

      // to make sure that package list is updated, full package download is called here
      if (_packageManager.GetLocalPackage ("EE") == null) {
        _packageManager.StartPackageDownload ("EE");
      }
       
    }

    public override void OnPackageListFailed ()
    {
      Log.Debug ("OnPackageListFailed");
      // Failed to download package list
    }

    public override void OnPackageStatusChanged (string id, int version, PackageStatus status)
    {
      // a portion of package is downloaded. Update your progress bar here.
      // Notice that the view and SDK are in different threads, so data copy id needed
      Log.Debug ("OnPackageStatusChanged " + id + " ver " + version + " progress " + status.Progress);
    }

    public override void OnPackageCancelled (string id, int version)
    {
      // called when you called cancel package download
      Log.Debug ("OnPackageCancelled");
    }

    public override void OnPackageUpdated (string id, int version)
    {
      // called when package is updated
      Log.Debug ("OnPackageUpdated");
    }

    public override void OnPackageFailed (string id, int version, PackageErrorType errorType)
    {
      // Failed to download package " + id + "/" + version
      Log.Debug ("OnPackageFailed: " + errorType);
    }
  }

// Add the following to your activity/controller:

    packageManager.PackageManagerListener = new PackageListener(packageManager);
    if (packageManager.ServerPackageListAge > 24 * 60 * 60) {
        packageManager.StartPackageListDownload ();
    }

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item">
  {% highlight html %}

  /*
   * Package manager listener. Listener is notified about asynchronous events
   * about packages.
   */
  @interface PackageManagerListener : NTPackageManagerListener

  - (void)onPackageListUpdated;
  - (void)onPackageListFailed;
  - (void)onPackageUpdated:(NSString*)packageId version:(int)version;
  - (void)onPackageCancelled:(NSString*)packageId version:(int)version;
  - (void)onPackageFailed:(NSString*)packageId version:(int)version errorType:(NTPackageErrorType)errorType;
  - (void)onPackageStatusChanged:(NSString*)packageId version:(int)version status:(NTPackageStatus*)status;

  @property(readonly, atomic) NSHashTable* packageManagerControllers;

  @end

  // 2. create listener implementation

  @implementation PackageManagerListener

  - (void)onPackageListUpdated
  {
      // called when package list is downloaded
      // now you can start downloading packages
      
  }

  - (void)onPackageListFailed
  {
      // called when package list download is failed (network connection)
  }

  - (void)onPackageUpdated:(NSString*)packageId version:(int)version
  {
      // called when package is updated
  }

  - (void)onPackageCancelled:(NSString*)packageId version:(int)version
  {
      // called when you called cancel package download
  }

  - (void)onPackageFailed:(NSString*)packageId version:(int)version errorType:(NTPackageErrorType)errorType
  {
      // error in package download
  }

  - (void)onPackageStatusChanged:(NSString*)packageId version:(int)version status:(NTPackageStatus*)status
  {
      // a portion of package is downloaded. Update your progress bar here.
  }

  @end
  
  // And the following to your Controller class:

    // 3. create listener object instance
  _packageManagerListener = [[PackageManagerListener alloc] init];

  // 4. Register this controller with listener to receive notifications about events
  [_packageManagerListener addPackageManagerController:self];

  // 5. Attach package manager listener
  [_packageManager setPackageManagerListener:_packageManagerListener];

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item">
  {% highlight html %}

  COMING SOON...

  {% endhighlight %}
  </div>
  
</div>

*  **Download of country package**

The following starts download of map of Estonia. See [list of available packages](/guides/packages) for available countries. Generally packages are for ISO 2-letter administrative country codes, with two exceptions:

* Some bigger countries: USA, Canada, Russia, Germany and France have one level of subdivision 
* Some countries (UK, Spain) have are both options: whole country, and subdivisions

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
  {% highlight html %}

  @Override
  public void onPackageListUpdated() {
    // called when package list is downloaded
    // now you can start downloading packages
    packageManager.startPackageDownload("EE");
  }

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item">
  {% highlight html %}

  public override void OnPackageListUpdated ()
  {
    // called when package list is downloaded
    // now you can start downloading packages
    Log.Debug ("OnPackageListUpdated");

    // to make sure that package list is updated, full package download is called here
    if (_packageManager.GetLocalPackage ("EE") == null) {
      _packageManager.StartPackageDownload ("EE");
    }
  }

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item">
  {% highlight html %}

  - (void)onPackageListUpdated
  {
    // called when package list is downloaded
    // now you can start downloading packages
   [_packageManager startPackageDownload:@"EE"];
  }

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item">
  {% highlight html %}COMING SOON...

  {% endhighlight %}
  </div>
  
</div>

*  **Download of bounding box**

If you do not need whole country, then you can define smaller area with bounding box. There is limitation of 5000 map tiles per download, depending on latitude (@ 150x150 km area). 

Bounding box is defined using the following format **bbox(lonMin,latMin,lonMax,latMax)**. You can use this 3rd party [BoundingBox](http://boundingbox.klokantech.com/) web service to define  areas (use CSV format to get the appropriate format for coordinates).

**Note:** This download does not require PackageList download, so you can start it as soon as the PackageManger is created.

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
  {% highlight html %}

// London (about 30MB)
  String bbox = "bbox(-0.8164,51.2382,0.6406,51.7401)"; 
  if (packageManager.getLocalPackage(bbox) == null) {
    packageManager.startPackageDownload (bbox);
  }

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item">
  {% highlight html %}

// London (about 30MB)
  var bbox = "bbox(-0.8164,51.2382,0.6406,51.7401)"; 
  if (packageManager.GetLocalPackage(bbox) == null) {
    packageManager.StartPackageDownload (bbox);
  }

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item">
  {% highlight html %}

// Get London greater area, near maximum area package size
 [_packageManager startPackageDownload:@"bbox(-0.8164,51.2383,0.6406,51.7402)"];

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item">
  {% highlight html %}COMING SOON...

  {% endhighlight %}
  </div>
  
</div>

* **Add datasource with offline map**

This special Vector Tile Data Source must be used: **PackageManagerTileDataSource**.

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
  {% highlight html %}

// 1. Create layer with vector styling
        BinaryData styleBytes = AssetUtils.loadAsset("osmbright.zip");
        ZippedAssetPackage assetPackage = new ZippedAssetPackage(styleBytes);

        MBVectorTileDecoder vectorTileDecoder = null;

        if (styleBytes != null) {
            // Create style set
            CompiledStyleSet vectorTileStyleSet = new CompiledStyleSet(assetPackage);
            vectorTileDecoder = new MBVectorTileDecoder(vectorTileStyleSet);
        }

// 2. Create offline datasource from Package Manager
        PackageManagerTileDataSource dataSource = new PackageManagerTileDataSource(packageManager);

        VectorTileLayer baseLayer = new VectorTileLayer(dataSource, vectorTileDecoder);

        mapView.getLayers().add(baseLayer);

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item">
  {% highlight html %}

// Define styling for vector map
      BinaryData styleBytes = AssetUtils.LoadAsset("osmbright.zip");
      ZippedAssetPackage assetPackage = new ZippedAssetPackage(styleBytes);

      MBVectorTileDecoder vectorTileDecoder = null;

      if (styleBytes != null)
      {
        // Create style set
        CompiledStyleSet vectorTileStyleSet = new CompiledStyleSet(assetPackage);
        vectorTileDecoder = new MBVectorTileDecoder(vectorTileStyleSet);
      }
      else {
        Log.Error("Failed to load style data");
      }

      var baseLayer = new VectorTileLayer(new PackageManagerTileDataSource(packageManager), vectorTileDecoder);
      MapView.Layers.Add(baseLayer);

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item">
  {% highlight html %}

// 1. Load vector tile styleset
    NSString* styleAssetName = @"osmbright.zip";
    
    NTBinaryData *vectorTileStyleSetData = [NTAssetUtils loadAsset:styleAssetName];
    NTZippedAssetPackage * package = [[NTZippedAssetPackage alloc]initWithZipData:vectorTileStyleSetData];
    
    NTCompiledStyleSet *vectorTileStyleSet = [[NTCompiledStyleSet alloc] initWithAssetPackage:package];
    
// 2. Create vector tile decoder using the styleset and update style parameters
    NTMBVectorTileDecoder* vectorTileDecoder = [[NTMBVectorTileDecoder alloc] initWithCompiledStyleSet:vectorTileStyleSet];
    [vectorTileDecoder setStyleParameter:@"lang" value:@"en"];
    
    // Optional: enable 3D elements
    [vectorTileDecoder setStyleParameter:@"buildings3d" value:@"YES"];
    [vectorTileDecoder setStyleParameter:@"markers3d" value:@"1"];
    
// 3. Create tile data source from PackageManager
    NTTileDataSource* vectorTileDataSource = [[NTPackageManagerTileDataSource alloc] initWithPackageManager:_packageManager];
    
// 4. Create vector tile layer, using previously created data source and decoder
    NTVectorTileLayer* baseLayer = [[NTVectorTileLayer alloc] initWithDataSource:vectorTileDataSource decoder:self.vectorTileDecoder];
    
// 5. Add vector tile layer as first layer
    [[mapView getLayers] insert:0 layer:baseLayer];

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item">
  {% highlight html %}COMING SOON...

  {% endhighlight %}
  </div>
  
</div>


### Updating Packages

There is no method to regularly check for event package updates, so updates can be managed in your applications by using the following logic. You can call request this logic on-demand. CARTO maps service contains an OSM offline package, which updates approximately once a month. Different packages can be updated for different frequencies.

The following procedure describes how to manage your event listener packages:

1. Use packagemanager *startPackageListDownload* to request server packages

2. Wait for listener's *onPackageListUpdated* event, and new server packages are downloaded

3. Use packageManager *getLocalPackages* to view the previously downloaded, local packages

4. Iterate a list of local packages, compare the metadata to see if the server package list contains a newer timestamp

5. If there is newer version, the package is automatically pdated

6. The updated package download behaves as an initial download

**Note:** Old maps retain all the original listener events until the new download is completed, ensuring that existing mobile maps will not be interrupted by new downloads.

#### Additional Notes

- The *startPackageListDownload* method does not need to be requested every time a package list is required. In fact, once a package list is successfully downloaded, it will saved locally. It is recommended to refresh the package contents once a day, or even once a week. Use the *getServerPackageListAge* method for checking the age of the package list data

- The Package Manager keeps a persistent task queue of all submitted requests. Even when it is stopped, downloads will automatically resume when it is restarted

- You can pause downloads by setting the task priority to -1. Downloads will resume once the priority is set to non-negative number

- *PackageInfo* class provides two methods for human-readable package names: *getNames* and *getName*

  - *getName* takes language argument and returns a localized name of the package (assuming the language is supported). Currently, only the following languages are supported: English, German, French

  - *getNames* returns list of names across multiple categories. Using *getNames* is preferred over *getName*

- Each package includes **tile mask**. Tile mask lists all tiles of the package up to a certain zoom level (currently ten). Tile mask can be used to quickly find a package corresponding to given tile coordinates, or to find a package containing given tile
