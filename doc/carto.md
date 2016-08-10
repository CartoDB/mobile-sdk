# CARTO maps with Mobile SDK

## Introduction

Mobile SDK can be used as stand-alone maps SDK, or as client for CARTO platform. Here we cover the CARTO platform integration options.


### High-level API to CARTO services

CARTO Mobile SDK since 4.0.0 provides special high-level API to connect to CARTO services. When you use these services, then CARTO layers will be automatically reconfigured to use vector rendering in the mobile client side, intead of rasters what are normally used in web.

1. Load *viz.json* visualizations: **CartoVisLoader** and **CartoVisBuilder** - this configures whole map view as set of Layers based on your Shared map, while giving some methods to override and finetune map to mobile specifics.
2. Use *Maps API* for anonymous or named maps: **CartoMapsService** - useful for finetuned single map layers
3. Use *SQL API* for accessing database: **CartoSQLService**

### Low-level usage of CARTO API-s

Mobile SDK has also methods to connect to CARTO APIs by their URL end-points. 

1. Use **raster map tiles**, just define the tile URL for RasterTileLayer.
1. To get **interactivity** (object click data) you can use the UTFGrid. This uses both raster map tiles and json-based UTF tiles. On the CARTO Builder side, you need to enable and define tooltips.
1. SDK can load data as **vector tiles**. The CARTO platform has support to provide Mapbox Vector Tile (MVT) format tiles, and mobile SDK can render on the client side. To use them you also need CartoCSS styling, and this is also provided by CARTO API-s. This provides many advantages, from zooming and rotation of maps to data that can be packaged as offline using mbtiles.
1. **Loading GeoJSON vector data** is useful when you need more deep interactivity (object click actions) or dynamic client-side styling of the objects. For vector data, CARTO Engine provides an SQL API and app that can load whole tables and render on maps. You can also use client-side simplification and clustering.
1. If the **data table is large** (more than a few thousand objects), then loading whole tables can be too much for the mobile client. Instead, you can use smart, on-demand, view-based loading of vector data. Similar to the SQL API and GeoJSON format used from the CARTO Engine side, the SDK will use custom vector data sources to consume it. This loads an only a selected visible area of the map. The app can control zoom levels, server-side generalizations, or simplification can be applied.
1. For point-based time-series visualizations, you will want to use **Torque**. This provides animated rendering, and mobile SDK has a special layer *TorqueTileLayer* for it. From the API point of view, it uses the SQL API and CartoCSS styling, but Torque has an additional time control method.

![CARTO integations](https://carto.com/blog/img/posts/2016-03-31-nutiteq3/scheme.b36b8fa7.png)

## Showing map from viz.json
### Preparation

A common data flow would be following:

1. You upload your map data to CARTO platform
2. Create a Map with Builder (or Editor) - you can have single or several layer, use different create thematic maps etc. You can preview in the web.
3. Use **Share** button which gives you URL ending with viz.json - this defines your map visualization, and same URL can be used in your custom webapp via CartoDB.js SDK, and in your custom mobile apps using CARTO Mobile SDK.

Note: CARTO Mobile SDK does not load deep-insight vizards yet, it deals with map part only. This will come a bit later.

### App integration

Viz.json loading is not a simple method call, but a callback-based asynchronous process. This is needed for several things:

* Online API is requested and this has to run always in another thread
* Sometimes you want to customize how layers are added, make them a bit different than in web, maybe skip a layer etc
* Some viz.json elements, like Overlays (Legend, Copyright etc) are not handled by SDK, so your app will get the data in special callback and can add these elements according to your app design requirements.


If you prepared (and shared) a Map in CARTO platform, then you can load it to the mobile. Use **CartoVisBuilder** interface to implement this callback. Note that it has to run in another thread, as it does online connections.

CartoVisBuilder works as a parser for visualization, so you define callbacks for key elements of visualization to implement key data in the viz.json, like adding map layers (most important), also you may want to set map center and zoom. This way you can control which aspects of Map are loaded in mobile, and can change some layer interpretation aspects, as sometimes you want your map to be a bit different on mobile, for visual or technical reasons. 

1. To make sure your app has all fonts CARTO uses add **carto-fonts.zip** file to your project. You can download [carto-fonts.zip](https://github.com/CartoDB/mobile-ios-samples/raw/master/AdvancedMap/Assets/carto-fonts.zip) from our iOs sample project. This is not needed if your Map does not have any texts.

2. A typical/minimal _CartoVisBuilder_ implementation, you can append it to your main ViewController.mm

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
  {% highlight java %}

private class MyCartoVisBuilder extends CartoVisBuilder {

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
            // Get JSON for remaining elements, like Overlays, Wizards, metadata etc
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
	    {% highlight objectiveC %}

	  @interface MyCartoVisBuilder : NTCartoVisBuilder

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



3. Load CARTO vizualisation. As it has online connection, then make sure it runs on background thread:

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
  {% highlight java %}

  private String visJSONURL = "http://documentation.carto.com/api/v2/viz/2b13c956-e7c1-11e2-806b-5404a6a683d5/viz.json";

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
  {% highlight objectiveC %}
  
 dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
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

4. Run your app and it should show this map (if you use sample URL):

![](https://raw.githubusercontent.com/CartoDB/mobile-ios-samples/master/sample_viz_mobile.png)



## Samples

CARTO sample projects have number of working samples for all the platforms:

* CartoVisJsonActivity - load complete map configurations (from online viz.json)
* CartoSQLActivity -  vector data via SQL API
* CartoTorqueActivity for Torque tiles
* CartoUTFGridActivity for raster tiles and UTFGrid
* CartoVectorTileActivity for vector tiles with CartoCSS styling 
* CartoRasterTileActivity for raster tiles with CartoCSS styling 

