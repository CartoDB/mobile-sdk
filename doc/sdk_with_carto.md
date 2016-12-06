# SDK and other CARTO Tools

For select account plans, you can connect to the CARTO Engine APIs via Mobile SDK, to retrieve map visualizations and table data from your CARTO account. _API access is not available for free users._ [Contact us](mailto:sales@carto.com) for questions about your account plan and enabling this feature.

## SDK and CARTO APIs

If you are using other CARTO Engines, you can automatically define mobile map feature for rendering directly from the client-side. This involves some additional, unique Mobile SDK API parameters to be included with the CARTO Engine CARTO.js, Maps API, and SQL API requests.

1) Load a [CARTO.js](/docs/carto-engine/carto-js/) *viz.json* visualization for managing mobile layers

  - `CartoVisLoader` is used to load and configure all corresponding layers
  - `CartoVisBuilder` is high level interface for loading VisJSON configurations. You can define which mobile layers are configured and visualized by default

  **Tip:** For additional viz.json mobile map rendering, see how to [Publish a Mobile Map](#publish-a-mobile-map).

2) Integrate with the [Maps API](/docs/carto-engine/maps-api/) for anonymous or named maps

  `CartoMapsService` is a mobile service that can be used to automatically configure layers using anonymous map configurations, or by using parametrized named maps

3) Integrate with the [SQL API](/docs/carto-engine/sql-api/) for accessing database

  `CartoSQLService` is a high-level interface for the CARTO SQL Service. The mobile service can be used to query data from CARTO databases using explicit SQL queries. _Note that this is only available for Public datasets._

## Loading CARTO Map Data to Mobile Apps

You can also manage mobile rendering by defining how mobile features load with the CARTO APIs and with the CARTO Builder. For example:

- To use **raster map tiles**, define the tile URL for RasterTileLayer

- To apply **interactivity** (object click data), use UTFGrid. This uses both raster map tiles and json-based UTF tiles

  **Tip:** For CARTO Builder, you will need to enable and define tooltips with the [POP-UP](/docs/carto-builder/map-layers-for-rendering-data/#pop-up) feature

- Load **vector tiles**, the CARTO Engine supports Mapbox Vector Tile (MVT) format tiles, which the Mobile SDK can render on the client side. You will also need [CartoCSS](https://carto.com/docs/carto-engine/cartocss/) styles to view vector tiles. This is useful for applying advanced styling features, such as zooming and rotating maps based on data that can be packaged for offline line, using mbtiles

- **Load GeoJSON vector data**. This is useful if you need need more advanced interactivity (object click actions) or dynamic client-side styling of the objects. For vector data, the CARTO Engine provides a [SQL API](/docs/carto-engine/sql-api/) and mobile app that can load entire tables and render maps. You can also use client-side simplification and clustering

- If the **data table is large** (more than a few thousand objects), then loading entire tables can overload the mobile client. Alternatively, use on-demand, view-based loading of vector data. Similar to the SQL API and GeoJSON format used on the CARTO Engine side, the SDK applies custom vector data sources to load data. _Only a selected, visible area, of the map will load._ The mobile app can control zoom levels, server-side generalizations, and simplifications can be applied

- For point-geometry time-series visualizations, use the _Animated_ aggregation to define Torque maps. This provides animated rendering, and the Mobile SDK has a special layer `TorqueTileLayer` to define this. From an API standpoint, Torque uses the SQL API and CartoCSS styling, but Torque contains an additional [time control method](/docs/carto-engine/torque-js/)

## Publish a Mobile Map

This high-level workflow describes how to prepare your mobile data for rendering, using the Import API and CARTO Builder.

1) Upload your map data using the [Import API](/docs/carto-engine/import-api/)

2) Create a map with the [CARTO Builder](https://carto.com/learn/guides)

3) Click _SHARE_ to publish your map

