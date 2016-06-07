Nutiteq SDK update from 2.4 to 3.0
==================================

The APIs of Nutiteq SDK 3.0 are different from 2.4, so you need some
application changes. Here is one possible way to update SDK in a typical
app, and a comparison table for Nutiteq versions. This guide is
applicable for Android apps only, as there was no Nutiteq 2.x SDK for
iOS.

General steps
-------------

You cannot keep (easily) two SDK versions working in parallel, as some
API classes are overlapping. So we use following strategy:

1\. Remove old SDK from the project and add new SDK to the project\
2. Replace critical places in your code to get the map view working,
comment out all other code which gives compile errors\
3. Step by step, get old features working again: redefine base layer,
add your layers and datasources, add listeners and other settings.

The update is certainly simpler if your map-related code and Nutiteq
classes are not all around the app, but mostly reside in a single class,
usually a special Activity or Fragment.

**1. Replace SDK binaries**

-   Remove old .jar file
-   Add new SDK minimal files: .jar file, armeabi/.so file and
    osmbright.zip for styling

**2. Clean up code**

-   remove old **com.nutiteq** imports
-   comment out all mapview code (shown as red lines in your IDE)

**3. Minimal map features**

See Getting Started code for the following:

-   Create MapView, link it to your Layout
-   Set the base layer. For SDK 2.4 you most likely used some
    raster-based tiled base layer, and samples were based on
    MapQuest tiles. Technically you can keep using the same raster-based
    tile source, but now the default (and suggested) base layer is
    **NutiteqOnlineTileDataSource** for online maps, or
    **PackageManagerTileDataSource** for offline maps. Both are provided
    as part of the Nutiteq Maps service.

**4. Map options**

Note that many configuration options are different or changed in SDK
3.0, see the table below for details

**5. Map Listener**

**MapListener** is quite similar to 2.4 - it has same usage pattern and
similar event classes. Specific method parameters are different, though.
For example, for vector element click you now get all the vector
elements in the clicked location, not only the topmost one. Also,
instead of set of parameters, a data holder object is used. Typically
the changes should be quite straightforward to implement.

**6. Handling object Labels**

The handling of labels is one of the most different aspects in SDK 3.0
compared to SDK 2.4. Instead of **Labels**, which were always specified
in other vector element constructors in SDK 2.4, SDK 3.0 includes a
special class of vector elements called **Popups**. **Popups** can be
customized via subclassing, or a built-in implementation
**BalloonPopup** can be used. **BalloonPopup** has more extensive
styling options that **DefaultLabel** class in SDK 2.4, so subclassing
is rarely needed. Popups can be placed at anywhere on the map or
attached to another vector element. As **Popup** is a regular vector
element, it needs to be added to a data source, just like other vector
elements. Also opening and closing the Popups has to be handled by the
app. This means a bit more code, but gives more flexibility - e.g. you
can now have many Popups visible. See a sample in advancedmap project
how to get the old behavior (show last object label, hide when new
object is clicked etc).

SDK 3.0 has no **ViewLabel** replacement currently. If you need to use
Android Views (eg. WebView for HTML rendering), then you should create a
custom Popup class and render your view to a Bitmap. Advancedmap3
project has a sample **CustomPopup.java** that can be used as a base for
this.

**7. Attaching custom data to vector elements**

Vector elements in SDK 2.4 had **userData** field of type **Object**, so
it was possible to associate any object with the vector element. SDK 3.0
has similar **metaData** field, but this is a map of strings to strings,
not **Object** anymore. So it is not possible to attach a custom object
to a vector element anymore. A separate data structure and state is
needed for this (most likely a Map < VectorElement, Object > that is
shared between the activity and the map listener of the app).

**8. Subclassing SDK classes**

SDK 2.x was a pure Java implementation and it was possible to subclass
any SDK class (though this was not recommended). SDK 3.0 is a native
implementation with platform-specific wrappers around the native code
and only defines a small subset of the classes that support subclassing.
These classes are **data sources**, **listeners** and **popups**. For
other classes subclassing will not give any compile time errors, but
overrided methods are not called and some usage patterns may lead to
crashes (both Java and native side).

