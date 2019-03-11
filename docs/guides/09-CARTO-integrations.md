## CARTO Engine integrations

If you have CARTO Enterprise account plans, you can connect to the CARTO Engine APIs via Mobile SDK, to retrieve map visualizations and table data from your CARTO account. 

There are several methods of connecting map data from your CARTO account (via the Mobile SDK) to your mobile app; depending on the size of your data, the visual requirements, and other factors.

- To use a map as **raster map tiles**, define the tile URL for `RasterTileLayer`
- To apply **interactivity** (object click data), use UTFGrid. This uses both raster map tiles and json-based UTF tiles.  UTFGrids are applicable to both raster and vector tiles, though are more useful for raster tiles. *For CARTO Builder Map, you will need to enable and define tooltips with the Pop-up feature*
- Load **vector tiles**, the CARTO Engine supports Mapbox Vector Tile (MVT) format tiles, which the Mobile SDK can render on the client side. You will also need [CartoCSS]({{site.styling_cartocss}}/) styles to view vector tiles. This is useful for applying advanced styling features, such as zooming and rotating maps based on data that can be packaged for offline line, using mbtiles
- **Load GeoJSON vector data**. This is useful if you need need more advanced interactivity (object click actions) or dynamic client-side styling of the objects. For vector data, the CARTO Engine provides a [SQL API]({{site.sqlapi_docs}}/) and mobile app that can load gsimplification and clustering
- If the **data table is large** (more than a ten thousand objects), then loading entire tables can overload the mobile client. Alternatively, use on-demand, view-based loading of vector data. Similar to the SQL API and GeoJSON format used on the CARTO Engine side, the SDK applies custom vector data sources to load data. _Only a selected, visible area, of the map will load._ The mobile app can control zoom levels, server-side generalizations, and simplifications can be applied
- For point-geometry time-series visualizations, use the _Animated_ aggregation to define Torque maps. This provides animated rendering, and the Mobile SDK has a special layer `TorqueTileLayer` to define this. 

### Offline maps

CARTO Mobile SDK can take map offline via the CARTO platform:

1. Upload your data to CARTO, create a new map with CARTO Builder, and apply map custom styling with CartoCSS.
2. Use the [Mobile Tile Packager](https://github.com/CartoDB/mobile-tile-packager) tool to create the offline map data package.
3. Load the package file to the mobile device - you can have your app download it from your server, or add it as bundled asset to your app.
4. Add the map to MapView, as a `VectorTileLayer`, from `MBTilesTileDataSource` and CartoCSS. 

This method enables you to get both optimized vector tiles and suitable CartoCSS styling for your map.

For details, see the **Offline Maps** page.

### Online maps 

CARTO Mobile SDK supports **CARTO Maps API** integration for Anonymous Maps and Named Maps. *Anonymous maps* allow you to instantiate a map given SQL and CartoCSS. *Named Maps* are essentially the same as Anonymous Maps except the MapConfig is stored on the server, and the map is given a unique name.

### Anonymous Map

Use `CartoMapsService` class to configure layers. Note that this must be done in a separate thread on Android, as Maps API requires connecting to server, which is not allowed in the main thread. The following snippet sets up the config (SQL and CartoCSS) we are going to use to define the map. As Anonymous map means that the map configuration - data and styles is created by client, then there are two phases. 

1) defining map parameters first:

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

    private String getConfigJson() {

        // Define server config
        JSONObject configJson = new JSONObject();

        try {
            // Change these according to your DB
            String sql = "SELECT * FROM stations_1";
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

  <div id="tab-csharp">
    {% highlight csharp linenos %}

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

            optionJson.Add("sql", "SELECT * FROM stations_1");
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

  <div id="tab-objectivec">
    {% highlight objc linenos %}

    - (NSString*) getConfig
    {
        NSDictionary* options = @{
                                  @"sql": @"SELECT FROM stations_1",
                                  @"cartocss": [self getCartoCSS],
                                  @"cartocss_version": @"2.1.1",
                                  @"interactivity": [self getInteractivityJson],
                                  @"attributes": [self getAttributesJson]
                                  };

        NSMutableArray* layerArray = [[NSMutableArray alloc] init];
        NSDictionary* layerJson = @{ @"options": options, @"type": @"cartodb" };
        
        [layerArray addObject:layerJson];
        
        NSDictionary* json = @{
                               @"version": @"1.0.1",
                               @"stat_tag": @"3c6f224a-c6ad-11e5-b17e-0e98b61680bf",
                               @"layers": layerArray
                               };
        
        NSError* error;
        NSData* data = [NSJSONSerialization dataWithJSONObject:json options:0 error:&error];
        
        return [[NSString alloc] initWithData:data encoding:NSUTF8StringEncoding];
    }

    -(NSString*) getCartoCSS
    {
        NSString* base = @"#stations_1 { marker-fill-opacity:0.9; marker-line-color:#FFF; marker-line-width:2; marker-line-opacity:1; marker-placement:point; marker-type:ellipse; marker-width:10; marker-allow-overlap:true; }";
        
        NSString* attributes = @"#stations_1[status = 'In Service'] { marker-fill:#0F3B82; } #stations_1[status = 'Not In Service'] { marker-fill:#aaaaaa; } #stations_1[field_9 = 200] { marker-width:80.0; } #stations_1[field_9 <= 49] { marker-width:25.0; } #stations_1[field_9 <= 38] { marker-width:22.8; } #stations_1[field_9 <= 34] { marker-width:20.6; } #stations_1[field_9 <= 29] { marker-width:18.3; } #stations_1[field_9 <= 25] { marker-width:16.1; } #stations_1[field_9 <= 20.5] { marker-width:13.9; } #stations_1[field_9 <= 16] { marker-width:11.7; } #stations_1[field_9 <= 12] { marker-width:9.4; } #stations_1[field_9 <= 8] { marker-width:7.2; } #stations_1[field_9 <= 4] { marker-width:5.0; } ";
        
        return [base stringByAppendingString:attributes];
    }

    -(NSMutableArray*) getInteractivityJson
    {
        NSMutableArray* array = [[NSMutableArray alloc] init];
        [array addObject:@"cartodb_id"];
        return array;
    }

    -(NSDictionary*) getAttributesJson
    {
        NSMutableArray* columns = [[NSMutableArray alloc] init];
        [columns addObject:@"name"];
        [columns addObject:@"field_9"];
        [columns addObject:@"slot"];
        
        return @{ @"id": @"cartodb_id", @"columns": columns };
    }

    {% endhighlight %}
  </div>

  <div id="tab-swift">
    {% highlight swift linenos %}

    public func getConfig() -> String? {
        
        let options = NSDictionary(dictionaryLiteral:
            ("sql","SELECT * FROM stations_1"),
            ("cartocss", getCartoCSS()),
            ("cartocss_version", "2.1.1"),
            ("interactivity", getInteractivityJson()),
            ("attributes", getAttributesJson())
        )
        
        let layerArray = NSMutableArray()
        let layerJson = NSDictionary(dictionaryLiteral: ("options", options), ("type", "cartodb"))
        
        layerArray.add(layerJson)
        
        let json = NSDictionary(dictionaryLiteral:
            ("version", "1.0.1"),
            ("stat_tag", "3c6f224a-c6ad-11e5-b17e-0e98b61680bf"),
            ("layers", layerArray)
        );
        
        if let theJSONData = try? JSONSerialization.data(withJSONObject: json, options: []) {
            return String(data: theJSONData, encoding: .ascii)
        }
        
        return nil
    }

    public func getCartoCSS() -> String {
        let base = "#stations_1 { marker-fill-opacity:0.9; marker-line-color:#FFF; marker-line-width:2; marker-line-opacity:1; marker-placement:point; marker-type:ellipse; marker-width:10; marker-allow-overlap:true; }"
        
        let attributes = "#stations_1[status = 'In Service'] { marker-fill:#0F3B82; } #stations_1[status = 'Not In Service'] { marker-fill:#aaaaaa; } #stations_1[field_9 = 200] { marker-width:80.0; } #stations_1[field_9 <= 49] { marker-width:25.0; } #stations_1[field_9 <= 38] { marker-width:22.8; } #stations_1[field_9 <= 34] { marker-width:20.6; } #stations_1[field_9 <= 29] { marker-width:18.3; } #stations_1[field_9 <= 25] { marker-width:16.1; } #stations_1[field_9 <= 20.5] { marker-width:13.9; } #stations_1[field_9 <= 16] { marker-width:11.7; } #stations_1[field_9 <= 12] { marker-width:9.4; } #stations_1[field_9 <= 8] { marker-width:7.2; } #stations_1[field_9 <= 4] { marker-width:5.0; } "
        
        return base + attributes;
    }

    public func getInteractivityJson() -> NSMutableArray {
        return NSMutableArray(array: ["cartodb_id"])
    }

    public func getAttributesJson() -> NSDictionary {
        let columns = NSMutableArray(array: ["name", "field_9", "slot"])
        return NSDictionary(dictionaryLiteral: ("id", "cartodb_id"), ("columns", columns))
    }

    {% endhighlight %}
  </div>

  <div id="tab-kotlin">
    {% highlight kotlin linenos %}

     fun getConfigJson(): String? {

        // Define server config
        val configJson = JSONObject()

        try {
            // Change these according to your DB
            val sql = "SELECT * FROM stations_1"
            val statTag = "3c6f224a-c6ad-11e5-b17e-0e98b61680bf"
            val columns = arrayOf("name", "status", "slot")

            val cartoCSS = "#stations_1{" +
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
            configJson.put("version", "1.0.1")
            configJson.put("stat_tag", statTag)

            val layersArrayJson = JSONArray()
            val layersJson = JSONObject()
            layersJson.put("type", "cartodb")

            val optionsJson = JSONObject()
            optionsJson.put("sql", sql)
            optionsJson.put("cartocss", cartoCSS)
            optionsJson.put("cartocss_version", "2.1.1")

            val interactivityJson = JSONArray()
            interactivityJson.put("cartodb_id")

            optionsJson.put("interactivity", interactivityJson)

            val attributesJson = JSONObject()
            attributesJson.put("id", "cartodb_id")

            val columnsJson = JSONArray()

            for (i in 0..columns.size - 1) {
                columnsJson.put(columns[i])
            }

            attributesJson.put("columns", columnsJson)
            optionsJson.put("attributes", attributesJson)
            layersJson.put("options", optionsJson)
            layersArrayJson.put(layersJson)
            configJson.put("layers", layersArrayJson)

        } catch (e: JSONException) {
            return null
        }

        return configJson.toString()
    }

    {% endhighlight %}
  </div>

</div>

2) Now that the config is set up, you initiate the map and create Layer to be added to the MapView. This snippet inits vector tiles, but you can change the default **vector layer mode to false** if you want to get a raster tiles.

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
                mapView.getLayers().addAll(layers);
            }
            catch (IOException e) {
                Log.e("EXCEPTION", "Exception: " + e);
            }
        }
    });

    serviceThread.start();

    {% endhighlight %}
  </div>

  <div id="tab-csharp">
    {% highlight csharp linenos %}

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
            service.DefaultVectorLayerMode = true; // Use VectorLayers

            LayerVector layers = service.BuildMap(Variant.FromString(config.ToString()));
            map.Layers.AddAll(layers);
        });
    }

    {% endhighlight %}
  </div>

  <div id="tab-objectivec">
    {% highlight objc linenos %}

    NTCartoMapsService* mapsService = [[NTCartoMapsService alloc] init];
    
    [mapsService setUsername:@"nutiteq"];
    
    // Use vector layers, not raster layers
    [mapsService setDefaultVectorLayerMode:TRUE];
    [mapsService setInteractive:TRUE];
    
    NTVariant* variant = [NTVariant fromString:[self getConfig]];
    NTLayerVector* layers = [mapsService buildMap:variant];
    [[self.mapView getLayers] addAll:layers];

    {% endhighlight %}
  </div>

  <div id="tab-swift">
    {% highlight swift linenos %}

    let config = getConfig()
    
    // This calculation should be in background thread
    DispatchQueue.global(qos: .userInitiated).async {
        
        let mapsService = NTCartoMapsService()
        mapsService?.setUsername("nutiteq")
        mapsService?.setDefaultVectorLayerMode(true) // use vector layers
        
        let layers = mapsService?.buildMap(NTVariant.fromString(config))
        self.mapView?.getLayers()?.addAll(layers)
    }

    {% endhighlight %}
  </div>

  <div id="tab-kotlin">
    {% highlight kotlin linenos %}

    val config = getConfigJson()

    // Use the Maps service to configure layers.
    // Note that this must be done in a separate thread on Android,
    // as Maps API requires connecting to server, which is not allowed in main thread.

    // Remember: Put your operations back on the main thread to change the UI
    // Note:
    // doAsync requires anko coroutines dependency
    // compile "org.jetbrains.anko:anko-sdk25-coroutines:$anko_version"
    doAsync {

        val mapsService = CartoMapsService()
        mapsService.username = "nutiteq"
        mapsService.isDefaultVectorLayerMode = true // use vector layers

        try {
            val layers = mapsService.buildMap(Variant.fromString(config))
            mapView?.layers?.addAll(layers)
        } catch (e: IOException) {
            println("Exception: " + e.message)
        }
    }

    {% endhighlight %}
  </div>

