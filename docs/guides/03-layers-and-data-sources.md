## Layers and Data Sources

Maps in CARTO Mobile SDK are organized overlaying **layers**. Layer displays map and takes data from a  connected **DataSource**, which has data loading and processing logics.


### Layers overview

Depending on data type you can use following layer types:

* **VectorTileLayer** - renders styled maps from Mapbox Vector Tile (MVT) data. Use your own datasource, or one of the builtin ones:
  * **CartoVectorTileLayer** - uses built-in styles, requires suitable DataSource
  * **CartoOnlineVectorTileLayer** - specific one which connects to CARTO basemap server with built-in online tile DataSource. It is typical basemap, we provide several built-in styles for this.
  * **CartoOfflineVectorTileLayer** - connects to CARTO offline map package service via *PackageManager*. 
* **RasterTileLayer** - uses raster tiles, which need to be in global OpenStreetMap tiling schema. You can use own defined server, or CARTO one:
  * **CartoOnlineRasterTileLayer** - connects to raster-based maps bundled with CARTO service, currently used for **Satellite images**
* **VectorLayer** - renders individual vector elements, this is used to add your own Markers, Points, Lines, Polygons and 3D elements to the map. It gives more flexibility than VectorTileLayer (which can also be used for your own data), but does not scale so well for large number of objects. 
* **ClusteredVectorLayer** - special version of VectorLayer which builds interactive and dynamically rendered clusters from points in datasource 
* **EditableVectorLayer** - another special version of VectorLayer which allows to drag points, change lines and polygons. Has special methods to get events for edits and saves.
* **NMLModelLODTreeLayer** - An advanced layer for 3D models that supports automatic Level of Detail (LOD) resolution calculation based on view. 
* **SolidLayer** - just global solid color. Meant for backgrounds and effects


### Online map sample

An online map app configuration could be composed from following classes and elements:

<span class="wrap-border"><img src="../../img/mapview-online-layers.png" alt="Online map classes" /></span>

Here are:
* **Basemap** is standard online map. You just select style, and CARTO provides the map. You can use custom styles there also.
* On top of basemap there is **VectorLayer** with Vector objects, e.g. points. The data can be loaded from your own server, using whatever API what the server provides. Your app defines custom DataSource which defines how exactly objects (Markers, Lines, Polygons etc) are created based on data what is loaded from your server. New data is needed with every map movement, so you need to be careful there, use caching for example. You do not need to change SDK code for that, all can be done in app code.
* On top of this is another **Editable layer**, with a Marker which is used as draggable pin, to mark some location. User can change the pin. You may even have editable Lines and Polygons for more complex cases there. If you do not need geometry editing for the objects, just use **plain VectorLayer**, and via *LocalVectorDataSource* you can add immutable, but still reactive to clicks, map objects to map.

Every app is different, and in your app you may need different combination of Layers and Data Sources, and SDK provides you many of them. Data Sources can be implemented by app, if built-in ones are not enough, or if you need to support your specific server API or file format. Layers and vector objects are fixed; and there is very rarely any need to change these.

The layers are added from bottom to top order.

#### Basemap layer: CARTO Online Vector Tile Layer

This adds to map CARTO built-in vector tile service with generic streetmaps, you just set styling and all the rest is done automatically.

<div class="js-tabs-mobilesdk">
   <ul class="tab-navigation">
     <li class="tab-navigationItem">
       <a href="tab-java">Java</a>
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
  
     CartoOnlineVectorTileLayer layer = new CartoOnlineVectorTileLayer(CartoBaseMapStyle.CARTO_BASEMAP_STYLE_POSITRON);
     mapView.getLayers().add(layer);

     {% endhighlight %}
  </div>
  <div id="tab-csharp">
     {% highlight csharp linenos %}
  
     var layer = new CartoOnlineVectorTileLayer(CartoBaseMapStyle.CartoBasemapStylePositron);
     MapView.Layers.Add(layer);

     {% endhighlight %}
  </div>
  <div id="tab-objectivec">
     {% highlight objc linenos %}
  
     NTCartoOnlineVectorTileLayer* layer = [[NTCartoOnlineVectorTileLayer alloc] initWithStyle:NT_CARTO_BASEMAP_STYLE_POSITRON];
     [[self.mapView getLayers] add:layer];
  
     {% endhighlight %}
  </div>
  <div id="tab-swift">
     {% highlight swift linenos %}

     let layer = NTCartoOnlineVectorTileLayer(style: NTCartoBaseMapStyle.CARTO_BASEMAP_STYLE_POSITRON)
     mapView?.getLayers()?.add(layer)

     {% endhighlight %}
  </div>
  <div id="tab-kotlin">
     {% highlight kotlin linenos %}

     val layer = CartoOnlineVectorTileLayer(CartoBaseMapStyle.CARTO_BASEMAP_STYLE_POSITRON)
     mapView?.layers?.add(layer)

     {% endhighlight %}
  </div>
</div>

#### Basemap layer: CARTO Satellite

For certain maps - especially for **Satellite imagery** (sourced from HERE) CARTO provides ready-made online raster maps layer. Following ID-s can be used as "source" parameter in *CartoOnlineRasterTileLayer*:

* here.satellite.day
* here.satellite.day@2x - for higher resolution
* here.normal.day
* here.normal.day@2x


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

  CartoOnlineVectorTileLayer layer = new CartoOnlineRasterTileLayer("here.satellite.day");
  mapView.getLayers().add(layer);

  {% endhighlight %}
  </div>

  <div id="tab-csharp">
  {% highlight csharp linenos %}

  var layer = new CartoOnlineRasterTileLayer("here.satellite.day");
  MapView.Layers.Add(layer);

  {% endhighlight %}
  </div>

  <div id="tab-objectivec">
  {% highlight objc linenos %}

  NTCartoOnlineVectorTileLayer* layer = [[NTCartoOnlineRasterTileLayer alloc] initWithSource:@"here.satellite.day"];
  [[self.mapView getLayers] add:layer];

  {% endhighlight %}
  </div>

  <div id="tab-swift">
  {% highlight swift linenos %}

  let layer = NTCartoOnlineRasterTileLayer(source: "here.satellite.day")
  mapView?.getLayers()?.add(layer)

  {% endhighlight %}
  </div>

  <div id="tab-kotlin">
  {% highlight kotlin linenos %}

  val layer = CartoOnlineRasterTileLayer("here.satellite.day")
  mapView?.layers?.add(layer)

  {% endhighlight %}
  </div>
</div>



### Other Data Sources


#### Custom DataSources

Your **app can define own DataSources for custom needs**, e.g. implement loading from own API, process/edit data on the fly etc. This can be done in app level and in whatever language you use, without touching SDK code. Custom DataSource should extend one of following abstract base classes:

-   **TileDataSource** - for tiles (raster or vector)
-   **VectorDataSource** - for vector objects
-   **CacheTileDataSource** - for custom tile caching (it is a *TileDataSource*)

Some DataSources are more generic and can be used for different data types and map layers (vector or raster). Others can only be applied for specific layer types.

####  Tile DataSources

Data Sources can deal with map tiles, or with individual vector objects. Here are the key tile-based datasources:

**MBTilesTileDataSource**  loads data as tiles from MBTiles file on device. The tiles can be raster tiles or vector tiles (Mapbox Vector Tiles). Your app has to know where is the MBTIles and has to take care logistics of the file. 

**HTTPTileDataSource** loads online tiles using HTTP. It does not care whether the content is raster of vector tile, but you need to know it, and use it with either *RasterTileLayer* or *VectorTileLayer*.

**PersistentCacheTileDataSource** is a 'virtual' data source in a sense that it does not provide data itself, but takes data from another specified datasource, caches it in file system with given parameters, and outputs it. So if already cached tile is requested, then it is loaded from the cache, not original source. It works with tiled data sources, and is typically used with *HTTPTileDataSource* to cache persistently online tiles.

#### Custom server with raster tiles

As example following code loads online raster tiles into your map view. You can use any common web tiles in PNG or JPG formats, in Spherical Mercator system, just define proper ZXY or Quad-tree-based URL pattern for tiles. 

**Tip:** For general information about raster tiles and geospatial data, see the following Open Source Geospatial Foundation reference materials for [Tile Map Service](http://wiki.openstreetmap.org/wiki/TMS) and [Featured Tile Layers](http://wiki.openstreetmap.org/wiki/Featured_tiles).

The following tags are supported in the URL definition: **zoom, x, y, xflipped, yflipped, quadkey**.

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

    String url = "http://ecn.t3.tiles.virtualearth.net/tiles/a{quadkey}.jpeg?g=471&mkt=en-US";
    TileDataSource baseRasterTileDataSource = new HTTPTileDataSource(1, 19, url);

    TileLayer baseLayer = new RasterTileLayer(baseRasterTileDataSource);
    mapView.getLayers().add(baseLayer);

    {% endhighlight %}
  </div>
  <div id="tab-csharp">
    {% highlight csharp linenos %}

    string url = "http://ecn.t3.tiles.virtualearth.net/tiles/a{quadkey}.jpeg?g=471&mkt=en-US";
    var baseRasterTileDataSource = new HTTPTileDataSource(1, 19, url);

    var baseLayer = new RasterTileLayer(baseRasterTileDataSource);
    MapView.Layers.Add(baseLayer);

    {% endhighlight %}
  </div>

  <div id="tab-objectivec">
    {% highlight objc linenos %}

    NSString* url = @"http://ecn.t3.tiles.virtualearth.net/tiles/a{quadkey}.jpeg?g=471&mkt=en-US";
    NTHTTPTileDataSource* baseRasterTileDataSource = [[NTHTTPTileDataSource alloc] initWithMinZoom:1 maxZoom:19 baseURL:url];
      
    NTRasterTileLayer* baseLayer = [[NTRasterTileLayer alloc] initWithDataSource:baseRasterTileDataSource];
    [[mapView getLayers] add:baseLayer];

    {% endhighlight %}
  </div>

  <div id="tab-swift">
    {% highlight swift linenos %}

    let url = "http://ecn.t3.tiles.virtualearth.net/tiles/a{quadkey}.jpeg?g=471&mkt=en-US"
    let baseRasterTileDataSource = NTHTTPTileDataSource(minZoom: 1, maxZoom: 19, baseURL: url)

    let baseLayer = NTRasterTileLayer(dataSource: baseRasterTileDataSource)
    mapView?.getLayers()?.add(baseLayer)

    {% endhighlight %}
  </div>

  <div id="tab-kotlin">
    {% highlight kotlin linenos %}

    val url = "http://ecn.t3.tiles.virtualearth.net/tiles/a{quadkey}.jpeg?g=471&mkt=en-US"
    val baseRasterTileDataSource = HTTPTileDataSource(1, 19, url)

    val baseLayer = RasterTileLayer(baseRasterTileDataSource)
    mapView?.layers?.add(baseLayer)

    {% endhighlight %}
  </div>    
</div>