Code samples: Nutiteq SDK 2.4 vs 3.0
------------------------------------

### 1. Basics - define license, MapView and set base layer

<div id="tabs1">
<ul>
<li>
<a href="#sdk24"><span>SDK 2.4</span></a></li>

<li>
<a href="#sdk30"><span>SDK 3.0</span></a></li>

</ul>
<div id="sdk24">
``` {.brush: .java}
  // define License
  MapView.registerLicense(license,context);

  // Create map view from Layout
  mapView = (MapView) this.findViewById(R.id.map_view);
  mapView.setComponents(new Components());


  // typical base map
  RasterDataSource dataSource = new HTTPRasterDataSource(new EPSG3857(), 0, 18, "http://otile1.mqcdn.com/tiles/1.0.0/osm/{zoom}/{x}/{y}.png");
  RasterLayer mapLayer = new RasterLayer(dataSource, 0);
  mapView.getLayers().setBaseLayer(mapLayer);
    
```

</div>
<div id="sdk30">
``` {.brush: .java}
  // Define license
  MapView.registerLicense(license,context);

  // Create map view from Layout
  mapView = (MapView) this.findViewById(R.id.map_view);
  // Note: no need to create and set Components!

  // Use Nutiteq styled vector layer - SUGGESTED 
  UnsignedCharVector styleBytes = AssetUtils.LoadBytes("osmbright.zip");
  MBVectorTileStyleSet vectorTileStyleSet = new MBVectorTileStyleSet(styleBytes);
  MBVectorTileDecoder vectorTileDecoder = new MBVectorTileDecoder(vectorTileStyleSet);

  TileDataSource vectorTileDataSource = new NutiteqOnlineTileDataSource("nutiteq.mbstreets");
  VectorTileLayer baseLayer = new VectorTileLayer(vectorTileDataSource, vectorTileDecoder);
  mapView.getLayers().add(baseLayer);

  // Alternative: use same raster base layer as in SDK 2.4:

  TileDataSource baseTileDataSource = new HTTPTileDataSource(0, 18, "http://otile1.mqcdn.com/tiles/1.0.0/osm/{zoom}/{x}/{y}.png");
  baseLayer = new RasterTileLayer(baseTileDataSource);
  mapView.getLayers().add(baseLayer);
```

</div>
</div>
### 2. MapView key manipulations

<div id="tabs2">
<ul>
<li>
<a href="#sdk242"><span>SDK 2.4</span></a></li>

<li>
<a href="#sdk302"><span>SDK 3.0</span></a></li>

</ul>
<div id="sdk242">
``` {.brush: .java}
// 1. set view location
  mapView.setZoom(float)
  mapView.setFocusPoint(mapView.getLayers().getBaseProjection().fromWgs84(longitude, latitude), 500);

// 2. Set mapview with bounding box
  mapView.setBoundingBox(new Bounds(xMin, yMax, xMax, yMin), false, 500);

// 3. get current bounds
  final MapPos topLeft = mapView.screenToWorld(0, 0);
  final MapPos bottomRight = mapView.screenToWorld(mapView.getWidth(), mapView.getHeight());
```

</div>
<div id="sdk302">
``` {.brush: .java}
// 1. Set view
// Note: animation is in seconds, not milliseconds anymore.
  mapView.setZoom(float, 0.0f)
  mapView.setFocusPos(mapView.getOptions().getBaseProjection().fromWgs84(new MapPos(longitude, latitude)), 0.5f);

// 2. Set mapview with bounding box
// Note: screen and map coordinates are now with different objects: MapPos vs ScreenPos
  DisplayMetrics displaymetrics = new DisplayMetrics();
  getActivity().getWindowManager().getDefaultDisplay().getMetrics(displaymetrics);
  int height = displaymetrics.heightPixels;
  int width = displaymetrics.widthPixels;

  MapBounds mapBounds = new MapBounds(new MapPos(xMin, yMin), new MapPos(xMax, yMax));
  ScreenBounds screenBounds = new ScreenBounds(new ScreenPos(0,0), new ScreenPos(width, height));

  mapView.moveToFitBounds(mapBounds, screenBounds, false, 0.5f);

// 3. get MapView bounds
// Note: use ScreenPos() here, screenToMap is method name
  final MapPos topLeft = mapView.screenToMap(new ScreenPos(0, 0));
  final MapPos bottomRight = mapView.screenToMap(new ScreenPos(mapView.getWidth(), mapView.getHeight()));
```