4) Select _CARTO Mobile SDK_ as the publish option

  This publishing option provides a URL to the mobile viz.json file, which is required if you are using the CARTO Mobile SDK to publish custom maps for Android, iOS, and Windows platforms.

  **Note:** [Widgets](https://carto.com/learn/guides/widgets/exploring-widgets) are currently not supported for the CARTO Mobile SDK format, but will be available in a future release.

## App Integration

You can use the following CARTO APIs for managing your mobile app.

### Maps API

CARTO Mobile SDK supports [Maps API](https://carto.com/docs/carto-engine/maps-api/) integration for Anonymous Maps and Named Maps. Anonymous maps allow you to instantiate a map given SQL and CartoCSS. Named Maps are essentially the same as Anonymous Maps except the MapConfig is stored on the server, and the map is given a unique name.

#### Building an Anonymous map config (SQL and CartoCSS)

Use CARTO Maps service class to configure layers. Note that this must be done in a separate thread on Android, as Maps API requires connecting to server, which is not allowed in the main thread.

The following snippet sets up the config (SQL and CartoCSS) we are going to use in our query

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
</ul>

<div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--java is-active">
{% highlight java %}

    private String getConfigJson() {

        // Define server config
        JSONObject configJson = new JSONObject();

        try {
            // Change these according to your DB
            String sql = "select * from stations_1";
            String statTag = "3c6f224a-c6ad-11e5-b17e-0e98b61680bf";
            String[] columns = new String[] { "name", "status", "slot" };
            String cartoCSS = "#stations_1{" +
                    "marker-fill-opacity:0.9;marker-line-color:#FFF;" +
                    "marker-line-width:2;marker-line-opacity:1;marker-placement:point;" +
                    "marker-type:ellipse;marker-width:10;marker-allow-overlap:true;}\n" +
                    "" +
                    "#stations_1[status = 'In Service']{marker-fill:#0F3B82;}\n" +
                    "#stations_1[status = 'Not In Service']{marker-fill:#aaaaaa;}\n" +
                    "#stations_1[field_9 = 200]{marker-width:80.0;}\n" +
                    "#stations_1[field_9 <= 49]{marker-width:25.0;}\n" +
                    "#stations_1[field_9 <= 38]{marker-width:22.8;}\n" +
                    "#stations_1[field_9 <= 34]{marker-width:20.6;}\n" +
                    "#stations_1[field_9 <= 29]{marker-width:18.3;}\n" +
                    "#stations_1[field_9 <= 25]{marker-width:16.1;}\n" +
                    "#stations_1[field_9 <= 20.5]{marker-width:13.9;}\n" +
                    "#stations_1[field_9 <= 16]{marker-width:11.7;}\n" +
                    "#stations_1[field_9 <= 12]{marker-width:9.4;}\n" +
                    "#stations_1[field_9 <= 8]{marker-width:7.2;}\n" +
                    "#stations_1[field_9 <= 4]{marker-width:5.0;}";

            // You may not need to change much of these standards
            configJson.put("version", "1.0.1");
            configJson.put("stat_tag", statTag);

            JSONArray layersArrayJson = new JSONArray();
            JSONObject layersJson = new JSONObject();
            layersJson.put("type", "cartodb");

            JSONObject optionsJson = new JSONObject();
            optionsJson.put("sql", sql);
            optionsJson.put("cartocss", cartoCSS);
            optionsJson.put("cartocss_version", "2.1.1");

            JSONArray interactivityJson = new JSONArray();
            interactivityJson.put("cartodb_id");

            optionsJson.put("interactivity", interactivityJson);

            JSONObject attributesJson = new JSONObject();
            attributesJson.put("id", "cartodb_id");

            JSONArray columnsJson = new JSONArray();

            for (String col : columns) {
                columnsJson.put(col);
            }

            attributesJson.put("columns", columnsJson);
            optionsJson.put("attributes", attributesJson);
            layersJson.put("options", optionsJson);
            layersArrayJson.put(layersJson);
            configJson.put("layers", layersArrayJson);

        } catch (JSONException e) {
            return  null;
        }

        return configJson.toString();
    }

{% endhighlight %}
</div>

<div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--csharp">
{% highlight csharp %}

    static string CartoCSS
    {
      get
      {
        return "#stations_1{" +
              "marker-fill-opacity:0.9;" +
              "marker-line-color:#FFF;" +
              "marker-line-width:2;" +
              "marker-line-opacity:1;" +
              "marker-placement:point;" +
              "marker-type:ellipse;" +
              "marker-width:10;" +
              "marker-allow-overlap:true;" +
            "}" +
            "#stations_1[status = 'In Service']{marker-fill:#0F3B82;}" +
            "#stations_1[status = 'Not In Service']{marker-fill:#aaaaaa;}" +
            "#stations_1[field_9 = 200]{marker-width:80.0;}" +
            "#stations_1[field_9 <= 49]{marker-width:25.0;}" +
            "#stations_1[field_9 <= 38]{marker-width:22.8;}" +
            "#stations_1[field_9 <= 34]{marker-width:20.6;}" +
            "#stations_1[field_9 <= 29]{marker-width:18.3;}" +
            "#stations_1[field_9 <= 25]{marker-width:16.1;}" +
            "#stations_1[field_9 <= 20.5]{marker-width:13.9;}" +
            "#stations_1[field_9 <= 16]{marker-width:11.7;}" +
            "#stations_1[field_9 <= 12]{marker-width:9.4;}" +
            "#stations_1[field_9 <= 8]{marker-width:7.2;}" +
            "#stations_1[field_9 <= 4]{marker-width:5.0;}";
      }
    }

    public static JsonValue VectorLayerConfigJson
    {
      get
      {
        JsonObject json = new JsonObject();

        json.Add("version", "1.0.1");
        json.Add("stat_tag", "3c6f224a-c6ad-11e5-b17e-0e98b61680bf");

        JsonArray layerArray = new JsonArray();
        JsonObject layerJson = new JsonObject();

        layerJson.Add("type", "cartodb");

        JsonObject optionJson = new JsonObject();

        optionJson.Add("sql", "select * from stations_1");
        optionJson.Add("cartocss", CartoCSS);
        optionJson.Add("cartocss_version", "2.1.1");

        JsonArray interactivityJson = new JsonArray();
        interactivityJson.Add("cartodb_id");

        optionJson.Add("interactivity", interactivityJson);

        JsonObject attributesJson = new JsonObject();
        attributesJson.Add("id", "cartodb_id");

        JsonArray columnJson = new JsonArray { "name", "field_9", "slot" };

        attributesJson.Add("columns", columnJson);
        optionJson.Add("attributes", attributesJson);
        layerJson.Add("options", optionJson);
        layerArray.Add(layerJson);

        json.Add("layers", layerArray);

        return json;
      }
    }

{% endhighlight %}
</div>

<div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--objective-c">
{% highlight objc %}

- (NSString*) getConfig
{
    NSDictionary* options = @{
                              @"sql": @"select * from stations_1",
                              @"cartocss": [self getCartoCSS],
                              @"cartocss_version": @"2.1.1",
                              @"interactivity": [self getInteractivityJson],
                              @"attributes": [self getAttributesJson]
                              };

    NSMutableArray *layerArray = [[NSMutableArray alloc]init];
    NSDictionary *layerJson = @{ @"options": options, @"type": @"cartodb" };
    
    [layerArray addObject:layerJson];
    
    NSDictionary *json = @{
                           @"version": @"1.0.1",
                           @"stat_tag": @"3c6f224a-c6ad-11e5-b17e-0e98b61680bf",
                           @"layers": layerArray
                           };
    
    NSError *error;
    NSData *data = [NSJSONSerialization dataWithJSONObject:json options:0 error:&error];
    
    return [[NSString alloc] initWithData:data encoding:NSUTF8StringEncoding];
}

-(NSString *) getCartoCSS
{
    NSString *base = @"#stations_1 { marker-fill-opacity:0.9; marker-line-color:#FFF; marker-line-width:2; marker-line-opacity:1; marker-placement:point; marker-type:ellipse; marker-width:10; marker-allow-overlap:true; }";
    
    NSString *attributes = @"#stations_1[status = 'In Service'] { marker-fill:#0F3B82; } #stations_1[status = 'Not In Service'] { marker-fill:#aaaaaa; } #stations_1[field_9 = 200] { marker-width:80.0; } #stations_1[field_9 <= 49] { marker-width:25.0; } #stations_1[field_9 <= 38] { marker-width:22.8; } #stations_1[field_9 <= 34] { marker-width:20.6; } #stations_1[field_9 <= 29] { marker-width:18.3; } #stations_1[field_9 <= 25] { marker-width:16.1; } #stations_1[field_9 <= 20.5] { marker-width:13.9; } #stations_1[field_9 <= 16] { marker-width:11.7; } #stations_1[field_9 <= 12] { marker-width:9.4; } #stations_1[field_9 <= 8] { marker-width:7.2; } #stations_1[field_9 <= 4] { marker-width:5.0; } ";
    
    return [base stringByAppendingString:attributes];
}

-(NSMutableArray *) getInteractivityJson
{
    NSMutableArray *array = [[NSMutableArray alloc]init];
    [array addObject:@"cartodb_id"];
    return array;
}

-(NSDictionary *) getAttributesJson
{
    NSMutableArray *columns = [[NSMutableArray alloc]init];
    [columns addObject:@"name"];
    [columns addObject:@"field_9"];
    [columns addObject:@"slot"];
    
    return @{ @"id": @"cartodb_id", @"columns": columns };
}

{% endhighlight %}
</div>
</div>

Now that the config is set up, you can apply the actual query. This snippet is for querying data from an anonymous vector table (change the default vector layer mode to false if you are using a raster table).

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
</ul>

<div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--java is-active">
{% highlight java %}

final String config = getConfigJson();

// Use the Maps service to configure layers.
// Note that this must be done in a separate thread on Android,
// as Maps API requires connecting to server, which is not allowed in main thread.
Thread serviceThread = new Thread(new Runnable() {
  @Override
  public void run() {

    CartoMapsService mapsService = new CartoMapsService();
    mapsService.setUsername("nutiteq");
    mapsService.setDefaultVectorLayerMode(true); // use vector layers

            try {
                LayerVector layers = mapsService.buildMap(Variant.fromString(config));
                for (int i = 0; i < layers.size(); i++) {
                    mapView.getLayers().add(layers.get(i));
                }
            }
            catch (IOException e) {
                Log.e("EXCEPTION", "Exception: " + e);
            }
  }
});

serviceThread.start();

{% endhighlight %}
</div>

<div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--csharp">
{% highlight csharp %}

      JsonValue config = JsonUtils.VectorLayerConfigJson;

      MapView.ConfigureAnonymousVectorLayers(config);

      // Extension method in static class
      public static void ConfigureAnonymousVectorLayers(this MapView map, JsonValue config)
      {
        // Use the Maps service to configure layers. 
        // Note that this must be done in a separate thread on Android, 
        // as Maps API requires connecting to server, which cannot be done in the main thread.

        System.Threading.Tasks.Task.Run(delegate
        {
          CartoMapsService service = new CartoMapsService();
          service.Username = "nutiteq";

          // Use VectorLayers
          service.DefaultVectorLayerMode = true;
          service.Interactive = true;

          LayerVector layers = service.BuildMap(Variant.FromString(config.ToString()));

          for (int i = 0; i < layers.Count; i++)
          {
            map.Layers.Add(layers[i]);
          }
        });
      }


{% endhighlight %}
</div>

<div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--objective-c">
{% highlight objc %}

    NTCartoMapsService* mapsService = [[NTCartoMapsService alloc] init];
    
    [mapsService setUsername:@"nutiteq"];
    
    // Use vector layers, not raster layers
    [mapsService setDefaultVectorLayerMode:TRUE];
    [mapsService setInteractive:TRUE];
    
    NTVariant* variant = [NTVariant fromString:[self getConfig]];
    NTLayerVector *layers = [mapsService buildMap:variant];
    
    for (int i = 0; i < [layers size]; i++) {
        
        NTLayer* layer = [layers get:i];
        [[self.mapView getLayers]add:layer];
    }

{% endhighlight %}
</div>
</div>

#### If you have the name of a map, use that to query data, instead of providing the SQL or CartoCSS

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
</ul>

<div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--java is-active">
{% highlight java %}

    final CartoMapsService service = new CartoMapsService();

    // Use raster layers, not vector
    service.setDefaultVectorLayerMode(true);

    service.setUsername("nutiteq");

    final String name = "tpl_69f3eebe_33b6_11e6_8634_0e5db1731f59";

    // Be sure to make network queries on another thread
    Thread thread = new Thread(new Runnable() {
        @Override
        public void run() {
            try {

                LayerVector layers = service.buildNamedMap(name, new StringVariantMap());

                for (int i = 0; i < layers.size(); i++) {
                    Layer layer = layers.get(i);
                    mapView.getLayers().add(layer);
                }

            } catch (IOException e) {
                e.printStackTrace();
            }
        }
    });

    thread.start();

{% endhighlight %}
</div>

<div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--csharp">
{% highlight csharp %}


MapView.ConfigureNamedVectorLayers("tpl_69f3eebe_33b6_11e6_8634_0e5db1731f59");

public static void ConfigureNamedVectorLayers(this MapView map, string name)
{
  System.Threading.Tasks.Task.Run(delegate
  {
    CartoMapsService service = new CartoMapsService();
    service.Username = "nutiteq";

    // Use VectorLayers
    service.DefaultVectorLayerMode = true;

    LayerVector layers = service.BuildNamedMap(name, new StringVariantMap());

    for (int i = 0; i < layers.Count; i++)
    {
      map.Layers.Add(layers[i]);
    }
  });
}

{% endhighlight %}
</div>

<div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--objective-c">
{% highlight objc %}

NTCartoMapsService* mapsService = [[NTCartoMapsService alloc] init];

[mapsService setUsername:@"nutiteq"];

// Use raster layers, not vector layers
[mapsService setDefaultVectorLayerMode:YES];

NTLayerVector *layers = [mapsService buildNamedMap:@"tpl_69f3eebe_33b6_11e6_8634_0e5db1731f59" templateParams: [[NTStringVariantMap alloc] init]];

for (int i = 0; i < [layers size]; i++) {
    NTLayer* layer = [layers get:i];
    [[self.mapView getLayers]add:layer];
}

{% endhighlight %}
</div>
  
</div>

### SQL API

CARTO’s [SQL API](https://carto.com/docs/carto-engine/sql-api/) allows you to interact with your tables and data inside CARTO, as if you were running SQL statements against a normal database. You can use the SQL API to insert, update or delete data (i.e., insert a new column with a latitude and longitude data) or to select data from public tables in order to use it on your website or application (i.e., display the 10 closest records to a particular location).

#### Using CARTO SQL Service class to make a query

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
  </ul>

  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--java is-active">
  {% highlight java %}

  static final String query = "SELECT * FROM cities15000 WHERE population > 100000";

  final CartoSQLService service = new CartoSQLService();
  service.setUsername("nutiteq");

  // Be sure to make network queries on another thread
  Thread thread = new Thread(new Runnable() {
      @Override
      public void run() {
          try {
              features = service.queryFeatures(query, baseProjection);

              for (int i = 0; i < features.getFeatureCount(); i++) {

                  // This data set features point geometry,
                  // however, it can also be LineGeometry or PolygonGeometry

                  PointGeometry geometry = (PointGeometry)features.getFeature(i).getGeometry();
                  source.add(new Point(geometry, getPointStyle()));
              }

          } catch (IOException e) {
              e.printStackTrace();
          }
      }
  });

  thread.start();

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--csharp">
  {% highlight csharp %}

  const string query = "SELECT * FROM cities15000 WHERE population > 100000";

  CartoSQLService service = new CartoSQLService();
  service.Username = "nutiteq";

  PointStyleBuilder builder = new PointStyleBuilder
  {
    Color = new Carto.Graphics.Color(255, 0, 0, 255),
    Size = 1
  };

  MapView.QueryFeatures(service, source, builder.BuildStyle(), query);

  // Extension Method in a static class (shared code)
  public static void QueryFeatures(this MapView map, CartoSQLService service, LocalVectorDataSource source, PointStyle style, string query)
  {
    System.Threading.Tasks.Task.Run(delegate
    {
      FeatureCollection features = service.QueryFeatures(query, map.Options.BaseProjection);

      for (int i = 0; i < features.FeatureCount; i++)
      {
        Feature feature = features.GetFeature(i);

        // This data set features point geometry,
        // however, it can also be LineGeometry or PolygonGeometry
        PointGeometry geometry = (PointGeometry)feature.Geometry;

        var point = new Point(geometry, style);
        source.Add(point);
      }

    }); 
  }

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--objective-c">
  {% highlight objc %}

  // Only get cities with over 100k, or else it will be too many results
  NSString *sql = @"SELECT * FROM cities15000 WHERE population > 100000";
  
  // Initialize CartoSQL service, set a username
  NTCartoSQLService *service = [[NTCartoSQLService alloc] init];
  [service setUsername:@"nutiteq"];
  
  NTPointStyleBuilder *builder = [[NTPointStyleBuilder alloc]init];
  NTColor *color = [[NTColor alloc] initWithR:255 g:0 b:0 a:255];
  [builder setColor:color];
  [builder setSize:1];

  NTPointStyle *style = [builder buildStyle];
  
  dispatch_queue_t queue = dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0ul);
  
  // Set on background thread for "animated" point appear
  dispatch_async(queue, ^{
      NTFeatureCollection *features = [service queryFeatures:sql proj:self.projection];
  
      for (int i = 0; i < [features getFeatureCount]; i++) {

          // This data set features point geometry,
          // however, it can also be LineGeometry or PolygonGeometry
          NTPointGeometry *geometry = (NTPointGeometry *)[[features getFeature:i] getGeometry];
      
          NTPoint *point = [[NTPoint alloc] initWithGeometry:geometry style:style];
          [self.source add:point];
      }
  });

  {% endhighlight %}
  </div>
</div>

### CARTO.js API

In order to integrate your published mobile map into your app, you need a callback-based asynchronous process to request the viz.json with [CARTO.js](https://carto.com/docs/carto-engine/carto-js/.)

**Warning!** The current version of CARTO.js is still in development. Use at your own risk.

1)  The following requirements are needed for app integration of a published CARTO Mobile SDK map:

  - Online API is requested and will run in another thread

  - In certain cases, you will need to customize how layers are added in mobile, compared to how they appear on the desktop version of a map

  - Some viz.json elements, such as Overlays (Legend, Copyright, and so on) are not handled by Mobile SDK. Your mobile app gets data using a different callback method, which can be added as elements, according to your app design requirements

2) If you published a map through any of the CARTO Engines, you can load it to a mobile app. Use `CartoVisBuilder` interface to implement this callback

  **Note:** This type of request must run in another thread, as it relies on an online connection.

  `CartoVisBuilder` works as a parser for visualization. If you define callbacks for key elements of the visualization to implement key data in the viz.json (such as adding map layers), you may want to set the map center and zoom level

  This enables you to control which aspects of your mobile map are rendered. You can change some layer interpretation aspects for mobile, as it may be required for the mobile map to appear differently. 

3) Ensure your mobile app has all CARTO fonts installed in your mobile project folder

  **Tip** You can download [carto-fonts.zip](https://github.com/CartoDB/mobile-ios-samples/raw/master/AdvancedMap/Assets/carto-fonts.zip) from our iOs sample project. Note that these assets are not needed if your map does not contains any text.

4) The typical (minimal) request for `_CartoVisBuilder_` implementation requires the following, which you can append from your main ViewController.mm

