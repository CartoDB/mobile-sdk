# Managing Mobile Maps with CARTO APIs

You can automatically manage some features of your mobile maps by integrating with other CARTO APIs. This enables you to use the Mobile SDK as a high-level API, or define URL endpoints to manage mobile maps.

## Mobile SDK as Service

By using the Mobile SDK as its own engine, you can automatically manage mobile map rendering from the mobile client. This involves some additional, unique Mobile SDK API parameters to be included with the CARTO Engine CARTO.js, Maps API, and SQL API requests.

1) Load a [CARTO.js](/docs/carto-engine/carto-js/) *viz.json* visualization for managing mobile layers

  - `CartoVisLoader` is used to load and configure all corresponding layers
  - `CartoVisBuilder` is high level interface for loading VisJSON configurations. You can define which mobile layers are configured and visualized by default

  **Tip:** For additional viz.json mobile map rendering, see [Displaying a Mobile Map from vis.json](#display-a-mobile-map-from-a-vizjson).

2) Integrate with the [Maps API](/docs/carto-engine/maps-api/) for anonymous or named maps

  `CartoMapsService` is a mobile service that can be used to automatically configure layers using anonymous map configurations, or by using parametrized named maps

3) Integrate with the [SQL API](/docs/carto-engine/sql-api/) for accessing database

  `CartoSQLService` is a high-level interface for CARTO SQL Service. The mobile service can be used to query data from CARTO databases using explicit SQL queries

## Manage Mobile Maps by Endpoint URLs

You can also manage mobile maps by defining mobile specific-features in the URL end-points for CARTO APIs. For example:

- To use **raster map tiles**, define the tile URL for RasterTileLayer

- To apply **interactivity** (object click data), use UTFGrid. This uses both raster map tiles and json-based UTF tiles

  **Tip:** For CARTO Builder, you will need to enable and define tooltips with the [POP-UP](/docs/carto-builder/map-layers-for-rendering-data/#pop-up) feature

- Load **vector tiles**, the CARTO Engine supports Mapbox Vector Tile (MVT) format tiles, which the Mobile SDK can render on the client side. You will also need [CartoCSS](https://carto.com/docs/carto-engine/cartocss/) styles to view vector tiles. This is useful for applying advanced styling features, such as zooming and rotating maps based on data that can be packaged for offline line, using mbtiles

- **Load GeoJSON vector data**. This is useful if you need need more advanced interactivity (object click actions) or dynamic client-side styling of the objects. For vector data, the CARTO Engine provides a [SQL API](/docs/carto-engine/sql-api/) and mobile app that can load entire tables and render maps. You can also use client-side simplification and clustering

- If the **data table is large** (more than a few thousand objects), then loading entire tables can overload the mobile client. Alternatively, use on-demand, view-based loading of vector data. Similar to the SQL API and GeoJSON format used on the CARTO Engine side, the SDK applies custom vector data sources to load data. _Only a selected, visible area, of the map will load._ The mobile app can control zoom levels, server-side generalizations, and simplifications can be applied

- For point-geometry time-series visualizations, use the _Animated_ aggregation to define Torque maps. This provides animated rendering, and the Mobile SDK has a special layer `TorqueTileLayer` to define this. From an API standpoint, Torque uses the SQL API and CartoCSS styling, but Torque contains an additional [time control method](/docs/carto-engine/torque-js/)

![CARTO integrations](https://carto.com/blog/img/posts/2016-03-31-nutiteq3/scheme.b36b8fa7.png)

## Publish a Mobile SDK Map

This high-level workflow describes how to prepare your mobile data for rendering, using the Import API and CARTO.js.

1) Upload your map data using the [Import API](/docs/carto-engine/import-api/)

2) Create a map with the [CARTO Builder](/docs/carto-builder/)

3) Click _SHARE_ to [publish your map](/docs/carto-builder/publishing-and-sharing-maps/#publish-options)

4) Select _CARTO Mobile SDK_ as the publish option

  This publishing option provides a URL to the mobile viz.json file, which is required if you are using the CARTO Mobile SDK to publish custom maps for Android, iOS, and Windows platforms

  **Note:** [Widgets](/docs/carto-builder/interactive-map-widgets/#interactive-map-widgets) are currently not supported for the CARTO Mobile SDK format, but will be available in a future release.

### App Integration

In order to integrate your published mobile map into the app, you need a callback-based asynchronous process to request the viz.json  (a simple method call will not suffice).

1)  The following requirements are needed for app integration of a published CARTO Mobile SDK map:

  - Online API is requested and will run in another thread

  - In certain cases, you will need to customize how layers are added in mobile compared to how they appear on the desktop version of a map

  - Some viz.json elements, such as Overlays (Legend, Copyright, and so on) are not handled by Mobile SDK so your mobile app gets the data using a different callback method, which can then be added as elements according to your app design requirements

2) If you published a map through a CARTO API, you can load it to a mobile app. Use `CartoVisBuilder` interface to implement this callback

  **Note:** This type of request must run in another thread, as it relies on an online connections.

  `CartoVisBuilder` works as a parser for visualization, so if you define callbacks for key elements of visualization to implement key data in the viz.json (such as adding map layers), you may want to set the map center and zoom level

  This enables you to control which aspects of your mobile map are rendered. You can change some layer interpretation aspects for mobile, as it may be required for the mobile map to appear differently. 

3) Ensure your mobile app has all CARTO fonts installed in your mobile project folder

  **Tip** You can download [carto-fonts.zip](https://github.com/CartoDB/mobile-ios-samples/raw/master/AdvancedMap/Assets/carto-fonts.zip) from our iOs sample project. Note that these assets are not needed if your map does not contains any text.

4) The typical (minimal) request for `_CartoVisBuilder_` implementation requires the following, which you can append from your main ViewController.mm