</div>
</div>
### 3. Add a vector layer to map

<div id="tabs3">
<ul>
<li>
<a href="#sdk243"><span>SDK 2.4</span></a></li>

<li>
<a href="#sdk303"><span>SDK 3.0</span></a></li>

</ul>
<div id="sdk243">
``` {.brush: .java}
  // Add a geometry Layer:
  geomLayer = new GeometryLayer(mapView.getLayers().getBaseProjection());
  mapView.getLayers().addLayer(geomLayer);

  // add polygon with a hole
  double[][] pCoordsOuter = {{0,0},{0,51},{22,51},{0,0}}; // outer ring
  double[][] pCoordsInner = {{1,10},{1,50},{10,50},{1,10}}; // inner ring

  ArrayList outerPoses =  new ArrayList();
  for(double[] coord:pCoordsOuter){
    outerPoses.add(this.proj.fromWgs84((float)coord[0],(float)coord[1]));
  }

  ArrayList innerPoses =  new ArrayList();
  for(double[] coord:pCoordsInner){
    innerPoses.add(this.proj.fromWgs84((float)coord[0],(float)coord[1]));
  }

  // we need to create List of holes, as one polygon can have several holes
  List> holes =  new ArrayList>();
  holes.add(innerPoses);

  // Add Label and userData in Polygon constructor, add polygon to Layer
  geomLayer.add(new Polygon(outerPoses, holes, new DefaultLabel("Polygon"), polygonStyleSet, "123"));
```