<div class="js-TabPanes">
  <ul class="Tabs">
    <li class="Tab js-Tabpanes-navItem--lang is-active">
       <a href="#/0" class="js-Tabpanes-navLink--lang js-Tabpanes-navLink--lang--java">Java</a>
    </li>
      <li class="Tab js-Tabpanes-navItem--lang">
      <a href="#/2" class="js-Tabpanes-navLink--lang js-Tabpanes-navLink--lang--objective-c">Objective-C</a>
    </li>
  </ul>

  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--java is-active">
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

   <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--objective-c">
      {% highlight objc %}@interface MyCartoVisBuilder : NTCartoVisBuilder

    @property NTMapView* mapView;

    @end

    @implementation MyCartoVisBuilder

    // Methods to set map center and zoom based on defined map
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
     <li class="Tab js-Tabpanes-navItem--lang is-active">
       <a href="#/0" class="js-Tabpanes-navLink--lang js-Tabpanes-navLink--lang--java">Java</a>
    </li>
    <li class="Tab js-Tabpanes-navItem--lang">
      <a href="#/2" class="js-Tabpanes-navLink--lang js-Tabpanes-navLink--lang--objective-c">Objective-C</a>
    </li>
  </ul>

  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--java is-active">
  {% highlight java %}private String visJSONURL = "http://documentation.carto.com/api/v2/viz/2b13c956-e7c1-11e2-806b-5404a6a683d5/viz.json";

  Thread thread = new Thread(new Runnable() {
            @Override
            public void run() {
                mapView.getLayers().clear();

                // Create overlay layer for pop-ups
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

                // Add the created pop-up overlay layer on top of all visJSON layers
                mapView.getLayers().add(vectorLayer);
            }
        });
        thread.start(); 

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--objective-c">
  {% highlight objc %}dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
        // just make sure no other layers are on map
        [[mapView getLayers] clear];
        
        // Create VIS loader
        NTCartoVisLoader* loader = [[NTCartoVisLoader alloc] init];
        
        // Load fonts package, this has all fonts you may need.
        [loader setVectorTileAssetPackage:[[NTZippedAssetPackage alloc] initWithZipData:[NTAssetUtils loadAsset:@"carto-fonts.zip"]]];
        [loader setDefaultVectorLayerMode:YES];
        MyCartoVisBuilder* visBuilder = [[MyCartoVisBuilder alloc] init];
        visBuilder.mapView = mapView;

        // Use your Map URL in next line, you get it from the SHARE map publishing options, as shown in the following basic example:
        [loader loadVis:visBuilder visURL:@"http://documentation.carto.com/api/v2/viz/2b13c956-e7c1-11e2-806b-5404a6a683d5/viz.json"];
        
    });
      {% endhighlight %}
 
  </div>
</div>

6) Run your mobile app

  If you are using assets from the [sample apps](/docs/carto-engine/mobile-sdk/getting-started/#sample-apps) as part of your project, the following [map result](https://raw.githubusercontent.com/CartoDB/mobile-ios-samples/master/sample_viz_mobile.png) appears.


## SDK and CARTO API Samples

The CARTO [sample app](/docs/carto-engine/mobile-sdk/getting-started/#sample-apps) projects contain a number of working samples for all the mobile platforms:

- `CartoVisJsonActivity` load complete map configurations (from online viz.json)

- `CartoSQLActivity`  vector data via SQL API

- `CartoTorqueActivity` for Torque tiles

- `CartoUTFGridActivity` for raster tiles and UTFGrid

- `CartoVectorTileActivity` for vector tiles with CartoCSS styling 

- `CartoRasterTileActivity` for raster tiles with CartoCSS styling 
