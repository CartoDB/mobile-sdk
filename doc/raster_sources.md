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
## <a href="http://developer.mapquest.com/web/products/open/map">MapQuest Open Tiles</a>
<pre>http://otile1.mqcdn.com/tiles/1.0.0/osm/{zoom}/{x}/{y}.png</pre>
<img src="http://otile1.mqcdn.com/tiles/1.0.0/map/12/654/1583.jpg"/>

Free. Please place "Tiles Courtesy of MapQuest" on your page, or in the copyright area of the map and link the word "MapQuest" to http://www.mapquest.com. 
## <a href="http://developer.mapquest.com/web/products/open/map">MapQuest Open Aerial</a>
<pre>http://otile1.mqcdn.com/tiles/1.0.0/sat/{zoom}/{x}/{y}.png</pre>
<img src="http://otile1.mqcdn.com/tiles/1.0.0/sat/1/0/0.png"/>

Free, but limited number of zooms. Please place “Tiles Courtesy of MapQuest” on your page, or in the copyright area of the map and link the word “MapQuest” to http://www.mapquest.com. 

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