</div>
<div id="sdk303">
``` {.brush: .java}

  // Create datasource first , apply projection to DataSource
  LocalVectorDataSource vectorDataSource = new LocalVectorDataSource(mapView.getOptions().getBaseProjection());

  // Create Layer with the data source
  VectorLayer routeLineLayer = new VectorLayer(vectorDataSource);
  mapView.getLayers().add(routeLineLayer);

  // Create polygon style and poses
  PolygonStyleBuilder polygonStyleBuilder = new PolygonStyleBuilder();
  polygonStyleBuilder.setColor(new Color(0xFFFF0000));
  lineStyleBuilder = new LineStyleBuilder();
  lineStyleBuilder.setColor(new Color(0xFF000000));
  lineStyleBuilder.setWidth(1.0f);
  polygonStyleBuilder.setLineStyle(lineStyleBuilder.buildStyle());

  // Note: use MapPosVector and MapPosVectorVector instead of ArrayList !
  MapPosVector polygonPoses = new MapPosVector();
  polygonPoses.add(proj.fromWgs84(new MapPos(24.650930, 59.421659)));
  polygonPoses.add(proj.fromWgs84(new MapPos(24.657453, 59.416354)));
  polygonPoses.add(proj.fromWgs84(new MapPos(24.661187, 59.414607)));
  polygonPoses.add(proj.fromWgs84(new MapPos(24.667667, 59.418123)));
  polygonPoses.add(proj.fromWgs84(new MapPos(24.665736, 59.421703)));
  polygonPoses.add(proj.fromWgs84(new MapPos(24.661444, 59.421245)));
  polygonPoses.add(proj.fromWgs84(new MapPos(24.660199, 59.420677)));
  polygonPoses.add(proj.fromWgs84(new MapPos(24.656552, 59.420175)));
  polygonPoses.add(proj.fromWgs84(new MapPos(24.654010, 59.421472)));

  // Create 2 polygon holes 
  MapPosVectorVector polygonHoles = new MapPosVectorVector();
  polygonHoles.add(new MapPosVector());
  polygonHoles.get(0).add(proj.fromWgs84(new MapPos(24.658409, 59.420522)));
  polygonHoles.get(0).add(proj.fromWgs84(new MapPos(24.662207, 59.418896)));
  polygonHoles.get(0).add(proj.fromWgs84(new MapPos(24.662207, 59.417411)));
  polygonHoles.get(0).add(proj.fromWgs84(new MapPos(24.659524, 59.417171)));
  polygonHoles.get(0).add(proj.fromWgs84(new MapPos(24.657615, 59.419834)));
  polygonHoles.add(new MapPosVector());
  polygonHoles.get(1).add(proj.fromWgs84(new MapPos(24.665640, 59.421243)));
  polygonHoles.get(1).add(proj.fromWgs84(new MapPos(24.668923, 59.419463)));
  polygonHoles.get(1).add(proj.fromWgs84(new MapPos(24.662893, 59.419365)));

  // Add polygon
  Polygon polygon = new Polygon(polygonPoses, polygonHoles, polygonStyleBuilder.buildStyle());

  // Add MetaDataElements with String key-values instead of userData
  polygon.setMetaDataElement("ObjectID", "123");
  vectorDataSource.add(polygon);

  // NB! there is no automatic Label display on object click
  // Implement in MapEventListener as minimum:
  @Override
  public void onVectorElementClicked(VectorElementsClickInfo vectorElementsClickInfo) {
  // Multiple vector elements can be clicked at the same time, 
  // we only care about the one closest to the camera
  VectorElementClickInfo clickInfo = vectorElementsClickInfo.getVectorElementClickInfos().get(0);

  // Create style for Popup
  BalloonPopup clickPopup = null;
  BalloonPopupStyleBuilder styleBuilder = new BalloonPopupStyleBuilder();

  VectorElement vectorElement = clickInfo.getVectorElement();
  String clickText = "ID = " + vectorElement.getMetaDataElement("ObjectID");
  if (clickText == null || clickText.length() == 0) {
    return;
  }

  // for lines and polygons set label to click location
  BalloonPopup clickPopup = new BalloonPopup(clickInfo.getElementClickPos(),
    styleBuilder.buildStyle(),
    clickText,
    "");
  }
    // vectorDataSource was passed listener via listener constructor or setter
  vectorDataSource.add(clickPopup);
    oldClickLabel = clickPopup;
  }

  //  To implement automatic closing/replacing labels see MyMapEventListener.java 
  // in advancedmap3 sample
```

</div>
</div>
### 4. Style definitions

<div id="tabs4">
<ul>
<li>
<a href="#sdk244"><span>SDK 2.4</span></a></li>

<li>
<a href="#sdk304"><span>SDK 3.0</span></a></li>

</ul>
<div id="sdk244">
``` {.brush: .java}
// PointStyle (and other styles):
  pointStyle = PointStyle.builder()
    .setSize(scale * 0.1f)
    .setPickingSize(scale * 0.25f)
    .setBitmap(
     UnscaledBitmapLoader.decodeResource(getResources(), R.drawable.ic_stop)
     )
    .build();
```

</div>
<div id="sdk304">
``` {.brush: .java}
// Notes: 
// (1) Scaling is applied automatically
// (2) Method chaining cannot be used anymore
// (3) Size is in pixels now, not relative

 PointStyleBuilder pointStyleBuilder = new PointStyleBuilder();
pointStyleBuilder.setBitmap(BitmapUtils.CreateBitmapFromAndroidBitmap(
  BitmapFactory.decodeResource(getResources(), R.drawable.ic_stop)));
  pointStyleBuilder.setSize(16);
  pointStyle = pointStyleBuilder.buildStyle();
```

</div>
</div>
### 5. Enable SDK internal logging

<div id="tabs5">
<ul>
<li>
<a href="#sdk245"><span>SDK 2.4</span></a></li>

<li>
<a href="#sdk305"><span>SDK 3.0</span></a></li>

</ul>
<div id="sdk245">
``` {.brush: .java}
 com.nutiteq.log.Log.enableAll();
 com.nutiteq.log.Log.setTag("myapp");
```

