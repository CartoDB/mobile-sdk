# Basic Map Components

This section describes the basic map components that are required when creating mobile apps with the SDK. The following key objects are required for each mobile app:

-  **MapView** is the object which goes to your app view hierarchy. It provides methods for directly manipulating the map and its view. For example, you can set center location, zoom, and so on.  MapViews contain objects for the map layers and map options

    - **Layer** (member of MapView) is an object that provides methods for adding data to the map

      - **DataSource** (member of Layers) is an object created as a member of the Layer object, which defines where the data comes from. There are specific DataSource implementations which keep data in memory, load from a persistent file or on-line API

    -  **Options** (member of MapView) is an object that provides methods for adjusting the map parameters

## Layers

Map data is organized by **Layers**, which are needed for rendering your visualization. The usual approach is to add one **TileLayer** as a general background then add one or more interactive **VectorLayers** on top of the first layer. You can then include _Markers_, _Texts_, _Popups_ and other map features on the top layer.

### DataSource

For mobile maps, each map layer is required to have a **DataSource**, which defines where the layer retrieves data. Several common data source implementations are built directly into the Mobile SDK, but you can also define your own data sources using the following parameters. 

- `HTTPRasterTileDataSource` is used for retrieving map tiles as images over HTTP connection

- `LocalVectorDataSource` stores data in memory and is used for adding vector objects to the map, per each user session