</div>


### Named Map

If you have created **Named map** using CARTO Maps API then map is already configured in the server, and map initiation is simpler:

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

    final CartoMapsService service = new CartoMapsService();

    // Use vector layers
    service.setDefaultVectorLayerMode(true);

    service.setUsername("nutiteq");

    final String name = "tpl_69f3eebe_33b6_11e6_8634_0e5db1731f59";

    // Be sure to make network queries on another thread
    Thread thread = new Thread(new Runnable() {
        @Override
        public void run() {
            try {
                LayerVector layers = service.buildNamedMap(name, new StringVariantMap());
                mapView.getLayers().addAll(layers);
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
    });

    thread.start();

    {% endhighlight %}
  </div>

  <div id="tab-csharp">
    {% highlight csharp linenos %}

    MapView.ConfigureNamedVectorLayers("tpl_69f3eebe_33b6_11e6_8634_0e5db1731f59");

    // Extension method
    public static void ConfigureNamedVectorLayers(this MapView map, string name)
    {
        System.Threading.Tasks.Task.Run(delegate
        {
            CartoMapsService service = new CartoMapsService();
            service.Username = "nutiteq";

            // Use VectorLayers
            service.DefaultVectorLayerMode = true;

            LayerVector layers = service.BuildNamedMap(name, new StringVariantMap());
            map.Layers.AddAll(layers);
        });
    }

    {% endhighlight %}
    </div>

    <div id="tab-objectivec">
    {% highlight objc linenos %}

    NTCartoMapsService* mapsService = [[NTCartoMapsService alloc] init];

    [mapsService setUsername:@"nutiteq"];

    // Use vector layers
    [mapsService setDefaultVectorLayerMode:YES];

    NTLayerVector* layers = [mapsService buildNamedMap:@"tpl_69f3eebe_33b6_11e6_8634_0e5db1731f59" templateParams: [[NTStringVariantMap alloc] init]];
    [[self.mapView getLayers] addAll:layers];

    {% endhighlight %}
  </div>

  <div id="tab-swift">
    {% highlight swift linenos %}

    let service = NTCartoMapsService()
    
    // Use vector layers
    service?.setDefaultVectorLayerMode(true)
    
    service?.setUsername("nutiteq")
    
    let name = "tpl_69f3eebe_33b6_11e6_8634_0e5db1731f59"

    DispatchQueue.global(qos: .userInitiated).async {
        let layers = service?.buildNamedMap(name, templateParams: NTStringVariantMap())
        self.mapView?.getLayers()?.addAll(layers)
    }

    {% endhighlight %}
  </div>

  <div id="tab-kotlin">
    {% highlight kotlin linenos %}

    val service = CartoMapsService()
        
    // Use vector layers
    service.isDefaultVectorLayerMode = true

    service.username = "nutiteq"

    val name = "tpl_69f3eebe_33b6_11e6_8634_0e5db1731f59"

    // Be sure to make network queries on another thread

    // Remember: Put your operations back on the main thread to change the UI
    // Note:
    // doAsync requires anko coroutines dependency
    // compile "org.jetbrains.anko:anko-sdk25-coroutines:$anko_version"
    doAsync {
        try {
            val layers = service.buildNamedMap(name, StringVariantMap())
            mapView?.layers?.addAll(layers)
        } catch (e: IOException) {
            e.printStackTrace()
        }
    }

    {% endhighlight %}
  </div>

</div>

### SQL API

CARTO’s **SQL API** allows you to interact with your tables and data inside CARTO, as if you were running SQL statements against a normal database. In general you can use the SQL API to insert, update or delete data (i.e., insert a new column with a latitude and longitude data) or to select data from public tables in order to use it on your website or application (i.e., display the 10 closest records to a particular location).

**Note:** In mobile SDK you can only **SELECT** data from public tables without api_key for higher security. Private tables and using api_key is not allowed from mobile directly. If you need these, you need to proxy CARTO SQL API to a custom API towards your app, so api_key requests are done from server to server

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

  <div id="tab-csharp">
    {% highlight csharp linenos %}

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

  <div id="tab-objectivec">
    {% highlight objc linenos %}

    // Only get cities with over 100k, or else it will be too many results
    NSString* sql = @"SELECT * FROM cities15000 WHERE population > 100000";
    
    // Initialize CartoSQL service, set a username
    NTCartoSQLService* service = [[NTCartoSQLService alloc] init];
    [service setUsername:@"nutiteq"];
    
    NTPointStyleBuilder* builder = [[NTPointStyleBuilder alloc] init];
    NTColor* color = [[NTColor alloc] initWithR:255 g:0 b:0 a:255];
    [builder setColor:color];
    [builder setSize:1];
    NTPointStyle* style = [builder buildStyle];
    
    dispatch_queue_t queue = dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0ul);
    
    // Set on background thread for "animated" point appear
    dispatch_async(queue, ^{
        NTFeatureCollection *features = [service queryFeatures:sql proj:self.projection];
    
        for (int i = 0; i < [features getFeatureCount]; i++) {

            // This data set features point geometry,
            // however, it can also be LineGeometry or PolygonGeometry
            NTPointGeometry* geometry = (NTPointGeometry *)[[features getFeature:i] getGeometry];
        
            NTPoint* point = [[NTPoint alloc] initWithGeometry:geometry style:style];
            [self.source add:point];
        }
    });

    {% endhighlight %}
  </div>
    
  <div id="tab-swift">
  {% highlight swift linenos %}

    let query = "SELECT * FROM cities15000 WHERE population > 100000"
    
    let service = NTCartoSQLService()
    service?.setUsername("nutiteq")
    
    let builder = NTPointStyleBuilder()
    builder?.setSize(1.0)
    builder?.setColor(NTColor(r: 255, g: 0, b: 0, a: 255))
    let style = builder?.buildStyle()
    
    let source = NTLocalVectorDataSource(projection: projection);
    
    // Networking should be placed on a background thread
    // Remember: Put your operations back on the main thread to change the UI
    DispatchQueue.global(qos: .userInitiated).async {
        
        let features = service?.queryFeatures(query, proj: projection)
        
        let count = Int((features?.getFeatureCount())!)
        for i in 0..<count {
            
            // This data set features point geometry,
            // however, it can also be LineGeometry or PolygonGeometry
            
            let geometry = features?.getFeature(Int32(i)).getGeometry() as! NTPointGeometry
            source?.add(NTPoint (geometry: geometry, style: style))
        }
    }

    {% endhighlight %}
  </div>

  <div id="tab-kotlin">
    {% highlight kotlin linenos %}

    val query = "SELECT * FROM cities15000 WHERE population > 100000"

    val service = CartoSQLService()
    service.username = "nutiteq"

    val builder = PointStyleBuilder()
    builder.size = 1F
    builder.color = Color(255, 0, 0, 255)
    val style = builder.buildStyle()

    // Remember: Put your operations back on the main thread to change the UI
    // Note:
    // doAsync requires anko coroutines dependency
    // compile "org.jetbrains.anko:anko-sdk25-coroutines:$anko_version"
    doAsync {

        try {
            val features = service.queryFeatures(query, projection)

            for (i in 0..features.featureCount - 1) {

                // This data set features point geometry,
                // however, it can also be LineGeometry or PolygonGeometry

                val geometry = features.getFeature(i).geometry as PointGeometry
                source?.add(Point (geometry, style))
            }

        } catch (e: IOException) {

        }
    }

    {% endhighlight %}
  </div>

</div>