</div>
<div id="sdk305">
``` {.brush: .java}
 com.nutiteq.utils.Log.SetTag("myapp");
 com.nutiteq.utils.Log.SetShowDebug(true);
 com.nutiteq.utils.Log.SetShowInfo(true);
 com.nutiteq.utils.Log.SetShowError(true);
 com.nutiteq.utils.Log.SetShowWarn(true);
```

</div>
</div>
### 6. MapView options

<div id="tabs6">
<ul>
<li>
<a href="#sdk246"><span>SDK 2.4</span></a></li>

<li>
<a href="#sdk306"><span>SDK 3.0</span></a></li>

</ul>
<div id="sdk246">
``` {.brush: .java}

// 1. preload images and zoom level bias
 mapView.getOptions().setPreloading(true);
 mapView.getOptions().setTileZoomLevelBias(adjustment / 2.0f);

// 2. tile display options
 mapView.getOptions().setSeamlessHorizontalPan(true);
 mapView.getOptions().setTileFading(true);
 mapView.getOptions().setKineticPanning(true);

// 3. zoom actions
 mapView.getOptions().setDoubleClickZoomIn(true);
 mapView.getOptions().setDualClickZoomOut(true)

// 4. MapView manipulation restrictions
 mapView.getConstraints().setRotatable(false);
 mapView.getConstraints().setTiltRange(new Range(90, 90));
 mapView.getConstraints().setZoomRange(new Range(0, 19));

```

</div>
<div id="sdk306">
``` {.brush: .java}
// 1. preload images and zoom level bias
// Set in Layers instead of MapView
 baseLayer.setPreloading(true);
 baseLayer.setZoomLevelBias(adjustment / 2.0f);

// 2. tile display options
// SeamlessHorizontalPan, TileFading and KineticPanning are always on
// currently cannot be turned off

// 3. zoom actions
// Not built-in anymore, implement with MapEventListener
// Can be customized easily this way

  @Override
  public void onMapClicked(MapClickInfo mapClickInfo) {
   if (mapClickInfo.getClickType() == ClickType.CLICK_TYPE_DOUBLE) {
     mapView.zoom(1.5f, mapClickInfo.getClickPos(), 0.6f);
   } else if (mapClickInfo.getClickType() == ClickType.CLICK_TYPE_DUAL) {
     mapView.zoom(-1.5f, 0.6f);
   }
  }


// 4. MapView manipulation restrictions
  mapView.getOptions().setRotatable(false);
  mapView.getOptions().setZoomRange(new MapRange(0,19));
  mapView.getOptions().setTiltRange(new MapRange(90,90));
```

</div>
</div>
### 7. Object selection

<div id="tabs7">
<ul>
<li>
<a href="#sdk247"><span>SDK 2.4</span></a></li>

<li>
<a href="#sdk307"><span>SDK 3.0</span></a></li>

</ul>
<div id="sdk247">
``` {.brush: .java}
// MapView “select” object to open popup:
 mapView.selectVectorElement(marker);
```

</div>
<div id="sdk307">
``` {.brush: .java}
// There is no automatic label solution and object selection tracking. 
// application should take care of it. To mark object as "selected":
// Add BalloonPopup() for the selected object, and/or change object style in code. 
// Now you can open as many BaloonPopups() as you want, as BalloonPopup is 
// an ordinary graphics element, just as Marker,  Point etc. You can even add
// BallonPopups to other BalloonPopups
```

</div>
</div>
### 8. Data Sources

<div id="tabs8">
<ul>
<li>
<a href="#sdk248"><span>SDK 2.4</span></a></li>

<li>
<a href="#sdk308"><span>SDK 3.0</span></a></li>

</ul>
<div id="sdk248">
``` {.brush: .java}
// 1. Mapsforge Offline rendering with .map files

// 2. OGR and GDAL datasource (Shapefile, MapInfo, GeoTIFF, ECW etc)

// 3. Spatialite datasource

// 4. Read-write DataSources and Editable MapView
```

