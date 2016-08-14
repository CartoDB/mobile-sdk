STILL BEING EDITED....

# Advanced Map Features

This section describes some of the advanced map features that are available for select account plans, such as [Raster Tiles](#raster-tile-sources), [Cluster](#cluster), [MBTiles](#mbtiles-for-map-data), [Ground Overlays](#ground-overlays), [Vector Styles](#vector-styles), and [Routing-offline and online](#routing-offline-and-online).

## Raster Tile Sources

Raster tiles can produce a faster loading map by rendering the map based on pixels of small images. With certain account plans, you can use use external raster map tiles with the Mobile SDK. You can also create your own custom raster tile map source. This section describes the raster tile features available in the Mobile SDK.

In order to apply a raster tile source:

- Use the `HTTPTileDataSource` API function as the tile source

- Create `RasterTileLayer` to apply it

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
  {% highlight html %}

      String url = "http://your-url-with-placeholders-see-below";
      TileDataSource tileDataSource = new HTTPTileDataSource(0, 18, url);
      RasterTileLayer layer = new RasterTileLayer(tileDataSource);

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item">
  {% highlight html %}

    string url = "http://your-url-with-placeholders-see-below";
    TileDataSource tileDataSource = new HTTPTileDataSource(0, 18, url);
    RasterTileLayer layer = new RasterTileLayer(tileDataSource);

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item">
  {% highlight html %}

    NSString* url = @"http://your-url-with-placeholders-see-below";
    NTHTTPTileDataSource *source = [[NTHTTPTileDataSource alloc]initWithMinZoom:0 maxZoom:18 baseURL:url];
    NTRasterTileLayer *layer = [[NTRasterTileLayer alloc]initWithDataSource:source];

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item">
  {% highlight html %}

  COMING SOON...

  {% endhighlight %}
  </div>
  
</div>

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

Free for limited use. See a description of the [_Tile usage policy_](http://wiki.openstreetmap.org/wiki/Tile_usage_policy).
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

<a href="http://www.arcgis.com/home/item.html?id=10df2279f9684e4a9f6a7f08febac2a9">Terms and more info</a>. ArcGIS&trade; Developer service subscription is required.

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

### Raster Tile References

For general information about raster tiles and geospatial data, see the following Open Source Geospatial Foundation reference materials 

- [OSM Wiki TMS page](http://wiki.openstreetmap.org/wiki/TMS)
- [OSM Featured tiles](http://wiki.openstreetmap.org/wiki/Featured_tiles)


# Clusters

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

## API Methods for Clusters

Clusters are generated dynamically, based on `VectorDataSource` data that loads the map layer. If using an API, it works as a unique layer with the `ClusteredVectorLayer` method, and includes the following parameters in the a hierarchal order:

1. Select the layer `DataSource`

  In most cases,  the `LocalVectorDataSource` function contains all the elements to request the data. It is important that the DataSource displays all elements in a layer, and does not limit it to the current map visualization bbox (bounding box)

2. `ClusterElementBuilder` defines a single method `buildClusterElement`

### Define data and layer

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
  {% highlight html %}

        var proj = new EPSG3857();

      // 5. Create overlay layer for markers
      var dataSource = new LocalVectorDataSource(proj);

      // 6. Create Marker objects and add them to vectorDataSource.
      // **Note:** This depends on the _app type_ of your mobile app settings. See samples with JSON loading

      var layer = new ClusteredVectorLayer(dataSource, new MyClusterElementBuilder());
      layer.MinimumClusterDistance = 20; // in pixels

      MapView.Layers.Add(layer);

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item">
  {% highlight html %}

      // 7. Initialize a local vector data source
    NTProjection* proj = [[mapView getOptions] getBaseProjection];
    NTLocalVectorDataSource* vectorDataSource = [[NTLocalVectorDataSource alloc] initWithProjection:proj];
    
    // 8. Create Marker objects and add them to vectorDataSource.
    // **Note:** This depends on the _app type_ of your mobile app settings. See AdvancedMap for samples with JSON loading and random point generation
    
    // 9. Create element builder
    MyMarkerClusterElementBuilder* clusterElementBuilder = [[MyMarkerClusterElementBuilder alloc] init];
    
    // 10. Initialize a vector layer with the previous data source
    NTClusteredVectorLayer* vectorLayer = [[NTClusteredVectorLayer alloc] initWithDataSource:vectorDataSource clusterElementBuilder:clusterElementBuilder];
    
    // 11. Add the previous vector layer to the map
    [[mapView getLayers] add:vectorLayer];

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item">
  {% highlight html %}

  COMING SOON...

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
  {% highlight html %}

  private class MyClusterElementBuilder extends ClusterElementBuilder {

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
  {% highlight html %}

    public class MyClusterElementBuilder : ClusterElementBuilder
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
  {% highlight html %}

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
  {% highlight html %}

  COMING SOON...

  {% endhighlight %}
  </div>
  
</div>


## MBTiles for Map Data

MBTiles contain the TileJSON formats that include basic map styling for Mapbox. [MBTiles](http://mapbox.com/developers/mbtiles/) support is included in CARTO Mobile SDK. MBTiles uses the SQLite database format, which only requires one file to be downloaded and managed.

### Raster Maps

The following tools enable you to create MBTile packages:

- [MapTiler](http://www.maptiler.com/) is a utility to create MBTiles from raster geo files (GeoTIFF, JPG, ECW, and so on)

* [TileMill](http://mapbox.com/tilemill/) is an open source generator of nice map packages for vector geo files, such as Shapefile or PosgGIS geo data
- [MOBAC](http://mobac.sourceforge.net) is available to download from variety of free sources, such as Bing, OpenStreetMap, and so on. You can even load it from WMS with added configuration

- [MBUtil](https://github.com/mapbox/mbutil) enables you to create mbtiles from/to TMS-style tile folders, created with different utilities, such as GDAL utility

- [Portable Basemap Server](https://geopbs.codeplex.com/) is a free utility for Windows and loads data from various commercial servers and custom sources. It is also available in ESRI formats. It works mainly as WMTS server, but can create MBTiles as an extra feature

### Vector Maps

The following vector map tools enable you to create MBTiles:

- Nutiteq SDK bundled [Package Manager API](/guides/offline-maps) downloads country-wide, or bounding box based, map packages with **OpenStreetMap data**. These files are not directly visible, as Package Manager downloads the files, you only use the API.

_**Note:** If you need to create vector map files **from different sources, or with your own data** (e.g. Shapefiles), please contact [CARTO support](mailto:carto@support.com)._ Our Mobile Developers will provide you with a free converter toolchain for custom conversions.

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

### Ground Overlay Code Samples

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
  {% highlight html %}

    com.carto.graphics.Bitmap overlayBitmap = BitmapUtils.loadBitmapFromAssets("jefferson-building-ground-floor.jpg");

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
    bitmapPoses.add(new ScreenPos(0, overlayBitmap.getHeight()));
    bitmapPoses.add(new ScreenPos(overlayBitmap.getWidth(), overlayBitmap.getHeight()));
    bitmapPoses.add(new ScreenPos(overlayBitmap.getWidth(), 0));

    // 1. Create bitmap overlay raster tile data source
    BitmapOverlayRasterTileDataSource rasterDataSource = new BitmapOverlayRasterTileDataSource(0, 20, overlayBitmap, proj, mapPoses, bitmapPoses);
    RasterTileLayer rasterLayer = new RasterTileLayer(rasterDataSource);
    mapView.getLayers().add(rasterLayer);

    // 2. Apply zoom level bias to the raster layer
    // - By default, bitmaps are upsampled on high-DPI screens
    // 3. Correct this by applying appropriate bias
    float zoomLevelBias = (float) (Math.log(mapView.getOptions().getDPI() / 160.0f) / Math.log(2));
    rasterLayer.setZoomLevelBias(zoomLevelBias * 0.75f);
    rasterLayer.setTileSubstitutionPolicy(TileSubstitutionPolicy.TILE_SUBSTITUTION_POLICY_VISIBLE);

    mapView.setFocusPos(pos, 0);
    mapView.setZoom(16f, 0);

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item">
  {% highlight html %}

  var overlayBitmap = BitmapUtils.LoadBitmapFromAssets("jefferson-building-ground-floor.jpg");

  // 4. Create two vector geographical positions, and corresponding raster image pixel coordinates
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

  // 5. Create bitmap overlay raster tile data source
  var rasterDataSource = new BitmapOverlayRasterTileDataSource(0, 20, overlayBitmap, proj, mapPoses, bitmapPoses);
  var rasterLayer = new RasterTileLayer(rasterDataSource);
  MapView.Layers.Add(rasterLayer);

  // 6. Apply zoom level bias to the raster layer
  // - By default, bitmaps are upsampled on high-DPI screens
  // 7. Correct this by applying appropriate bias
  float zoomLevelBias = (float)(Math.Log(MapView.Options.DPI / 160.0f) / Math.Log(2));
  rasterLayer.ZoomLevelBias = zoomLevelBias * 0.75f;
  rasterLayer.TileSubstitutionPolicy = TileSubstitutionPolicy.TileSubstitutionPolicyVisible;

  MapView.SetFocusPos(pos, 0);
  MapView.SetZoom(16f, 0);

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item">
  {% highlight html %}

    // 8. Load ground overlay bitmap
    NTBitmap *overlayBitmap = [NTBitmapUtils loadBitmapFromAssets:@"jefferson-building-ground-floor.jpg"];
    
    // 9. Create two vector geographical positions, and corresponding raster image pixel coordinates
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
    
    // 10. Create bitmap overlay raster tile data source
    NTBitmapOverlayRasterTileDataSource* rasterDataSource = [[NTBitmapOverlayRasterTileDataSource alloc] initWithMinZoom:0 maxZoom:20 bitmap:overlayBitmap projection:proj mapPoses:mapPoses bitmapPoses:bitmapPoses];
    NTRasterTileLayer* rasterLayer = [[NTRasterTileLayer alloc] initWithDataSource:rasterDataSource];
    [[mapView getLayers] add:rasterLayer];
    
    // 11. Apply zoom level bias to the raster layer
    // - By default, bitmaps are upsampled on high-DPI screens
    // 12. Correct this by applying appropriate bias
    float zoomLevelBias = log([[mapView getOptions] getDPI] / 160.0f) / log(2);
    [rasterLayer setZoomLevelBias:zoomLevelBias * 0.75f];
    [rasterLayer setTileSubstitutionPolicy:NT_TILE_SUBSTITUTION_POLICY_VISIBLE];
    
    [mapView setFocusPos:pos durationSeconds:0];
    [mapView setZoom:16f durationSeconds:0];

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item">
  {% highlight html %}

  COMING SOON...

  {% endhighlight %}
  </div>
  
</div>


## Vector Styles

Usage of vector-based base maps enables you to re-style maps according to your needs. You can set the colors, transparency, line styles (width, patterns, casings, endings), polygon patterns, icons, text placements, fonts and many other vector data parameters. 

CARTO uses Mapnik (http://mapnik.org) XML style description language for customizing the visual style of vector tiles. Our styling is optimized for mobile and contain some unique style parameters, while ignoring some others. In general, you can reuse your existing Mapnik XML, or CartoCSS, styling files and tools (such as TileMill/Mapbox Studio).

Vector styling is applied in the mobile client, where the style files are bundled with the application installer. The application can change the styling anytime, without reloading vector map data. This enables your to download map data once, and change styling from "day mode" to "night mode" with no new downloads.

CARTO map rendering implementation is intended for real-time rendering. As a result, several limitations apply.

### Ready-made Styles

See [Mobile Styles](/mobileStyles) for pre-packaged styles used by CARTO. These are compatible with our vector tile sources.

### Mapnik Style Format

Mapnik map style definition is a common file format for map styles, based on XML. It was originally created for Mapnik, but is used by other software, such as our Mobile SDK. File format specification is located in this [XMLConfigReference](https://github.com/mapnik/mapnik/wiki/XMLConfigReference) document. There are several ways you can apply these styles:

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

### Creating Style Package

CARTO vector styles are distributed as zip-archives. All style-related files and folders must be placed into a single zip file.

The most important part of the style is the style definition file, typically named _project.xml_. This file contains style descriptions for all layers, and usually references other files, such as fonts, icons, and pattern bitmaps (which should be placed in various subfolders).

### Performance Hints 

The following recommendations are suggested for the best performance with vector styles:

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

The following lists contains all supported symbolizers and parameters:

- `PointSymbolizer`: file, opacity, allow-overlap, ignore-placement, transform

- `BuildingSymbolizer`: fill, fill-opacity, height, geometry-transform

- `LineSymbolizer`: stroke, stroke-width, stroke-opacity, stroke-linejoin, stroke-linecap, stroke-dasharray, geometry-transform

- `LinePatternSymbolizer`: file, fill, opacity, geometry-transform

- `PolygonSymbolizer`: fill, fill-opacity, geometry-transform

- `PolygonPatternSymbolizer`: file, fill, opacity, geometry-transform

- `MarkersSymbolizer`: file, placement, marker-type, fill, opacity, width, height, spacing, allow-overlap, ignore-placement, transform

- `TextSymbolizer`: name, face-name, fontset-name, placement, size, spacing, fill, opacity, halo-fill, halo-opacity, halo-radius, allow-overlap, minimum-distance, text-transform, orientation, dx, dy, wrap-width, wrap-before, character-spacing, line-spacing, horizontal-alignment, vertical-alignment

- `ShieldSymbolizer`: name, face-name, fontset-name, placement, size, spacing, fill, opacity, halo-fill, halo-opacity, halo-radius, allow-overlap, minimum-distance, text-transform, orientation, dx, dy, wrap-width, wrap-before, character-spacing, line-spacing, horizontal-alignment, vertical-alignment, file, shield-dx, shield-dy, unlock-image

### CartoCSS Extensions

Metavariables and nutiparameters are also available as CartoCSS style extensions.