**Tip:** For details about selecting the DataSource through the map listener events, see [DataSources](/docs/carto-engine/mobile-sdk/map-listener-events/#map-listener-events).

### Basemap

Basemaps apply the map background required for rendering data. Basemaps are required as the bottom layer. You can add a vector layer that contains the background of your mobile map by using the following code: 

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
  {% highlight html %}Projection proj = mapView.getOptions().getBaseProjection();

// 1. Initialize an local vector data source
      LocalVectorDataSource vectorDataSource1 = new LocalVectorDataSource(proj);

// 2. Initialize a vector layer with the previous data source
      VectorLayer vectorLayer1 = new VectorLayer(vectorDataSource1);

// 3. Add the previous vector layer to the map
      mapView.getLayers().add(vectorLayer1);

// 4. Set limited visible zoom range for the vector layer
      vectorLayer1.setVisibleZoomRange(new MapRange(10, 24));

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item">
  {% highlight html %}Projection proj = MapView.Options.BaseProjection;

// 1. Initialize an local vector data source
      LocalVectorDataSource vectorDataSource1 = new LocalVectorDataSource(proj);

// 2. Initialize a vector layer with the previous data source
      VectorLayer vectorLayer1 = new VectorLayer(vectorDataSource1);

// 3. Add the previous vector layer to the map
      MapView.Layers.Add(vectorLayer1);

// 4. Set limited visible zoom range for the vector layer
      vectorLayer1.VisibleZoomRange = new MapRange(10, 24);

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item">
  {% highlight html %}

// this was already initialized before
  NTEPSG3857* proj = [[NTEPSG3857 alloc] init];

// 1. Initialize a local vector data source
      NTLocalVectorDataSource* vectorDataSource1 = [[NTLocalVectorDataSource alloc] initWithProjection:proj];
  
// 2. Initialize a vector layer with the previous data source
      NTVectorLayer* vectorLayer1 = [[NTVectorLayer alloc] initWithDataSource:vectorDataSource1];
  
// 3. Add the previous vector layer to the map
      [[self getLayers] add:vectorLayer1];
  
// 4. Set visible zoom range for the vector layer
      [vectorLayer1 setVisibleZoomRange:[[NTMapRange alloc] initWithMin:10 max:24]];

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item">
  {% highlight html %}

  COMING SOON...

  {% endhighlight %}
  </div>
    
</div>

## MapView Options

In following examples, **vector elements** (Markers, Points, Lines, Polygons, Texts and BalloonPopups) are added to a mobile map application. For each object, the styling is defined and objects are created based on given coordinates. These coordinates are store in the memory-based vector data source parameter, `LocalVectorDataSource`.

- Before adding any MapView objects, you must create a [VectorLayer](#layers) as the data source and add the layer to the map

**Note:** A popup (callout, bubble) which appears when you click on map is a map object of its own, and should be added using object click listener. For details, see [Add a BalloonPopup](/docs/carto-engine/mobile-sdk/basic-map-components/#add-a-balloonpopup).

### Add a Marker

Add a marker and apply marker styling using the following code:

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

// 1. Create marker style
      MarkerStyleBuilder markerStyleBuilder = new MarkerStyleBuilder();
      markerStyleBuilder.setSize(30);
      // Green colour as ARGB
      markerStyleBuilder.setColor(new Color(0xFF00FF00));

      MarkerStyle sharedMarkerStyle = markerStyleBuilder.buildStyle();

// 2. Add marker
      Marker marker1 = new Marker(proj.fromWgs84(new MapPos(24.646469, 59.426939)), sharedMarkerStyle);
      marker1.setMetaDataElement("ClickText", "Marker nr 1");
      vectorDataSource1.add(marker1);

// 3. Animate map to the marker
      mapView.setFocusPos(tallinn, 1);
      mapView.setZoom(12, 1);

{% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item">
  {% highlight html %}

// 1. Create marker style
      MarkerStyleBuilder markerStyleBuilder = new MarkerStyleBuilder();
      markerStyleBuilder.Size = 30;

// 2. Build style
      MarkerStyle sharedMarkerStyle = markerStyleBuilder.BuildStyle();

// 3. Set marker position
      MapPos tallinn = proj.FromWgs84(new MapPos(24.646469, 59.426939));

// 4. Add marker
      Marker marker1 = new Marker(tallinn, sharedMarkerStyle);
      marker1.SetMetaDataElement("ClickText", new Variant("Marker nr 1"));

// 5. Animate map to the marker
      MapView.SetFocusPos(tallinn, 1);
      MapView.SetZoom(12, 1);

      vectorDataSource1.Add(marker1);
    
  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item">
  {% highlight html %}

// 1. Create a marker style, we use default marker bitmap here
      NTMarkerStyleBuilder* markerStyleBuilder = [[NTMarkerStyleBuilder alloc] init];
      // Styles use dpi-independent units, no need to adjust it for retina
     [markerStyleBuilder setSize:30];
      // Green colour using ARGB format
      [markerStyleBuilder setColor:[[NTColor alloc] initWithColor:0xFF00FF00]]; 
      NTMarkerStyle* sharedMarkerStyle = [markerStyleBuilder buildStyle];

// 2. Define position and metadata for marker. Two important notes:
      // (1) Position in latitude/longitude has to be converted using projection
      // (2) X is longitude, Y is latitude !
      NTMapPos* pos = [proj fromWgs84:[[NTMapPos alloc] initWithX:24.651488 y:59.423581]];
      NTMarker* marker1 = [[NTMarker alloc] initWithPos:pos style:sharedMarkerStyle];
  
// 3. The defined metadata will be used later for Popups
      [marker1 setMetaDataElement:@"ClickText" element:[[NTVariant alloc] initWithString:@"Marker 1"]];
  
      [vectorDataSource1 add:marker1];

      [self.mapView setFocusPos:pos  durationSeconds:0]
      [self.mapView setZoom:12 durationSeconds:1];

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item">
  {% highlight html %}

  COMING SOON...

  {% endhighlight %}
  </div>
</div>

#### Example Marker on a Mobile Map

![pin](https://dl.dropboxusercontent.com/u/3573333/public_web/developersite/pin.png)

### Add a Point

Points are used to indicating specific location points on a map, similar to Markers. However, Points do not have overlapping controls and cannot be use with billboard style version 2.5D. If you have a lot of data (thousands of points) and are not using 2.5D views, use Points as an alternative to Markers. Your rendering time will be significantly faster.

You can add any type of vector objects to the same Layer and `UnculledVectorDataSource`. This enables you to reuse settings for a defined Marker. It is recommended to define different Layers and DataSources for managing your objects, as it allows you to:

-  Select and delete all objects of DataSource

-  Specify the drawing order of layers. The drawing order within a single Layer is undefined. For multiple layers, layers that are added are drawn underneath the initial map layer

Add a point and apply point styling using the following code:

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

// 1. Set marker position
      MapPos tallinn = proj.FromWgs84(new MapPos(24.646469, 59.426939));

// 2. Create style and position for the Point
      PointStyleBuilder pointStyleBuilder = new PointStyleBuilder();
      pointStyleBuilder.setColor(new Color(0xFF00FF00));
      pointStyleBuilder.setSize(16);

// 3. Create Point, add to datasource with metadata
      Point point1 = new Point(tallinn, pointStyleBuilder.buildStyle());
      point1.setMetaDataElement("ClickText", "Point nr 1");

      vectorDataSource1.add(point1);
  
// 4. Animate map to the point
      mapView.setFocusPos(tallinn, 1);
      mapView.setZoom(12, 1);
  
  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item">
  {% highlight html %}

// 1. Set point position
      MapPos tallinn = proj.FromWgs84(new MapPos(24.646469, 59.426939));

// 2. Create style and position for the Point
      var pointStyleBuilder = new PointStyleBuilder();
      pointStyleBuilder.Color = new Color(0, 255, 0, 255);
      pointStyleBuilder.Size = 16;

// 3. Create Point, add to datasource with metadata
      Point point1 = new Point(tallinn, pointStyleBuilder.BuildStyle());
      point1.SetMetaDataElement("ClickText", new Variant("Point nr 1"));

      vectorDataSource1.Add(point1);

// 4. Animate map to the point
      MapView.SetFocusPos(tallinn, 1);
      MapView.SetZoom(12, 1);

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item">
  {% highlight html %}

// 1. Create style and position for the Point
      NTPointStyleBuilder* pointStyleBuilder = [[NTPointStyleBuilder alloc] init];

      //color is defined as ARGB integer, i.e. following is opaque green
      //you can not use UIColor (or any other ObjectiveC specific class) in Carto mobile SDK
     
      [pointStyleBuilder setColor:[[NTColor alloc] initWithColor:0xFF00FF00]];
      [pointStyleBuilder setSize:16];
      NTMapPos* pos = [proj fromWgs84:[[NTMapPos alloc] initWithX:24.651488 y:59.423581]];

// 2. Create Point, add to datasource with metadata
      NTPoint* point1 = [[NTPoint alloc] initWithPos:pos style:[pointStyleBuilder buildStyle]];
      [point1 setMetaDataElement:@"ClickText" element:[[NTVariant alloc] initWithString:@"Point 1"]];
      [vectorDataSource1 add:point1];

      [self.mapView setFocusPos:pos  durationSeconds:0]
      [self.mapView setZoom:12 durationSeconds:1];

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item">
  {% highlight html %}

  COMING SOON...

  {% endhighlight %}
  </div>

</div>

### Add a Line

Lines can be added to the same `vectorDataSource1`. Add a line and apply line styling using the following code:

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

// 1. Create line style, and line poses
      LineStyleBuilder lineStyleBuilder = new LineStyleBuilder();
      lineStyleBuilder.setColor(new Color(0xFFFFFFFF));
      lineStyleBuilder.setLineJointType(LineJointType.LINE_JOINT_TYPE_ROUND);
      lineStyleBuilder.setWidth(8);

// 2. Special MapPosVector must be used for coordinates
      MapPosVector linePoses = new MapPosVector();
      MapPos initial = proj.fromWgs84(new MapPos(24.645565, 59.422074));
  
// 3. Add positions
      linePoses.add(initial);
      linePoses.add(proj.fromWgs84(new MapPos(24.643076, 59.420502)));
      linePoses.add(proj.fromWgs84(new MapPos(24.645351, 59.419149)));
      linePoses.add(proj.fromWgs84(new MapPos(24.648956, 59.420393)));
      linePoses.add(proj.fromWgs84(new MapPos(24.650887, 59.422707)));

// 4. Add a line
      Line line1 = new Line(linePoses, lineStyleBuilder.buildStyle());
      line1.setMetaDataElement("ClickText", "Line nr 1");
      vectorDataSource1.add(line1);
  
// 5. Animate map to the line
      mapView.setFocusPos(tallinn, 1);
      mapView.setZoom(12, 1);

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item">
  {% highlight html %}

// 1. Create line style, and line poses
      var lineStyleBuilder = new LineStyleBuilder();
      lineStyleBuilder.LineJoinType = LineJoinType.LineJoinTypeRound;
      lineStyleBuilder.Width = 8;
      lineStyleBuilder.Color = new Color(255, 0, 0, 255); // Red

      var positions = new MapPosVector();
      MapPos initial = proj.FromWgs84(new MapPos(24.645565, 59.422074));

// 2. Add positions
      positions.Add(initial);
      positions.Add(proj.FromWgs84(new MapPos(24.643076, 59.420502)));
      positions.Add(proj.FromWgs84(new MapPos(24.645351, 59.419149)));
      positions.Add(proj.FromWgs84(new MapPos(24.648956, 59.420393)));
      positions.Add(proj.FromWgs84(new MapPos(24.650887, 59.422707)));

// 3. Add line to source
      var line = new Line(positions, lineStyleBuilder.BuildStyle());
      vectorDataSource1.Add(line);

// 4. Animate zoom to the line
      MapView.SetFocusPos(initial, 1);
      MapView.SetZoom(15, 1);

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item">
  {% highlight html %}

// 1. Define line style
      NTLineStyleBuilder* lineStyleBuilder = [[NTLineStyleBuilder alloc] init];
        // White color, opaque
      [lineStyleBuilder setColor:[[NTColor alloc] initWithColor:0xFFFFFFFF]];
      [lineStyleBuilder setLineJointType:NT_LINE_JOINT_TYPE_ROUND];
      [lineStyleBuilder setWidth:8];

// 2. Define line positions, here as fixed locations
      MapPosVector* linePoses = [[MapPosVector alloc] init];
      NTMapPos* initial = [proj fromWgs84:[[NTMapPos alloc] initWithX:24.645565 y:59.422074]]

      [linePoses add:initial];
      [linePoses add:[proj fromWgs84:[[NTMapPos alloc] initWithX:24.643076 y:59.420502]]];
      [linePoses add:[proj fromWgs84:[[NTMapPos alloc] initWithX:24.645351 y:59.419149]]];
      [linePoses add:[proj fromWgs84:[[NTMapPos alloc] initWithX:24.648956 y:59.420393]]];
      [linePoses add:[proj fromWgs84:[[NTMapPos alloc] initWithX:24.650887 y:59.422707]]];

// 3. Create line, add metadata and add to the datasource
      NTLine* line1 = [[NTLine alloc] initWithGeometry:[[NTLineGeometry alloc] initWithPoses:linePoses] style:[lineStyleBuilder buildStyle]];
      [line1 setMetaDataElement:@"ClickText" element:[[NTVariant alloc] initWithString:@"Line 1"]];
      [vectorDataSource1 add:line1];

      [self.mapView setFocusPos:initial  durationSeconds:0]
      [self.mapView setZoom:15 durationSeconds:1];
  
  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item">
  {% highlight html %}

  COMING SOON...

  {% endhighlight %}
  </div>
  
</div>

#### Example Line on a Mobile Map

![line](https://dl.dropboxusercontent.com/u/3573333/public_web/developersite/line.png)

### Add a Polygon

Add a polygon and apply polygon styling using the following code. The following examples add a polygon with polygon holes:

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

// 1. Create polygon style and poses
      PolygonStyleBuilder polygonStyleBuilder = new PolygonStyleBuilder();
      polygonStyleBuilder.setColor(new Color(0xFFFF0000));
      lineStyleBuilder = new LineStyleBuilder();
      lineStyleBuilder.setColor(new Color(0xFF000000));
      lineStyleBuilder.setWidth(1.0f);
      polygonStyleBuilder.setLineStyle(lineStyleBuilder.buildStyle());

      MapPosVector polygonPoses = new MapPosVector();
      MapPos initial = proj.fromWgs84(new MapPos(24.650930, 59.421659));
      polygonPoses.add(initial);
      polygonPoses.add(proj.fromWgs84(new MapPos(24.657453, 59.416354)));
      polygonPoses.add(proj.fromWgs84(new MapPos(24.661187, 59.414607)));
      polygonPoses.add(proj.fromWgs84(new MapPos(24.667667, 59.418123)));
      polygonPoses.add(proj.fromWgs84(new MapPos(24.665736, 59.421703)));
      polygonPoses.add(proj.fromWgs84(new MapPos(24.661444, 59.421245)));
      polygonPoses.add(proj.fromWgs84(new MapPos(24.660199, 59.420677)));
      polygonPoses.add(proj.fromWgs84(new MapPos(24.656552, 59.420175)));
      polygonPoses.add(proj.fromWgs84(new MapPos(24.654010, 59.421472)));
        
// 2. Create 2 polygon holes
      MapPosVector holePoses1 = new MapPosVector();
      holePoses1.add(proj.fromWgs84(new MapPos(24.658409, 59.420522)));
      holePoses1.add(proj.fromWgs84(new MapPos(24.662207, 59.418896)));
      holePoses1.add(proj.fromWgs84(new MapPos(24.662207, 59.417411)));
      holePoses1.add(proj.fromWgs84(new MapPos(24.659524, 59.417171)));
      holePoses1.add(proj.fromWgs84(new MapPos(24.657615, 59.419834)));
      MapPosVector holePoses2 = new MapPosVector();
      holePoses2.add(proj.fromWgs84(new MapPos(24.665640, 59.421243)));
      holePoses2.add(proj.fromWgs84(new MapPos(24.668923, 59.419463)));
      holePoses2.add(proj.fromWgs84(new MapPos(24.662893, 59.419365)));
      MapPosVectorVector polygonHoles = new MapPosVectorVector();
          
      polygonHoles.add(holePoses1);
      polygonHoles.add(holePoses2);

// 3. Add polygon
      Polygon polygon = new Polygon(polygonPoses, polygonHoles, polygonStyleBuilder.buildStyle());
        polygon.setMetaDataElement("ClickText", new Variant("Polygon"));
        vectorDataSource1.add(polygon);

// 4. Animate zoom to position
      mapView.setFocusPos(initial, 1);
      mapView.setZoom(13, 1);

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item">
  {% highlight html %}

// 1. Create polygon style and poses
      PolygonStyleBuilder polygonStyleBuilder = new PolygonStyleBuilder();
      polygonStyleBuilder.Color = new Color(255, 0, 0, 255); // red
        var lineStyleBuilder = new LineStyleBuilder();
        lineStyleBuilder.Color = new Color(0, 0, 0, 255); // black
        lineStyleBuilder.Width = 1.0f;
        polygonStyleBuilder.LineStyle = lineStyleBuilder.BuildStyle();

// 2. Define coordinates of outer ring
      MapPosVector polygonPoses = new MapPosVector();
      MapPos initial = proj.FromWgs84(new MapPos(24.650930, 59.421659));
      polygonPoses.Add(initial);
      polygonPoses.Add(proj.FromWgs84(new MapPos(24.657453, 59.416354)));
      polygonPoses.Add(proj.FromWgs84(new MapPos(24.661187, 59.414607)));
      polygonPoses.Add(proj.FromWgs84(new MapPos(24.667667, 59.418123)));
      polygonPoses.Add(proj.FromWgs84(new MapPos(24.665736, 59.421703)));
      polygonPoses.Add(proj.FromWgs84(new MapPos(24.661444, 59.421245)));
      polygonPoses.Add(proj.FromWgs84(new MapPos(24.660199, 59.420677)));
      polygonPoses.Add(proj.FromWgs84(new MapPos(24.656552, 59.420175)));
      polygonPoses.Add(proj.FromWgs84(new MapPos(24.654010, 59.421472)));

// 3. Create polygon holes poses, note that special MapPosVectorVector must be used
    MapPosVectorVector polygonHoles = new MapPosVectorVector();

      MapPosVector hole1 = new MapPosVector();
      hole1.Add(proj.FromWgs84(new MapPos(24.658409, 59.420522)));
      hole1.Add(proj.FromWgs84(new MapPos(24.658409, 59.420522)));
      hole1.Add(proj.FromWgs84(new MapPos(24.662207, 59.418896)));
      hole1.Add(proj.FromWgs84(new MapPos(24.662207, 59.417411)));
      hole1.Add(proj.FromWgs84(new MapPos(24.659524, 59.417171)));
      hole1.Add(proj.FromWgs84(new MapPos(24.657615, 59.419834)));

      MapPosVector hole2 = new MapPosVector();
      hole2.Add(proj.FromWgs84(new MapPos(24.665640, 59.421243)));
      hole2.Add(proj.FromWgs84(new MapPos(24.668923, 59.419463)));
      hole2.Add(proj.FromWgs84(new MapPos(24.662893, 59.419365)));

      polygonHoles.Add(hole1);
      polygonHoles.Add(hole2);

// 4. Add polygon
      Polygon polygon = new Polygon(polygonPoses, polygonHoles, polygonStyleBuilder.BuildStyle());
        polygon.SetMetaDataElement("ClickText", new Variant("Polygon"));
        vectorDataSource1.Add(polygon);

// 5. Animate zoom to position
      MapView.SetFocusPos(initial, 1);
      MapView.SetZoom(13, 1);

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item">
  {% highlight html %}

// 1. Create polygon style
      NTPolygonStyleBuilder* polygonStyleBuilder = [[NTPolygonStyleBuilder alloc] init];
         // polygon fill color: opaque red
      [polygonStyleBuilder setColor:[[NTColor alloc] initWithColor:0xFFFF0000]];
         // define polygon outline style as line style
      lineStyleBuilder = [[NTLineStyleBuilder alloc] init];
         // opaque black color
      [lineStyleBuilder setColor:[[NTColor alloc] initWithColor:0xFF000000]];
      [lineStyleBuilder setWidth:1.0f];
      [polygonStyleBuilder setLineStyle:[lineStyleBuilder buildStyle]];

// 2. define polygon coordinates
     // First define outline as MapPosVector, which is array of MapPos
     // We cannot use Objective C objects, like NSArray in Carto mobile SDK, 
     // so there are special objects for collections
      MapPosVector* polygonPoses = [[MapPosVector alloc] init];
      NTMapPos* initial = [proj fromWgs84:[[NTMapPos alloc] initWithX:24.650930 y:59.421659]];

      [polygonPoses add:initial];
      [polygonPoses add:[proj fromWgs84:[[NTMapPos alloc] initWithX:24.657453 y:59.416354]]];
      [polygonPoses add:[proj fromWgs84:[[NTMapPos alloc] initWithX:24.661187 y:59.414607]]];
      [polygonPoses add:[proj fromWgs84:[[NTMapPos alloc] initWithX:24.667667 y:59.418123]]];
      [polygonPoses add:[proj fromWgs84:[[NTMapPos alloc] initWithX:24.665736 y:59.421703]]];
      [polygonPoses add:[proj fromWgs84:[[NTMapPos alloc] initWithX:24.661444 y:59.421245]]];
      [polygonPoses add:[proj fromWgs84:[[NTMapPos alloc] initWithX:24.660199 y:59.420677]]];
      [polygonPoses add:[proj fromWgs84:[[NTMapPos alloc] initWithX:24.656552 y:59.420175]]];
      [polygonPoses add:[proj fromWgs84:[[NTMapPos alloc] initWithX:24.654010 y:59.421472]]];

          // Define polygon holes. This is two-dimensional array (MapPosVectorVector)
          // because Polygon can have several holes. In this sample there are two
      MapPosVectorVector* holes = [[MapPosVectorVector alloc] init];
      [holes add:[[MapPosVector alloc] init]];
      [[holes get:0] add:[proj fromWgs84:[[NTMapPos alloc] initWithX:24.658409 y:59.420522]]];
      [[holes get:0] add:[proj fromWgs84:[[NTMapPos alloc] initWithX:24.662207 y:59.418896]]];
      [[holes get:0] add:[proj fromWgs84:[[NTMapPos alloc] initWithX:24.662207 y:59.417411]]];
      [[holes get:0] add:[proj fromWgs84:[[NTMapPos alloc] initWithX:24.659524 y:59.417171]]];
      [[holes get:0] add:[proj fromWgs84:[[NTMapPos alloc] initWithX:24.657615 y:59.419834]]];
      [holes add:[[MapPosVector alloc] init]];
      [[holes get:1] add:[proj fromWgs84:[[NTMapPos alloc] initWithX:24.665640 y:59.421243]]];
      [[holes get:1] add:[proj fromWgs84:[[NTMapPos alloc] initWithX:24.668923 y:59.419463]]];
      [[holes get:1] add:[proj fromWgs84:[[NTMapPos alloc] initWithX:24.662893 y:59.419365]]];

// 3. Create polygon, define metadata and add to datasource
      NTPolygon* polygon = [[NTPolygon alloc] initWithGeometry:[[NTPolygonGeometry alloc] initWithPoses:polygonPoses holes:holes] style:[polygonStyleBuilder buildStyle]];
      [polygon setMetaDataElement:@"ClickText" element:[[NTVariant alloc] initWithString:@"Polygon"]];
      [vectorDataSource1 add:polygon];

      [self.mapView setFocusPos:initial  durationSeconds:0]
      [self.mapView setZoom:13 durationSeconds:1];

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item">
  {% highlight html %}

  COMING SOON...

  {% endhighlight %}
  </div>
  
</div>

#### Example Polygon on a Mobile Map

![polygon](https://dl.dropboxusercontent.com/u/3573333/public_web/developersite/polygon.png)

### Add Text

Text style parameters are similar to Markers, as both are Billboards - which are MapView objects that contain the following features:

- Control and prohibit text overlapping, based on priority and the location of where the text appears

- Display text as billboards in 2.5D (tilted) view by defining the `OrientationMode` parameter. There are 3 options: show on ground, and rotate with map (like street names), show on ground (do not rotate with map), or show as billboard (no rotation).

Add text and apply text styling using the following code.

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

// 1.Create text style
  TextStyleBuilder textStyleBuilder = new TextStyleBuilder();
  textStyleBuilder.setColor(new Color(0xFFFF0000));
  textStyleBuilder.setOrientationMode(BillboardOrientation.BILLBOARD_ORIENTATION_FACE_CAMERA);

// This enables higher resolution texts for retina devices, but consumes more memory and is slower
  textStyleBuilder.setScaleWithDPI(false);

// 2. Add text
  MapPos position = proj.fromWgs84(new MapPos(24.653302, 59.422269));
  Text textpopup1 = new Text(position, textStyleBuilder.buildStyle(), "Face camera text");
  textpopup1.setMetaDataElement("ClickText", "Text nr 1");
  vectorDataSource1.add(textpopup1);
  
// 3. Animate zoom to position
  mapView.setFocusPos(position, 1);
  mapView.setZoom(13, 1);

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item">
  {% highlight html %}

// 1. Create text style
  TextStyleBuilder textStyleBuilder = new TextStyleBuilder();
  textStyleBuilder.setColor(new Color(0xFFFF0000));
  textStyleBuilder.setOrientationMode(BillboardOrientation.BILLBOARD_ORIENTATION_FACE_CAMERA);

// This enables higher resolution texts for retina devices, but consumes more memory and is slower
  textStyleBuilder.setScaleWithDPI(false);

// 2. Add text
  MapPos position = proj.fromWgs84(new MapPos(24.653302, 59.422269));
  Text textpopup1 = new Text(position, textStyleBuilder.buildStyle(), "Face camera text");
  textpopup1.setMetaDataElement("ClickText", new Variant("Text nr 1"));
  
  vectorDataSource1.add(textpopup1);

// 3. Animate zoom to position
  MapView.SetFocusPos(position, 1);
  MapView.SetZoom(13, 1);

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item">
  {% highlight html %}

// 4. Create text style
  NTTextStyleBuilder* textStyleBuilder = [[NTTextStyleBuilder alloc] init];
  [textStyleBuilder setColor:[[NTColor alloc] initWithColor:0xFFFF0000]];
  [textStyleBuilder setOrientationMode:NT_BILLBOARD_ORIENTATION_FACE_CAMERA];
  
  // setScaleWithDPI enables higher resolution texts for retina devices,
  // but consumes more memory and is slower if you have many texts on map
  [textStyleBuilder setScaleWithDPI:false];

// 5. Define text location and add to datasource
  NTMapPos* pos = [proj fromWgs84:[[NTMapPos alloc] initWithX:24.653302 y:59.422269]];
  NTText* text1 = [[NTText alloc] initWithPos:pos style:[textStyleBuilder buildStyle] text:@"Face camera text"];
  
  [text1 setMetaDataElement:@"ClickText" element:[[NTVariant alloc] initWithString:@"Text 1"]];

  [vectorDataSource1 add:text1];

  [self.mapView setFocusPos:pos  durationSeconds:0]
  [self.mapView setZoom:13 durationSeconds:1];

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item">
  {% highlight html %}

  COMING SOON...

  {% endhighlight %}
  </div>
  
</div>

#### Example Text on a Mobile Map

![text](https://dl.dropboxusercontent.com/u/3573333/public_web/developersite/text.png)

### Add a BalloonPopup

A BalloonPopup appears based on click event of an object. You can also add a defined BalloonPopup. Add BalloonPops using the following code. Note that these examples contain several special styling elements, such as:

- applied an image (_info.png_) as the "Left Image"
- applied an arrow (_arrow.png_)  as the "Right Image"
- styled the appearance of tuned radius values

**Tip:** To use these styling elements as part of your own project, copy the above .png files, (available from the [Sample Apps](/docs/carto-engine/mobile-sdk/sample-apps)), to your local map application project.

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

// 1. Load bitmaps to show on the label
      Bitmap infoImage = BitmapFactory.decodeResource(getResources(), R.drawable.info);
      Bitmap arrowImage = BitmapFactory.decodeResource(getResources(), R.drawable.arrow);

// 2. Add popup
      BalloonPopupStyleBuilder builder = new BalloonPopupStyleBuilder();
      builder.setCornerRadius(20);
      builder.setLeftMargins(new BalloonPopupMargins(6, 6, 6, 6));
      builder.setLeftImage(BitmapUtils.createBitmapFromAndroidBitmap(infoImage));
      builder.setRightImage(BitmapUtils.createBitmapFromAndroidBitmap(arrowImage));
      builder.setRightMargins(new BalloonPopupMargins(2, 6, 12, 6));
      builder.setPlacementPriority(1);

      MapPos position = proj.fromWgs84(new MapPos(24.655662, 59.425521));
      BalloonPopup popup = new BalloonPopup(position, builder.buildStyle(), "Popup with pos", "Images, round");
        popup.setMetaDataElement("ClickText", new Variant("Popup caption nr 1"));

        vectorDataSource1.add(popup);

        mapView.setFocusPos(position, 1);
        mapView.setZoom(13, 1);

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item">
  {% highlight html %}

// Android
      Bitmap androidInfoBitmap = BitmapFactory.DecodeResource(Resources, HelloMap.Resource.Drawable.info);
      Carto.Graphics.Bitmap infoBitmap = BitmapUtils.CreateBitmapFromAndroidBitmap(androidInfoBitmap);

      Bitmap androidArrowBitmap = BitmapFactory.DecodeResource(Resources, HelloMap.Resource.Drawable.arrow);
      Carto.Graphics.Bitmap arrowBitmap = BitmapUtils.CreateBitmapFromAndroidBitmap(androidArrowBitmap);

// iOS
      var infoBitmap = BitmapUtils.CreateBitmapFromUIImage(UIImage.FromFile("info.png"));
      var arrowBitmap = BitmapUtils.CreateBitmapFromUIImage(UIImage.FromFile("arrow.png"));

      var builder = new BalloonPopupStyleBuilder();
      builder.LeftImage = infoBitmap;
      builder.RightImage = arrowBitmap;
      builder.LeftMargins = new BalloonPopupMargins(6, 6, 6, 6);
      builder.RightMargins = new BalloonPopupMargins(2, 6, 12, 6);
      builder.CornerRadius = 20;
      builder.TitleFontName = "Helvetica";
      builder.TitleFontSize = 16;
      builder.TitleColor = new Color(200, 0, 0, 255);
      builder.StrokeColor = new Color(200, 120, 0, 255);
      builder.StrokeWidth = 1;
      builder.PlacementPriority = 1;

      MapPos position = proj.FromWgs84(new MapPos(0, 20));
      var popup = new BalloonPopup(position, builder.BuildStyle(), "Popup Title", "Description");

      vectorDataSource1.Add(popup);

      // Animate zoom to position
      MapView.SetFocusPos(position, 1);
      MapView.SetZoom(13, 1);

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item">
  {% highlight html %}

// 1. Load styling bitmaps to show on the popups
      UIImage* infoImage = [UIImage imageNamed:@"info.png"];
      UIImage* arrowImage = [UIImage imageNamed:@"arrow.png"];
  
// 2. Create style for the BalloonPopup
      NTBalloonPopupStyleBuilder* balloonPopupStyleBuilder = [[NTBalloonPopupStyleBuilder alloc] init];
      [balloonPopupStyleBuilder setCornerRadius:20];
      [balloonPopupStyleBuilder setLeftMargins:[[NTBalloonPopupMargins alloc] initWithLeft:6 top:6 right:6 bottom:6]];
      [balloonPopupStyleBuilder setLeftImage:infoImage];
      [balloonPopupStyleBuilder setRightImage:arrowImage];
      [balloonPopupStyleBuilder setRightMargins:[[NTBalloonPopupMargins alloc] initWithLeft:2 top:6 right:12 bottom:6]];
    
    // Higher priority ensures that baloon is visible when overlaps with other billboards
  [balloonPopupStyleBuilder setPlacementPriority:1];
 
// 3. Sefine location
      NTMapPos* pos = [proj fromWgs84:[[NTMapPos alloc] initWithX:24.655662 y:59.425521]];

// 4. Create BalloonPopup and add to datasource
      NTBalloonPopup* popup1 = [[NTBalloonPopup alloc] initWithPos:pos
                               style:[balloonPopupStyleBuilder buildStyle]
                               title:@"Popup with pos"
                              desc:@"Images, round"];
  
      [popup1 setMetaDataElement:@"ClickText" element:[[NTVariant alloc] initWithString:@"Popup caption 1"]];
      
      [vectorDataSource1 add:popup1];

      [self.mapView setFocusPos:pos  durationSeconds:0]
      [self.mapView setZoom:13 durationSeconds:1];

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item">
  {% highlight html %}

  COMING SOON...

  {% endhighlight %}
  </div>
  
</div>

#### Example BalloonPopup on a Mobile Map

![popup](https://dl.dropboxusercontent.com/u/3573333/public_web/developersite/popup.png)

### Add 3D Model Objects

One special feature of the Mobile SDK is that you can add 3D objects (models) to a mobile map. For example, you can add small moving car or other decorative or informative elements.

**Note:** 3D objects are added to the same `LocalVectorDataSource` as 2D objects. However, 3D objects are only supported using the *Nutiteq Markup Language* (**NML**) format, as it was created by our [mobile partner,  Nutiteq](https://carto.com/engine/mobile/). This custom format is optimized for the multi-resolution of 3D files on mobile apps. _For details about tuning the performance of 3D models, see [`LocalVectorDataSource` Performance](#localvectordatasource-performance)._

The following procedure describes how to setup and add a 3D object to your mobile MapView:

1. Select a _NML file_

    **Tip:** You can retrieve some free samples from [Nutiteq's NHL sample page](https://github.com/nutiteq/hellomap3d/wiki/NML-model-samples).

    If you have own model as Collada DAE (or KMZ) format, then you would need **Carto Mobile 3D converter tools** to convert it to NML, so it can be used in mobile. Please [contact us](mailto:sales@carto.com) for more information.

2. Adjust the file size of the 3D object for rendering

    **Note:** Typically, NML files are smaller than 100K. Anything larger than that takes too long to render. Consider applying lower detail models for your 3D objects. You can then add these models to the **res/raw** folder of your Android app, or as a **resource file** in your iOS project.

3. Load the model file in the code to add it to your map application by using the following code:

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

// 1. Load NML model from a file
      UnsignedCharVector modelFile = AssetUtils.loadBytes("fcd_auto.nml");

// 2. Set location for model, and create NMLModel object with this
      MapPos modelPos = baseProjection.fromWgs84(new MapPos(24.646469, 59.423939));
      NMLModel model = new NMLModel(modelPos, modelFile);

// 3. Adjust the size- oversize it by 20*, just to make it more visible (optional)
      model.setScale(20);

// 4. Add metadata for click handling (optional)
      model.setMetaDataElement("ClickText", new Variant("Single model"));

// 5. Add it to normal datasource
      vectorDataSource1.add(model);

      mapView.setFocusPos(position, 1);
      mapView.setZoom(15, 1);

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item">
  {% highlight html %}

      var file = AssetUtils.LoadAsset("fcd_auto.nml");

// 1. Set location for model, and create NMLModel object with this
      var position = proj.FromWgs84(new MapPos(24.646469, 59.423939));
      var model = new NMLModel(position, file);

// 2. Adjust the size- oversize it by 20x, just to make it more visible (optional)
      model.Scale = 20;

// 3. Add metadata for click handling (optional)
      model.SetMetaDataElement("ClickText", new Variant("Single model"));

// 4. Add it to normal datasource
      vectorDataSource1.Add(model);

// 5. Animate zoom to position
      MapView.SetFocusPos(position, 1);
      MapView.SetZoom(15, 1);

  {% endhighlight %}
  </div>

<div class="Carousel-item js-Tabpanes-item">
  {% highlight html %}
// 1. Load NML model from a file
  NTUnsignedCharVector* modelData = [NTAssetUtils loadBytes:@"fcd_auto.nml"];
    
// 2. Set location for model, and create NMLModel object with this
  NTMapPos* pos = [proj fromWgs84:[[NTMapPos alloc] initWithX:24.646469 y:59.424939]];
  NTNMLModel* model = [[NTNMLModel alloc] initWithPos:pos sourceModelData:modelData];
    
// 3. Add metadata for click handling (optional)
  [model setMetaDataElement:@"ClickText" element:[[NTVariant alloc] initWithString:@"My nice car"]];    
  
// 4. Adjust the size- oversize it by 20*, just to make it more visible (optional)
  [model setScale:20];

  [vectorDataSource1 add:model];

  [self.mapView setFocusPos:pos  durationSeconds:0]
  [self.mapView setZoom:15 durationSeconds:1];

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item">
  {% highlight html %}

  COMING SOON...

  {% endhighlight %}
  </div>
  
</div>

#### Example 3D Model Object on a Mobile Map

![3d animated](https://developer.nutiteq.com/images/animated3d.gif)

#### `LocalVectorDataSource` Performance

You can define the `spatialIndexType` of the `LocalVectorDataSource` to improve the performance of how a 3D model loads, by defining a spatial index to the DataSource. By default, no spatial index is defined. The following rules apply:

-  Do not define a spatial index if the number of elements is small, below ~1000 points or markers (or even less if you have complex lines and polygons). _This is the default option_

-  Apply `NT_LOCAL_SPATIAL_INDEX_TYPE_KDTREE` as the index type if there are a larger number of elements 

The advantage of defining a spatial index is that CPU usage decreases for large number of objects, improving the map performance of panning and zooming. However, displaying overlays may slightly delay the map response, as the spatial index is not loaded immediately when your move the map, it only moves after some hundred milliseconds. 

The overall maximum number of objects on map is limited to the RAM available for the app. Systems define several hundred MB for iOS apps, and closer to tens of MB for Android apps, but it depends on the device and app settings (as well as the density of the data). It is recommended to test your app with the targeted mobile platform and full dataset for the actual performance. 

The following code describes how to adjust the `LocalVectorDataSource` performance by defining a spatial index:

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
  {% highlight html %}LocalVectorDataSource vectorDataSource2 = new LocalVectorDataSource(proj, LocalSpatialIndexType.LOCAL_SPATIAL_INDEX_TYPE_KDTREE);

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item">
  {% highlight html %}

  var vectorDataSource2 = new LocalVectorDataSource(proj, LocalSpatialIndexType.LocalSpatialIndexTypeKdtree);

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item">
  {% highlight html %}NTLocalVectorDataSource* vectorDataSource2 =
    [[NTLocalVectorDataSource alloc] initWithProjection:proj
                                       spatialIndexType: NTLocalSpatialIndexType::NT_LOCAL_SPATIAL_INDEX_TYPE_KDTREE];

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item">
  {% highlight html %}COMING SOON...

  {% endhighlight %}
  </div>
</div>

<br/><br/>**Note:** If you have **very complex lines or polygons**, this creates objects with high numbers (more than hundreds of points per object) of vertexes. For example, the GPS traces for long periods of time, recording a point every second or so. 

Spatial indexing will not help in this case, as you need to show the whole track on the screen. In this case, apply the **automatic line simplification** parameter for the `LocalVectorDataSource`. This reduces the number of polygon and line points, while maintaining the original object shape. 

**Tip:** Automatic simplification is zoom-dependent. When a map is zoomed out, more aggressive simplification is used. When you zoom in, less simplification is applied until you see the original details.

There are several ways to simplify geometries for automatic simplification by using the following code. Note that simplification is applied in two passes:

- First pass applies fast Radial Distance vertex rejection
- Second pass applies Ramer-Douglas-Peuckerworst algorithm (with the worst case quadratic complexity)

All this is can be applied with just one line of code - as shown in the following example (under iOS) for minimal 1-pixel simplification, the result is invisible but effects are significant, depending on your DataSource:

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
  {% highlight html %}vectorDataSource2.setGeometrySimplifier(new DouglasPeuckerGeometrySimplifier(1.0f / 320.0f));

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item">
  {% highlight html %}vectorDataSource2.GeometrySimplifier = new DouglasPeuckerGeometrySimplifier(1.0f / 320.0f);

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item">
  {% highlight html %}[vectorDataSource2 setGeometrySimplifier:
  [[NTDouglasPeuckerGeometrySimplifier alloc] initWithTolerance: 1.0f / 320.0f]];

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item">
  {% highlight html %}COMING SOON...

  {% endhighlight %}
  </div>
  
</div>

The automatic simplification renders a GPU tasks faster with some additional computation (CPU). For aggressive simplification, you will notice a decrease in the of line quality, so use is sparingly.