<div class="js-TabPanes">
  <ul class="Tabs">
    <li class="Tab js-Tabpanes-navItem is-active">
      <a href="#/0" class="js-Tabpanes-navLink">Java</a>
    </li>
    <li class="Tab js-Tabpanes-navItem">
      <a href="#/2" class="js-Tabpanes-navLink">Objective-C</a>
    </li>
  </ul>

  <div class="Carousel-item js-Tabpanes-item is-active">
  {% highlight java %}private class MyCartoVisBuilder extends CartoVisBuilder {

        @Override
        public void setCenter(MapPos mapPos) {
            mapView.setFocusPos(mapView.getOptions().getBaseProjection().fromWgs84(mapPos), 1.0f);
        }

        @Override
        public void setZoom(float zoom) {
            mapView.setZoom(zoom, 1.0f);
        }

        @Override
        public void setDescription(Variant variant) {
            // Get JSON for remaining elements, like Overlays, Wizards, metadata etc.
            // Variant is a JSON-like general structure in CARTO Mobile SDK
        }

        @Override
        public void addLayer(Layer layer, Variant attributes) {
            // Add the layer to the map view, ignore attributes here
            mapView.getLayers().add(layer);
        }
    }
  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item">
      {% highlight objectiveC %}@interface MyCartoVisBuilder : NTCartoVisBuilder

    @property NTMapView* mapView;

    @end

    @implementation MyCartoVisBuilder

    // methods to set map center and zoom based on defined map
    -(void)setCenter:(NTMapPos *)mapPos
    {
        [self.mapView setFocusPos:[[[self.mapView getOptions] getBaseProjection] fromWgs84:mapPos] durationSeconds:1.0f];
    }

    -(void)setZoom:(float)zoom
    {
        [self.mapView setZoom:zoom durationSeconds:1.0f];
    }

    // Add a layer to the map view
    -(void)addLayer:(NTLayer *)layer attributes:(NTVariant *)attributes
    {
        [[self.mapView getLayers] add:layer];
    }

    // Add a layer to the map view - most important work
    -(void)addLayer:(NTLayer *)layer attributes:(NTVariant *)attributes
    {
        [[self.mapView getLayers] add:layer];
    }

    @end

      {% endhighlight %}
 
  </div>
    
</div>



5) Load the CARTO visualization

  As it uses an online connection, ensure it runs on a background thread:

<div class="js-TabPanes">
  <ul class="Tabs">
    <li class="Tab js-Tabpanes-navItem is-active">
      <a href="#/0" class="js-Tabpanes-navLink">Java</a>
    </li>
    <li class="Tab js-Tabpanes-navItem">
      <a href="#/2" class="js-Tabpanes-navLink">Objective-C</a>
    </li>
  </ul>

  <div class="Carousel-item js-Tabpanes-item is-active">
  {% highlight java %}private String visJSONURL = "http://documentation.carto.com/api/v2/viz/2b13c956-e7c1-11e2-806b-5404a6a683d5/viz.json";

  Thread thread = new Thread(new Runnable() {
            @Override
            public void run() {
                mapView.getLayers().clear();

                // Create overlay layer for popups
                Projection proj = mapView.getOptions().getBaseProjection();
                LocalVectorDataSource dataSource = new LocalVectorDataSource(proj);
                VectorLayer vectorLayer = new VectorLayer(dataSource);

                // Create VIS loader
                CartoVisLoader loader = new CartoVisLoader();
                loader.setDefaultVectorLayerMode(true);
                MyCartoVisBuilder visBuilder = new MyCartoVisBuilder(vectorLayer);
                try {
                    loader.loadVis(visBuilder, visJSONURL);
                }
                catch (IOException e) {
                    Log.e(Const.LOG_TAG, "Exception: " + e);
                }

                // Add the created popup overlay layer on top of all visJSON layers
                mapView.getLayers().add(vectorLayer);
            }
        });
        thread.start(); 

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item">
  {% highlight objectiveC %}dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
        // just make sure no other layers are on map
        [[mapView getLayers] clear];
        
        // Create VIS loader
        NTCartoVisLoader* loader = [[NTCartoVisLoader alloc] init];
        
        // Load fonts package, this has all fonts you may need.
        [loader setVectorTileAssetPackage:[[NTZippedAssetPackage alloc] initWithZipData:[NTAssetUtils loadAsset:@"carto-fonts.zip"]]];
        [loader setDefaultVectorLayerMode:YES];
        MyCartoVisBuilder* visBuilder = [[MyCartoVisBuilder alloc] init];
        visBuilder.mapView = mapView;

        // Use your Map URL in next line, you get it from Share Map page. Here is a basic working sample:
        [loader loadVis:visBuilder visURL:@"http://documentation.carto.com/api/v2/viz/2b13c956-e7c1-11e2-806b-5404a6a683d5/viz.json"];
        
    });
      {% endhighlight %}
 
  </div>
</div>

6) Run your mobile app

  If you are using assets from the [sample apps](/docs/carto-engine/mobile-sdk/sample-apps/) as part of your project, the following [map result](https://raw.githubusercontent.com/CartoDB/mobile-ios-samples/master/sample_viz_mobile.png) appears.

## SDK and CARTO API Samples

The CARTO sample projects have number of working samples for all the mobile platforms:

- `CartoVisJsonActivity` load complete map configurations (from online viz.json)

- `CartoSQLActivity`  vector data via SQL API

- `CartoTorqueActivity` for Torque tiles

- `CartoUTFGridActivity` for raster tiles and UTFGrid

- `CartoVectorTileActivity` for vector tiles with CartoCSS styling 

- `CartoRasterTileActivity` for raster tiles with CartoCSS styling 