</div>
<div id="sdk308">
``` {.brush: .java}
// 1. Instead Mapsforge Nutiteq SDK 3 uses now own MBTilesTileDataSource 
// and VectorTileLayer(). Instead of .map files it uses different format 
// NTVT (Nutiteq Vector Tile) .mbtiles file, which has different structure
// The files are provided by Nutiteq

  UnsignedCharVector styleBytes = AssetUtils.LoadBytes("osmbright.zip");
  if (styleBytes != null){
    // Create style set
    MBVectorTileStyleSet vectorTileStyleSet = new MBVectorTileStyleSet(styleBytes);
    MBVectorTileDecoder vectorTileDecoder = new MBVectorTileDecoder(vectorTileStyleSet);

    MBTilesTileDataSource vectorTileDataSource = new MBTilesTileDataSource(0, 14,
"/sdcard/mymap.mbtiles");
    VectorTileLayer baseLayer = new VectorTileLayer(vectorTileDataSource, vectorTileDecoder);

    // insert as first (lowest) map Layer
    mapView.getLayers().insert(0, baseLayer);
  }else{
    // styling file not found - cannot add VectorTileLayer
    // should never happen normally
  }


// 2. OGR, GDAL (and other GIS-specific) datasources are now integrated in native 
// level, but as separate extension, not in standard package. 
// Native level gives much improved performance as there is much less 
// transfer of data to Java level, therefore tens of thousands of objects 
// can be displayed without noticeable delay. Also there is improved styling 
// API based on object attributes.
// Ask from sales@nutiteq.com for free trial copy of GIS extension.

  // Create sample point styles, one for cafes/restaurants, the other for all other POIs
  PointStyleBuilder pointStyleBuilder = new PointStyleBuilder();
  pointStyleBuilder.setColor(new Color(0xffff0000)); // fully opaque, red
  pointStyleBuilder.setSize(5.0f);
  PointStyle pointStyleBig = pointStyleBuilder.buildStyle();
  pointStyleBuilder.setColor(new Color(0x7f7f0000)); // half-transparent, red
  pointStyleBuilder.setSize(3.0f);
  PointStyle pointStyleSmall = pointStyleBuilder.buildStyle();

  // Create style selector.
  // Style selectors allow to assign styles based on element attributes and view parameters (zoom, for example)
  // Style filter expressions are given in a simple SQL-like language.
  StyleSelectorBuilder styleSelectorBuilder = new StyleSelectorBuilder()
    .addRule("type='cafe' OR type='restaurant'", pointStyleBig) // 'type' is a member of geometry meta data
    .addRule(pointStyleSmall);
  StyleSelector styleSelector = styleSelectorBuilder.buildSelector();

  //  Create data source. Use constructed style selector and copied 
  // shape file containing points.
  //  Note: if source data file is not in DataSource projection (always EPSG3857),
  // then it is automatically reprojected inside OGR, unless source data projection
  // is "unknown", then it is assumed EPSG3857

  OGRVectorDataSource ogrDataSource = new OGRVectorDataSource(new EPSG3857(), styleSelector, localDir + "/points.shp");
  MapBounds bounds = ogrDataSource.getDataExtent();
  Log.d("nutiteq","bounds:"+bounds.toString());
  mapView.setFocusPos(bounds.getCenter(), 0.0f);
  mapView.setZoom(5.0f, 0.0f);

  // Create vector layer using OGR data source
  VectorLayer ogrLayer = new VectorLayer(ogrDataSource);
  mapView.getLayers().add(ogrLayer);


// 3. Spatialite is not separate datasource, available via OGRDatasource

// 4. Editable MapView is part of SDK 3.x GIS Extension, and sample is provided to cutomers. Principles are similar to SDK 2.4 solution
```

</div>
</div>
<script>
\$( “\#tabs1” ).tabs();\
 \$( “\#tabs2” ).tabs();\
 \$( “\#tabs3” ).tabs();\
 \$( “\#tabs4” ).tabs();\
 \$( “\#tabs5” ).tabs();\
 \$( “\#tabs6” ).tabs();\
 \$( “\#tabs7” ).tabs();\
 \$( “\#tabs7” ).tabs();\
 \$( “\#tabs8” ).tabs();

</script>

