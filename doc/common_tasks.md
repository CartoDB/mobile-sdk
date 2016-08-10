# Adding objects to the map

## Basic principles

Following are key objects in Carto Maps SDK:

*  **MapView** is the object which goes to your app View hierarchy and it provides methods for directly manipulating the map and it's view - e.g. set center location, zoom etc. 
*  **Layers** object (member of MapView) provides methods for adding data to the map. 
* Layers are created with **DataSource** (member of Layer) which defines from where the data is taken, there are specific DataSource implementations which keep data in memory, load from a persistent file or on-line API.
*  **Options** object (member of MapView) provides methods for tweaking the map parameters.

### Adding objects to the map

Map data is organized as **Layers**, the concept which is familiar to GIS users. The usual approach is to add one **TileLayer** as a general background; e.g. a Tiled Vector Map with OpenStreetMap data, as given in Getting Started Guide. Then you add one or several interactive **VectorLayers** on top of it (with _Markers_, _Texts_, _Popups_ etc).

Each map layer is required to have a **DataSource**, which defines from where and how the layer retrieves data. Several common data source implementations are built into the maps SDK, but you can define also own specific data sources. For example **HTTPRasterTileDataSource** is used for retrieving map tiles as images over HTTP connection and **LocalVectorDataSource** keeps data in memory and is used for adding per user session vector objects to the map.

In following examples we show **vector elements**: Markers, Points, Lines, Polygons, Texts and BalloonPopups on the map. For each we define styling, create objects to given coordinates and add them to a memory-based vector data source *LocalVectorDataSource*. First of all we create a *VectorLayer* with the data source, and add the Layer to the map.

Note that Popup (callout, bubble) which is opened when you click on map is a map object of its own, and should be added using object click listener.

## 0. Base: add a (vector) layer that holds additional data

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

  Projection proj = mapView.getOptions().getBaseProjection();

  // Initialize an local vector data source
  LocalVectorDataSource vectorDataSource1 = new LocalVectorDataSource(proj);

  // Initialize a vector layer with the previous data source
  VectorLayer vectorLayer1 = new VectorLayer(vectorDataSource1);

  // Add the previous vector layer to the map
  mapView.getLayers().add(vectorLayer1);

  // Set limited visible zoom range for the vector layer
  vectorLayer1.setVisibleZoomRange(new MapRange(10, 24));

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item">
  {% highlight html %}

        Projection proj = MapView.Options.BaseProjection;

      // Initialize an local vector data source
      LocalVectorDataSource vectorDataSource1 = new LocalVectorDataSource(proj);

      // Initialize a vector layer with the previous data source
      VectorLayer vectorLayer1 = new VectorLayer(vectorDataSource1);

      // Add the previous vector layer to the map
      MapView.Layers.Add(vectorLayer1);

      // Set limited visible zoom range for the vector layer
      vectorLayer1.VisibleZoomRange = new MapRange(10, 24);

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item">
  {% highlight html %}

  // this was initialized before
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

## 1. Add a Marker

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

  // Create marker style
  MarkerStyleBuilder markerStyleBuilder = new MarkerStyleBuilder();
  markerStyleBuilder.setSize(30);
  // Green colour as ARGB
  markerStyleBuilder.setColor(new Color(0xFF00FF00));

  MarkerStyle sharedMarkerStyle = markerStyleBuilder.buildStyle();

  // Add marker
  Marker marker1 = new Marker(proj.fromWgs84(new MapPos(24.646469, 59.426939)), sharedMarkerStyle);
  marker1.setMetaDataElement("ClickText", "Marker nr 1");
  vectorDataSource1.add(marker1);

  // Animate map to the marker
  mapView.setFocusPos(tallinn, 1);
  mapView.setZoom(12, 1);

{% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item">
  {% highlight html %}  

      // Create marker style
      MarkerStyleBuilder markerStyleBuilder = new MarkerStyleBuilder();
      markerStyleBuilder.Size = 30;

      // Build style
      MarkerStyle sharedMarkerStyle = markerStyleBuilder.BuildStyle();

      // Set marker position
      MapPos tallinn = proj.FromWgs84(new MapPos(24.646469, 59.426939));

      // Add marker
      Marker marker1 = new Marker(tallinn, sharedMarkerStyle);
      marker1.SetMetaDataElement("ClickText", new Variant("Marker nr 1"));

      // Animate map to the marker
      MapView.SetFocusPos(tallinn, 1);
      MapView.SetZoom(12, 1);

      vectorDataSource1.Add(marker1);
    
  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item">
  {% highlight html %}

  // 5. Create a marker style, we use default marker bitmap here
  NTMarkerStyleBuilder* markerStyleBuilder = [[NTMarkerStyleBuilder alloc] init];
    // Styles use dpi-independent units, no need to adjust it for retina
  [markerStyleBuilder setSize:30];
   // Green colour using ARGB format
  [markerStyleBuilder setColor:[[NTColor alloc] initWithColor:0xFF00FF00]]; 
  NTMarkerStyle* sharedMarkerStyle = [markerStyleBuilder buildStyle];

  // 7. Define position and metadata for marker. Two important notes:
    // (1) Position in latitude/longitude has to be converted using projection
    // (2) X is longitude, Y is latitude !
  NTMapPos* pos = [proj fromWgs84:[[NTMapPos alloc] initWithX:24.651488 y:59.423581]];
  NTMarker* marker1 = [[NTMarker alloc] initWithPos:pos style:sharedMarkerStyle];
  
  // Defined metadata will be used later for Popups
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

![pin](https://dl.dropboxusercontent.com/u/3573333/public_web/developersite/pin.png)

## 2. Add a Point

Points are marking specific location points, just as Markers. Key difference is that Point does not have overlapping control and cannot be with billboard style in 2.5D. So if you have a lot of data (thousands of points) and do not use 2.5D views, then using Points is suggested, as it can be significantly faster.

You can add any type of vector objects to same Layer and UnculledVectorDataSource, so we reuse same as defined for Marker. You should define different Layers and DataSources for specific reasons:

 * to delete all objects of data source at once
 * specify drawing order. Drawing order within one Layer is undefined, but Layers which are added before are always drawn below next Layers.

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

## 3. Add a Line

Line can be added to the same *vectorDataSource1*:

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

  // Create line style, and line poses
  LineStyleBuilder lineStyleBuilder = new LineStyleBuilder();
  lineStyleBuilder.setColor(new Color(0xFFFFFFFF));
  lineStyleBuilder.setLineJointType(LineJointType.LINE_JOINT_TYPE_ROUND);
  lineStyleBuilder.setWidth(8);

  // Special MapPosVector must be used for coordinates
  MapPosVector linePoses = new MapPosVector();
  MapPos initial = proj.fromWgs84(new MapPos(24.645565, 59.422074));
  
  // Add positions
  linePoses.add(initial);
  linePoses.add(proj.fromWgs84(new MapPos(24.643076, 59.420502)));
  linePoses.add(proj.fromWgs84(new MapPos(24.645351, 59.419149)));
  linePoses.add(proj.fromWgs84(new MapPos(24.648956, 59.420393)));
  linePoses.add(proj.fromWgs84(new MapPos(24.650887, 59.422707)));

  // Add a line
  Line line1 = new Line(linePoses, lineStyleBuilder.buildStyle());
  line1.setMetaDataElement("ClickText", "Line nr 1");
  vectorDataSource1.add(line1);
  
  // 4. Animate map to the line
  mapView.setFocusPos(tallinn, 1);
  mapView.setZoom(12, 1);

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item">
  {% highlight html %}

      // Create line style, and line poses
      var lineStyleBuilder = new LineStyleBuilder();
      lineStyleBuilder.LineJoinType = LineJoinType.LineJoinTypeRound;
      lineStyleBuilder.Width = 8;
      lineStyleBuilder.Color = new Color(255, 0, 0, 255); // Red

      var positions = new MapPosVector();
      MapPos initial = proj.FromWgs84(new MapPos(24.645565, 59.422074));

      // Add positions
      positions.Add(initial);
      positions.Add(proj.FromWgs84(new MapPos(24.643076, 59.420502)));
      positions.Add(proj.FromWgs84(new MapPos(24.645351, 59.419149)));
      positions.Add(proj.FromWgs84(new MapPos(24.648956, 59.420393)));
      positions.Add(proj.FromWgs84(new MapPos(24.650887, 59.422707)));

      // Add line to source
      var line = new Line(positions, lineStyleBuilder.BuildStyle());
      vectorDataSource1.Add(line);

      // Animate zoom to the line
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

![line](https://dl.dropboxusercontent.com/u/3573333/public_web/developersite/line.png)

## 4. Add a Polygon

Following defines and adds a polygon with a hole

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

         // Create polygon style and poses
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
        
        // Create 2 polygon holes
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

        // Add polygon
        Polygon polygon = new Polygon(polygonPoses, polygonHoles, polygonStyleBuilder.buildStyle());
        polygon.setMetaDataElement("ClickText", new Variant("Polygon"));
        vectorDataSource1.add(polygon);

      // Animate zoom to position
      mapView.setFocusPos(initial, 1);
      mapView.setZoom(13, 1);

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item">
  {% highlight html %}

        // Create polygon style and poses
      PolygonStyleBuilder polygonStyleBuilder = new PolygonStyleBuilder();
      polygonStyleBuilder.Color = new Color(255, 0, 0, 255); // red
      var lineStyleBuilder = new LineStyleBuilder();
      lineStyleBuilder.Color = new Color(0, 0, 0, 255); // black
      lineStyleBuilder.Width = 1.0f;
      polygonStyleBuilder.LineStyle = lineStyleBuilder.BuildStyle();

      // Define coordinates of outer ring
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

      // Create polygon holes poses, note that special MapPosVectorVector must be used
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

      // Add polygon
      Polygon polygon = new Polygon(polygonPoses, polygonHoles, polygonStyleBuilder.BuildStyle());
      polygon.SetMetaDataElement("ClickText", new Variant("Polygon"));
      vectorDataSource1.Add(polygon);

      // Animate zoom to position
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

![polygon](https://dl.dropboxusercontent.com/u/3573333/public_web/developersite/polygon.png)


## 5. Add a Text

Text style parameters are similar to Marker, as both are Billboards, i.e. objects which have two special features:

* Overlapping can be prohibited, based which one is nearer and explicit priority parameter
* These can be shown as billboards in 2.5D (tilted) view. Set OrientationMode for this. There are 3 options: show on ground, and rotate with map (like street names), show on ground (do not rotate with map), or show as billboard (no rotation). See image below for the options.

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

  // Create text style
  TextStyleBuilder textStyleBuilder = new TextStyleBuilder();
  textStyleBuilder.setColor(new Color(0xFFFF0000));
  textStyleBuilder.setOrientationMode(BillboardOrientation.BILLBOARD_ORIENTATION_FACE_CAMERA);

  // This enables higher resolution texts for retina devices, but consumes more memory and is slower
  textStyleBuilder.setScaleWithDPI(false);

  // Add text
  MapPos position = proj.fromWgs84(new MapPos(24.653302, 59.422269));
  Text textpopup1 = new Text(position, textStyleBuilder.buildStyle(), "Face camera text");
  textpopup1.setMetaDataElement("ClickText", "Text nr 1");
  vectorDataSource1.add(textpopup1);
  
  // Animate zoom to position
  mapView.setFocusPos(position, 1);
  mapView.setZoom(13, 1);

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item">
  {% highlight html %}

  // Create text style
  TextStyleBuilder textStyleBuilder = new TextStyleBuilder();
  textStyleBuilder.setColor(new Color(0xFFFF0000));
  textStyleBuilder.setOrientationMode(BillboardOrientation.BILLBOARD_ORIENTATION_FACE_CAMERA);

  // This enables higher resolution texts for retina devices, but consumes more memory and is slower
  textStyleBuilder.setScaleWithDPI(false);

  // Add text
  MapPos position = proj.fromWgs84(new MapPos(24.653302, 59.422269));
  Text textpopup1 = new Text(position, textStyleBuilder.buildStyle(), "Face camera text");
  textpopup1.setMetaDataElement("ClickText", new Variant("Text nr 1"));
  
  vectorDataSource1.add(textpopup1);

  // Animate zoom to position
  MapView.SetFocusPos(position, 1);
  MapView.SetZoom(13, 1);

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item">
  {% highlight html %}

  // Create text style
  NTTextStyleBuilder* textStyleBuilder = [[NTTextStyleBuilder alloc] init];
  [textStyleBuilder setColor:[[NTColor alloc] initWithColor:0xFFFF0000]];
  [textStyleBuilder setOrientationMode:NT_BILLBOARD_ORIENTATION_FACE_CAMERA];
  
  // setScaleWithDPI enables higher resolution texts for retina devices,
  // but consumes more memory and is slower if you have many texts on map
  [textStyleBuilder setScaleWithDPI:false];

  // Define text location and add to datasource
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

![text](https://dl.dropboxusercontent.com/u/3573333/public_web/developersite/text.png)

## 6. Add a BalloonPopup

BalloonPopup is usually opened based on click event of an object, but you can just directly add them also. Following BalloonPopup has several special styling elements, like (i) image as "Left Image", arrow image as "Right Image" and tuned radius values to look nicer.

To use the same styling elements you have to copy the png files (info.png and arrow.png) from sample app projects to your app.

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

          // Load bitmaps to show on the label
        Bitmap infoImage = BitmapFactory.decodeResource(getResources(), R.drawable.info);
        Bitmap arrowImage = BitmapFactory.decodeResource(getResources(), R.drawable.arrow);

        // Add popup
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
 
    // 3. define location
  NTMapPos* pos = [proj fromWgs84:[[NTMapPos alloc] initWithX:24.655662 y:59.425521]];

  // 4. create BalloonPopup and add to datasource
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

![popup](https://dl.dropboxusercontent.com/u/3573333/public_web/developersite/popup.png)

## 6. Add 3D model objects

One special feature of Carto Mobile SDK is that you can add 3D objects (models) to map. For example you can add small moving car or other decorative or informative elements.

3D objects are added to same *LocalVectorDataSource* as 2D objects. The only special requirement is that it has to be in special *Nutiteq Markup Language* (**NML**) format. As name tells, it comes from the Nutiteq SDK, predecessor of Carto Mobile SDK and it is mobile-optimised (multi-resolution) 3D file format.

1. First you need a NML file. You can get some free samples from [here](https://github.com/nutiteq/hellomap3d/wiki/NML-model-samples). If you have own model as Collada DAE (or KMZ) format, then you would need **Carto Mobile 3D converter tools** to convert it to NML, so it can be used in mobile. Please ask sales@cartodb.com for more info about it.

2. Typical NML files are quite small (below 100K), if they are large then they are probably too slow to render anyway, so consider using lower detail models. Then you can add it to **res/raw** folder on Android app, or as **resource file** in iOS project.

3. Finally load the model in code, and add to map

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

  // load NML file model from a file
  UnsignedCharVector modelFile = AssetUtils.loadBytes("fcd_auto.nml");

  // set location for model, and create NMLModel object with this
  MapPos modelPos = baseProjection.fromWgs84(new MapPos(24.646469, 59.423939));
  NMLModel model = new NMLModel(modelPos, modelFile);

  // oversize it 20*, just to make it more visible (optional)
  model.setScale(20);

   // add metadata for click handling (optional)
  model.setMetaDataElement("ClickText", new Variant("Single model"));

  // add it to normal datasource
  vectorDataSource1.add(model);

  mapView.setFocusPos(position, 1);
  mapView.setZoom(15, 1);

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item">
  {% highlight html %}

      var file = AssetUtils.LoadAsset("fcd_auto.nml");

      // Set location for model, and create NMLModel object with this
      var position = proj.FromWgs84(new MapPos(24.646469, 59.423939));
      var model = new NMLModel(position, file);

      // Oversize it 20x, just to make it more visible (optional)
      model.Scale = 20;

      // Add metadata for click handling (optional)
      model.SetMetaDataElement("ClickText", new Variant("Single model"));

      // Add it to normal datasource
      vectorDataSource1.Add(model);

      // Animate zoom to position
      MapView.SetFocusPos(position, 1);
      MapView.SetZoom(15, 1);

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item">
  {% highlight html %}
  
  // Load NML file model from a file
  NTUnsignedCharVector* modelData = [NTAssetUtils loadBytes:@"fcd_auto.nml"];
    
  // Set location for model, and create NMLModel object with this
  NTMapPos* pos = [proj fromWgs84:[[NTMapPos alloc] initWithX:24.646469 y:59.424939]];
  NTNMLModel* model = [[NTNMLModel alloc] initWithPos:pos sourceModelData:modelData];
    
   // Add metadata for click handling (optional)
  [model setMetaDataElement:@"ClickText" element:[[NTVariant alloc] initWithString:@"My nice car"]];    
  
  // Oversize it 20*, just to make it more visible (optional)
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

![3d animated](https://developer.nutiteq.com/images/animated3d.gif)


### Performance tuning of LocalVectorDataSource

**LocalVectorDataSource** has additional creation option **spatialIndexType** which enables to define spatial index to the datasource. By default there is no spatial index. Our suggestion:

* Use no spatial index (default option) if number of elements is small, below ~1000 points or markers (less if you have complex lines and polygons)
* Use *NT_LOCAL_SPATIAL_INDEX_TYPE_KDTREE* as index type for **bigger number of elements.** 

Advantage of the index is that CPU usage decreases a lot for large number of objects and you will have smoother map interaction (pan and zoom). But the downside is that showing overlay is slightly delayed: using index is not done immediately when you move the map, but after some hundred milliseconds. 

Overall maximum number of objects on map is in any case limited to the RAM available for the app. System gives several hundred MB for iOS app, and more like tens of MB for Android app, but it depends a lot on device and app settings, also on data density. So we suggest to test with your target devices and full dataset. 

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

  LocalVectorDataSource vectorDataSource2 = new LocalVectorDataSource(proj, LocalSpatialIndexType.LOCAL_SPATIAL_INDEX_TYPE_KDTREE);

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item">
  {% highlight html %}

  var vectorDataSource2 = new LocalVectorDataSource(proj, LocalSpatialIndexType.LocalSpatialIndexTypeKdtree);

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item">
  {% highlight html %}

      NTLocalVectorDataSource* vectorDataSource2 =
    [[NTLocalVectorDataSource alloc] initWithProjection:proj
                                       spatialIndexType: NTLocalSpatialIndexType::NT_LOCAL_SPATIAL_INDEX_TYPE_KDTREE];

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item">
  {% highlight html %}

  COMING SOON...

  {% endhighlight %}
  </div>
  
</div>

Another special case is if you have **very complex lines or polygons**: objects with high number of vertexes. By high number I mean more than hundreds of points per object. Typical example would be GPS traces for long periods of time, recording a point every second or so. Indexing does not help here much, as you would want to show whole track on screen. Here LocalVectorDataSource provides **automatic line simplification** feature, which reduces number of polygon and line points while trying to keep the shape of the object similar to original. By *automatic* simplification we mean zoom-dependent re-simplification: when map is zoomed out, more aggressive simplification is used as you will not notice it, and when you zoom in, then less simplification is applied, until in fill zoom in you see original details.

There are several ways how to simplify geometries, currently Nutiteq SDK has one method implemented for lines and polygons: the most common Ramer-Douglas-Peucker algorithm. To be exact, simplification is done in two passes - first pass uses fast Radial Distance vertex rejection, second pass uses Ramer-Douglas-Peuckerworst algorithm (with worst case quadratic complexity). All this is can be used with just one line of code - in following example (under iOS) for minimal 1-pixel simplification, where result should be invisible but effect can be significant, depending on your source data:

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

  vectorDataSource2.setGeometrySimplifier(new DouglasPeuckerGeometrySimplifier(1.0f / 320.0f));

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item">
  {% highlight html %}

  vectorDataSource2.GeometrySimplifier = new DouglasPeuckerGeometrySimplifier(1.0f / 320.0f);

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item">
  {% highlight html %}

  [vectorDataSource2 setGeometrySimplifier:
  [[NTDouglasPeuckerGeometrySimplifier alloc] initWithTolerance: 1.0f / 320.0f]];

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item">
  {% highlight html %}

  COMING SOON...

  {% endhighlight %}
  </div>
  
</div>

The simplification makes rendering (GPU tasks) faster with the cost of some additional computation (CPU), and for aggressive simplification you can see decrease of line quality. So use it carefully, only when you need it.

# Listen events for map touches

## Introduction
Once you have implemented and set *MapEventListener* interface for MapView, this will get following events, with following data as their parameter value:

* **onMapMoved** - map moving, zooming, tilting, rotating. Note that this is called for each pixel movement, so any work in this method can make map panning animation visibly slower.
* **onMapClicked(MapClickInfo)** - map clicked, provides info about click:
  * **ClickType** - gives `CLICK_TYPE_SINGLE`, `CLICK_TYPE_LONG`, `CLICK_TYPE_DOUBLE` or `CLICK_TYPE_DUAL`
  * **ClickPos** - geographical click position (in map projection)
* **onVectorElementClicked(VectorElementsClickInfo)** - any vector object on map was clicked (could be line,polygon,point,marker,balloon popup or a 3D model)
  * **ClickType** - see above
  * **VectorElementClickInfos** - sorted list of clicked objects, nearest is first. Each ClickInfo has:
    * **ClickPos** - geographical click location
    * **ElementClickPos** - click locaton projected to element, different from ClickPos if click was not precisely on object (hint: it never really is).
    * **Distance** - distance from the vector element to the camera
    * **Order** - order of element within VectorElementClickInfos
    * **VectorElement** - actual clicked element


### 1. Implement MapEventListener

Create new class called **MyMapEventListener** which implements MapEventListner interface.

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

### 2. Initialize listener

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

  mapView.setMapEventListener(new MyMapEventListener(mapView, vectorDataSource));

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item">
  {% highlight html %}

  MapView.MapEventListener = new MapListener (dataSource);

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item">
  {% highlight html %}

  MyMapEventListener* mapListener = [[MyMapEventListener alloc] init];
  [self setMapEventListener:mapListener];

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item">
  {% highlight html %}

  COMING SOON...

  {% endhighlight %}
  </div>
  
</div>

## Show/hide labels for map clicks

MapEventListener, has special functionality: a small label text indicating info about clicked object, as a "Label". This is implemented in following way:

* If you click on an object, then it creates another map object:  **BalloonPopup** with a text from **metadata** of the clicked object. This is why in samples we add special Metadata field value. You can use metadata value directly, or use **object unique ID** as metadata, and then use database to query details about clicked object. 
* When user clicks new location on map, or object, then previous balloon is deleted, and new one is opened in new location

The BalloonPopup is added to a DataSource, which is linked to already existing Layer on map. We use same datasource what we use for other vector elements on map.

See [Event Listener guide](/guides/events) for sample code.

# Data sources

Following DataSources are available for Carto SDK 3.x on all supported
platforms (iOS, Android and others).

Some DataSources are more universal and could be used for different data
types and Layers: vector or raster, others can be used for specific
layer type only.

## 1. Offline maps from MBTiles {#offline-maps}

**MBTilesTileDataSource** is universal datasource for vector or raster
data, so it can be used two ways.

**a) MBtiles as RasterTileLayer**

For raster data it needs
[MBTiles](https://github.com/mapbox/mbtiles-spec) format files, see
[MBTiles guide](/guides/mbtiles) with list of some tools and sources to
generate them.

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

    MBTilesTileDataSource tileDataSource = new MBTilesTileDataSource(filePath);
    TileLayer rasterLayer = new RasterTileLayer(tileDataSource);

    mapView.getLayers().add(rasterLayer);

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item">
  {% highlight html %}
  
      var mbTilesLayer = new RasterTileLayer(new MBTilesTileDataSource(filePath));
      MapView.Layers.Add(mbTilesLayer);

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item">
  {% highlight html %}

   NSString* fullpathVT = [[NSBundle mainBundle] pathForResource:@"MBTILESFILENAME" ofType:@"mbtiles"];
   NTTileDataSource* tileDataSource = [[NTMBTilesTileDataSource alloc] initWithPath: fullpathVT];

   // Initialize a raster layer with the previous data source
   NTRasterTileLayer* rasterLayer = [[NTRasterTileLayer alloc] initWithDataSource:tileDataSource];

   // Add the raster layer to the map
   [[mapView getLayers] add:rasterLayer];

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item">
  {% highlight html %}

  COMING SOON...

  {% endhighlight %}
  </div>
  
</div>

**b) MBtiles as VectorTileLayer**

For own packaged vector data you need Carto specific vector files
packages (NTVT - *NutiTeq Vector Tile*) and styling files in Mapnik XML
format. Here is free sample packages with OpenStreetMap data:

-   [estonia\_ntvt.mbtiles](https://dl.dropboxusercontent.com/u/3573333/public_web/ntvt_packages/estonia_ntvt.mbtiles)

Carto SDK provides built-in download service called **Package
Manager** to get the map packages for a country or smaller region
easily, this is described in separate page: [learn
more](/guides/offline-maps) . Sample here is for case when you have
**own generated MBTiles file with vector data**.

Vector maps needs always proper **style definition**. You can find
compatible stylesheet files from sample code, these are zip files which
have **Mapnik XML styling** with resources (fonts, images) inside. You
can use ones with sample projects, e.g. OSMBright with 3D buildings
[osmbright.zip](https://github.com/nutiteq/hellomap3d-android/blob/master/com.nutiteq.advancedmap3/assets/osmbright.zip).
See [Vector Tile Styling](/guides/vector-styles) for details about
styling.

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
  {% highlight html %}

    NSString* fullpathVT = [[NSBundle mainBundle] pathForResource:@"estonia_ntvt" ofType:@"mbtiles"];
    NTTileDataSource* tileDataSource = [[NTMBTilesTileDataSource alloc] initWithMinZoom:0 maxZoom:4 path: fullpathVT];
    
    // 2. Load vector tile styleset
    NTBinaryData *vectorTileStyleSetData = [NTAssetUtils loadAsset: @"osmbright.zip"];
    NTZippedAssetPackage *package = [[NTZippedAssetPackage alloc] initWithZipData:vectorTileStyleSetData];
    NTCompiledStyleSet *vectorTileStyleSet = [[NTCompiledStyleSet alloc] initWithAssetPackage:package];
    
    // 3. Create vector tile decoder using the styleset
    NTMBVectorTileDecoder *vectorTileDecoder = [[NTMBVectorTileDecoder alloc] initWithCompiledStyleSet:vectorTileStyleSet];
    
    // 4. Create vector tile layer, using previously created data source and decoder
    NTVectorTileLayer *layer = [[NTVectorTileLayer alloc] initWithDataSource:tileDataSource decoder:vectorTileDecoder];
    
    // 5. Add vector tile layer
    [[mapView getLayers] add:layer];

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item">
  {% highlight html %}

  COMING SOON...

  {% endhighlight %}
  </div>
  
</div>

## 2. Online maps

**HTTPTileDataSource** is also universal datasource for vector or raster
data.

**a) Carto online Vector Tile Layer**

Carto SDK provides support for built-in vector tiles, as provided by
Carto Maps as a Service packages and it is available for all plans.
This is suitable as universial base map.

For vector styling you use exactly same osmbright.zip file as for
offline tiles, see above.

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
  
        BinaryData styleAsset = AssetUtils.loadAsset("nutibright-v2a.zip");
        VectorTileLayer baseLayer = new CartoOnlineVectorTileLayer("nutiteq.osm", new ZippedAssetPackage(styleAsset));

        mapView.getLayers().add(baseLayer);

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item">
  {% highlight html %}

      var styleAsset = AssetUtils.LoadAsset("nutibright-v2a.zip");
      var baseLayer = new CartoOnlineVectorTileLayer("nutiteq.osm", new ZippedAssetPackage(styleAsset));
      
      MapView.Layers.Add(baseLayer);

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item">
  {% highlight html %}

    NTBinaryData* styleData = [NTAssetUtils loadAsset:@"nutibright-v3.zip"];
    NTAssetPackage* assetPackage = [[NTZippedAssetPackage alloc] initWithZipData:styleData];
    NTVectorTileLayer* vectorTileLayer = [[NTCartoOnlineVectorTileLayer alloc] initWithSource: @"nutiteq.osm" styleAssetPackage:assetPackage];

    [[mapView getLayers] add:vectorTileLayer];
  
  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item">
  {% highlight html %}

  COMING SOON...

  {% endhighlight %}
  </div>
  
</div>

**b) Web (HTTP) tiles as VectorTileLayer**

With Carto Starter, Pro or Enterprise plans you can use also other
vector tile map data sources. It is similar to custom raster map data
sources, only adding vector decoder is needed.

Here we connect to MapBox vector tiles, which have very similar (but not
identical) data structure, so we can use same osmbright.zip vector
styling. Before using this in your app please check also from MapBox if
they allow this, and make sure you use your MapBox key in URL. You may
need to use your own vector tile server URL, and your suitable styling
instead.

Following tags are supported in URL definition: **zoom, x, y, xflipped,
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

      // 1. define data source
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
  {% highlight html %}

  COMING SOON...

  {% endhighlight %}
  </div>
  
</div>

**b) Web (HTTP) tiles as RasterTileLayer**

For online raster tiles you can use any common web tiles in PNG or JPG
formats, in Spherical Mercator system. See our [Raster tile sources
list](/guides/raster-tile-sources) .

Following tags are supported in URL definition: **zoom, x, y, xflipped,
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
  {% highlight html %}

  
        TileDataSource baseRasterTileDataSource = new HTTPTileDataSource(1, 19, "http://ecn.t3.tiles.virtualearth.net/tiles/a{quadkey}.jpeg?g=471&mkt=en-US");

        TileLayer baseLayer = new RasterTileLayer(baseRasterTileDataSource);

        mapView.getLayers().add(baseLayer);

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item">
  {% highlight html %}

  
      // Create a Bing raster data source. Note: tiles start from level 1, there is no single root tile!
      var baseRasterTileDataSource = new HTTPTileDataSource(1, 19, "http://ecn.t3.tiles.virtualearth.net/tiles/a{quadkey}.jpeg?g=471&mkt=en-US");

      // create layer and add to map
      var baseLayer = new RasterTileLayer(baseRasterTileDataSource);
      MapView.Layers.Add(baseLayer);

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item">
  {% highlight html %}

  NTHTTPTileDataSource* baseRasterTileDataSource = [[NTHTTPTileDataSource alloc] initWithMinZoom:0 maxZoom:19 baseURL:@"http://otile1.mqcdn.com/tiles/1.0.0/osm/{zoom}/{x}/{y}.png"];
    
    // Initialize a raster layer with the previous data source
    NTRasterTileLayer* rasterLayer = [[NTRasterTileLayer alloc] initWithDataSource:baseRasterTileDataSource];
    
    // Add the previous raster layer to the map
    [[mapView getLayers] add:rasterLayer];

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item">
  {% highlight html %}

  COMING SOON...

  {% endhighlight %}
  </div>
  
</div>

## 3. Application-defined vector overlay {#application-defined-vector-data}

Carto SDK has in-memory datasources where application can add vector
objects which are shown on map as overlay layer.

**LocalVectorDataSource** is a dynamic data source that is empty
initially and supports adding/removing objects. Objects are kept in
local memory and the state is not persistent. By default, all added
objects are always rendered. For detailed code samples see [map data
overlay](/guides/overlays) page, this has samples how to add Points,
Markers, Texts, Lines and Polygons to map.

## 4. Application-defined ground overlays {#ground-overlay}

Carto SDK supports **Ground Overlays** - bitmaps (PNG, JPG etc) which
are put to map to your defined location. Most commonly this is used for
**indoor floorplans**, but it can be used also for other cases.

**BitmapOverlayRasterTileDataSource** defines bitmap image, and
geographical coordinates of the bitmap. See [Ground Overlay
guide](/guides/ground-overlays) for more info.

## 5. Virtual data sources

There are some DataSources which take another DataSource as input, do
internal logic or processing and output DataSource itself. This way you
can pipe datasources to have customized logic.

-   **PersistentCacheTileDataSource**

Caches HTTP tiles to a persistent sqlite database file. If tile exists
in the database, then request to original data source is skipped. Can be
used for both raster and vector tiles. The data source takes original
source’s expires headers into account.

<div id="tabs6">
<ul>
<li>
<a href="#i6"><span>iOS ObjC</span></a></li>

<li>
<a href="#a6"><span>Android Java</span></a></li>

<li>
<a href="#n6"><span>.NET C\#</span></a></li>

</ul>
<div id="i6">
``` {.brush: .objc}
// Initialize a OSM raster data source from MapQuest Open Tiles
NTHTTPTileDataSource* baseRasterTileDataSource = [[NTHTTPTileDataSource alloc] initWithMinZoom:0 maxZoom:19 baseURL:@"http://otile1.mqcdn.com/tiles/1.0.0/osm/{zoom}/{x}/{y}.png"];

// Create persistent cache for the given data source  
NTPersistentCacheTileDataSource* cachedRasterTileDataSource = [[NTPersistentCacheTileDataSource alloc] initWithDataSource:baseRasterTileDataSource databasePath:[NTAssetUtils calculateWritablePath:@"mycache.db"]];

// Initialize a raster layer with the previous data source
NTRasterTileLayer* rasterLayer = [[NTRasterTileLayer alloc] initWithDataSource:cachedRasterTileDataSource];

// Add the previous raster layer to the map
[[self.mapView getLayers] add:rasterLayer];
```

</div>
<div id="a6">
``` {.brush: .java}

// Create a Bing raster data source. Note: tiles start from level 1, there is no single root tile!
TileDataSource baseRasterTileDataSource = new HTTPTileDataSource(1, 19, "http://ecn.t3.tiles.virtualearth.net/tiles/a{quadkey}.jpeg?g=471&mkt=en-US");

// Add persistent caching datasource, tiles will be stored locally on persistent storage
PersistentCacheTileDataSource cachedDataSource = 
  new PersistentCacheTileDataSource(baseRasterTileDataSource, getExternalFilesDir(null)+"/mapcache.db");

// Create layer and add to map
TileLayer baseLayer = new RasterTileLayer(cachedDataSource);
mapView.getLayers().add(baseLayer);
```

</div>
<div id="n6">
``` {.brush: .csharp}

// Create a Bing raster data source. Note: tiles start from level 1, there is no single root tile!
var baseRasterTileDataSource = new HTTPTileDataSource(1, 19, "http://ecn.t3.tiles.virtualearth.net/tiles/a{quadkey}.jpeg?g=471&mkt=en-US");

// Add persistent caching datasource, tiles will be stored locally on persistent storage
// fileDir must be a directory where files can be written - this is platform-specific
var cachedDataSource = 
  new PersistentCacheTileDataSource(baseRasterTileDataSource, fileDir+"/mapcache.db");


// create layer and add to map
var baseLayer = new RasterTileLayer(cachedDataSource);
mapView.Layers.Add(baseLayer);
```

</div>
</div>
-   **CompressedCacheTileDataSource**

An in-memory cache data source. Though layers also cache tiles, the
tiles are uncompressed and usually take 2-5x more memory after being
loaded. CompressedCacheTileDataSource keeps all tiles in compressed
format and thus makes better use of existing memory.

-   \***CombinedTileDataSource**

A tile data source that combines two data sources (usually offline and
online) and selects tiles based on zoom level. All requests below
specified zoom level are directed to the first data source and all
requests at or above specified zoom level are directed to the second
data source.

## 6. Other built-in data sources

-   **AssetTileDataSource** - loads offline tiles data from a folder
    (/sdcard or Application sandbox, depends on platform). You can use
    for example zoom/x/y.png or zoom\_x\_y.png file naming structures
    for offline data.
-   **NMLModelLODTreeOnlineDataSource** - uses NMLDB online API for “3D
    city” layer. 3D tools are provided as commercial service, contact
    Carto to get more info about it.
-   **NMLModelLODTreeSqliteDataSource** - An offline version of
    *NMLModelLODTreeOnlineDataSource*, model info is kept in sqlite
    database file. Note that simple individual 3D objects (NML models)
    can be added to usual *LocalVectorDataSource*.

## 7. Shapefiles, GeoTIFF an other GIS data dources {#shapefile}

With the GIS extension we provide **OGRVectorDataSource** and
**GDALRasterTileDataSource** which can open ESRI Shapefile, GeoTIFF,
KML, GML, GPX, MapInfo and many other GIS vector and raster formats
directly. Your code just defines file and layer to be used. Carto
provides it as commercial package, also free evaluation is avaliable,
email sales@nutiteq.com to get it.

## 8. Custom data sources

Last but not least - your application can define own Data Sources.

See *MyMergedRasterTileDataSource* in samples to get example for virtual
datasource which takes raster data from two raster datasources and
merges the map in pixel level. Note that the second datasources should
have a lot of transparent or semitransparent pixels, or the pixels from
the first datasource will be hidden.

Custom DataSource should extend one of following abstract base classes:

-   **TileDataSource** - for tiles (raster or vector)
-   **VectorDataSource** - for vector objects
-   **CacheTileDataSource** - for custom tile caching (it is a *TileDataSource*)


# Offline map service

## Introduction
Carto provides service to download base map packages in special vector format for offline use. There is no direct URL to get these packages, instead we provide cross-platform API from within SDK: **Package Manager API**. Application should use this API to download packages. The API provides following features:

* Listing of available packages (countries), get IDs and metadata of specific packages
* Start package download
* Pause and resume download
* Delete downloaded package
* Get status report of package state and download status (% downloaded)

Usage of API requires registration of license code.

### Typical scenarios
<img src = "http://share.gifyoutube.com/yaNw0r.gif" alt="Offline browser" align="right">

Currently we support following scenarios:

1. Provide **interactive list of all packages** to the user, who selects a country and downloads predefined map package. From app development point of view this is most complex, as you need to create nice UX. See advancedmap sample for it
2. You define **one specific country** what user wants to download. Your application provides UI button to start download; or even starts download automatically when user first starts your app. This is much simpler to implement, as there is no need for list and status handling. See below for example code for it.
3. Download **bounding box** map area. This is suitable for e.g. city guide apps where whole country is not needed. You can even let user to define own area of interest. See below example code for it.

### Available packages
* [List of available packages](/guides/packages) - see available package ID-s for country package downloads

## Usage

### Example code - complex scenario
We provide ready-made *advancedmap* examples: [**PackageManagerActivity**](https://github.com/nutiteq/hellomap3d-android/blob/master/com.nutiteq.advancedmap3/src/com/nutiteq/advancedmap3/PackageManagerActivity.java) for Android, and [**PackageMapController**](https://github.com/nutiteq/hellomap3d-ios/blob/master/advancedmap3/advancedmap3/PackageManagerController.mm) for iOS. These cover the more complex case - listing of all packages, and user can select one or many of them, and view the map.

### Event diagram
Following flow diagram displays key events and messages between app, Package Manager and online service.

![packagemanager flow](/images/pm_flow.png)


### Additional events
Application must handle properly following possible additional asynchronous events:

* *onPackageFailed* - package download failed, e.g. network connection issue
* *onPackageCancelled* - download canceled by application
* *onPackageListFailed* - package list download failed, probably network issue


### Steps for single package download
Following are steps to implement **single map package** downloading

* **Initialize CartoPackageManager**

<div id="tabs1">
  <ul>
    <li><a href="#i1"><span>iOS ObjC</span></a></li>
    <li><a href="#a1"><span>Android Java</span></a></li>
    <li><a href="#n1"><span>.NET C#</span></a></li>
  </ul>
<div id="i1">
<pre class="brush: objc">
    // Register license, this must be done before PackageManager can be used!
    [NTMapView registerLicense:"YOUR_LICENSE_HERE"];

    // Create folder for package manager files. Package manager needs persistent writable folder.
	NSArray* paths = NSSearchPathForDirectoriesInDomains(NSApplicationSupportDirectory, NSUserDomainMask,YES);
	NSString* appSupportDir = [paths objectAtIndex: 0];
	NSString* packagesDir = [appSupportDir stringByAppendingString:@"packages"];
	NSError *error;
	[[NSFileManager defaultManager] createDirectoryAtPath:packagesDir withIntermediateDirectories:NO attributes:nil error:&error];

	// Create package manager and package manager listener
	// we had defined packageManager and packageManagerListener already, as properties
	// currently the only package data source is nutiteq.mbstreets, it has OpenStreetMap global data
	_packageManager = [[NTCartoPackageManager alloc] initWithSource:@"nutiteq.mbstreets" dataFolder:packagesDir];
    [_packageManager start];
</pre>
</div>
<div id="a1">
<pre class="brush: java">
 // Register license, this must be done before PackageManager can be used!
 MapView.registerLicense("YOUR_LICENSE_HERE", getApplicationContext());
		
 // Create package manager
 File packageFolder = new File(getApplicationContext().getExternalFilesDir(null), "mappackages");
 if (!(packageFolder.mkdirs() || packageFolder.isDirectory())) {
    Log.e(Const.LOG_TAG, "Could not create package folder!");
 }
 packageManager = new CartoPackageManager("nutiteq.mbstreets", packageFolder.getAbsolutePath());
 packageManager.start();

</pre>
</div>
<div id="n1">
<pre class="brush: csharp">
 // 1) Android ONLY: Register license, this must be done before PackageManager can be used!
 MapView.RegisterLicense("YOUR-NUTITEQ-LICENSE", ApplicationContext);
 
 // Create/find folder for packages
 var packageFolder = new File (GetExternalFilesDir(null), "packages");
 if (!(packageFolder.Mkdirs() || packageFolder.IsDirectory)) {
   Log.Fatal("Could not create package folder!");
 }


 // 2) iOS ONLY: Register license, this must be done before PackageManager can be used!
 MapView.RegisterLicense("YOUR-NUTITEQ-LICENSE");

 // Find folder for packages
 var paths = NSSearchPath.GetDirectories (NSSearchPathDirectory.ApplicationSupportDirectory, NSSearchPathDomain.User);
 var packageFolder = paths [0] + "packages";
 NSFileManager.DefaultManager.CreateDirectory (packageFolder, true, null);

// Following code is identical for all platforms        

// Create package manager

// define PackageManager listener, definition is in same class above
var packageManager = new CartoPackageManager("nutiteq.mbstreets", packageFolder);
packageManager.PackageManagerListener = new PackageListener(packageManager);

// Download new package list only if it is older than 24h
// Note: this is only needed if pre-made packages are used
if (packageManager.ServerPackageListAge > 24 * 60 * 60) {
  packageManager.StartPackageListDownload ();
}

// Start manager - mandatory
packageManager.Start ();

</pre>
</div>
</div>

* **Implement and set PackageManagerListener** 

<div id="tabs2">
  <ul>
    <li><a href="#i2"><span>iOS ObjC</span></a></li>
    <li><a href="#a2"><span>Android Java</span></a></li>
    <li><a href="#n2"><span>.NET C#</span></a></li>
  </ul>
<div id="i2">
<pre class="brush: objc">

    // 1. define own listener object
    
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

    // following to your Controller class:

    // 3. create listener object instance
	_packageManagerListener = [[PackageManagerListener alloc] init];

	// 4. Register this controller with listener to receive notifications about events
	[_packageManagerListener addPackageManagerController:self];

	// 5. Attach package manager listener
	[_packageManager setPackageManagerListener:_packageManagerListener];
</pre>
</div>
<div id="a2">
<pre class="brush: java">
   packageManager.setPackageManagerListener(new PackageListener());
   packageManager.startPackageListDownload();
   
   
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
</pre>
</div>
<div id="n2">
<pre class="brush: csharp">

// to your main class:
packageManager.PackageManagerListener = new PackageListener(packageManager);
if (packageManager.ServerPackageListAge > 24 * 60 * 60) {
		packageManager.StartPackageListDownload ();
}

// PackageListener.cs:

using Carto.PackageManager;
using Carto.Utils;

namespace HelloMap
{

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
}

</pre>
</div>
</div>


*  **Download of country package**

Following starts download of map of Estonia. See [list of available packages](/guides/packages) for available countries. Generally packages are for ISO 2-letter administrative country codes, with two exceptions:

* Some bigger countries: USA, Canada, Russia, Germany and France have one level  subdivision 
* Some countries (UK, Spain) have are both options: whole country, and subdivisions


<div id="tabs3">
  <ul>
    <li><a href="#i3"><span>iOS ObjC</span></a></li>
    <li><a href="#a3"><span>Android Java</span></a></li>
    <li><a href="#n3"><span>.NET C#</span></a></li>
  </ul>
<div id="i3">
<pre class="brush: objc">
- (void)onPackageListUpdated
{
  // called when package list is downloaded
  // now you can start downloading packages
 [_packageManager startPackageDownload:@"EE"];
}
</pre>
</div>
<div id="a3">
<pre class="brush: java">
@Override
public void onPackageListUpdated() {
  // called when package list is downloaded
  // now you can start downloading packages
  packageManager.startPackageDownload("EE");
}
</pre>
</div>
<div id="n3">
<pre class="brush: csharp">
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
</pre>
</div>
</div>

*  **Download of bounding box**

If you do not need whole country, then you can define smaller area with bounding box. There is limitation of 5000 map tiles per download, depending on latitude it means about 150x150 km area. 

Bouding box is defined in format **bbox(lonMin,latMin,lonMax,latMax)**. You can use nice 3rd party [BoundingBox](http://boundingbox.klokantech.com/) web service to define  areas; use CSV format there to get appropriate format for coordinates.

This download does not require PackageList download, so you can start it right away when PackageManger is created.

<div id="tabs5">
  <ul>
    <li><a href="#i5"><span>iOS ObjC</span></a></li>
    <li><a href="#a5"><span>Android Java</span></a></li>
    <li><a href="#n5"><span>.NET C#</span></a></li>
  </ul>
<div id="i5">
<pre class="brush: objc">
  // get London greater area, near maximum area package size
 [_packageManager startPackageDownload:@"bbox(-0.8164,51.2383,0.6406,51.7402)"];

</pre>
</div>
<div id="a5">
<pre class="brush: java">


// London (about 30MB)
String bbox = "bbox(-0.8164,51.2382,0.6406,51.7401)"; 
if (packageManager.getLocalPackage(bbox) == null) {
	packageManager.startPackageDownload (bbox);
}
</pre>
</div>
<div id="n5">
<pre class="brush: csharp">

// London (about 30MB)
var bbox = "bbox(-0.8164,51.2382,0.6406,51.7401)"; 
if (packageManager.GetLocalPackage(bbox) == null) {
	packageManager.StartPackageDownload (bbox);
}
</pre>
</div>
</div>

* **Add datasource with offline map**

There is special Vector Tile Data Source: **PackageManagerTileDataSource** which needs to be used.

<div id="tabs4">
  <ul>
    <li><a href="#i4"><span>iOS ObjC</span></a></li>
    <li><a href="#a4"><span>Android Java</span></a></li>
    <li><a href="#n4"><span>.NET C#</span></a></li>
  </ul>
<div id="i4">
<pre class="brush: objc">
	// Load vector tile styleset
	NSString* styleAssetName = @"osmbright.zip";

	NTUnsignedCharVector *vectorTileStyleSetData = [NTAssetUtils LoadBytes:styleAssetName];
	NTMBVectorTileStyleSet *vectorTileStyleSet = [[NTMBVectorTileStyleSet alloc] initWithData:vectorTileStyleSetData];

	// Create vector tile decoder using the styleset and update style parameters
	NTMBVectorTileDecoder* vectorTileDecoder = [[NTMBVectorTileDecoder alloc] initWithStyleSet:vectorTileStyleSet];
	[vectorTileDecoder setStyleStringParameter:@"lang" value:@"en"];
	
	// Optional: enable 3D elements
    [self.vectorTileDecoder setStyleBoolParameter:@"buildings3d" value:YES];
    [self.vectorTileDecoder setStyleStringParameter:@"markers3d" value:@"1"];
		
	// Create tile data source from PackageManager
	NTTileDataSource* vectorTileDataSource = [[NTPackageManagerTileDataSource alloc] initWithPackageManager:_packageManager];
	
	// Create vector tile layer, using previously created data source and decoder
	NTVectorTileLayer* baseLayer = [[NTVectorTileLayer alloc] initWithDataSource:vectorTileDataSource decoder:self.vectorTileDecoder];

	// Add vector tile layer as first layer
	[[self getLayers] insert:0 layer:baseLayer];
</pre>
</div>
<div id="a4">
<pre class="brush: java">
 // Create layer with vector styling
UnsignedCharVector styleBytes = AssetUtils.loadBytes("osmbright.zip");
MBVectorTileDecoder vectorTileDecoder = null;
if (styleBytes != null){

// Create style set
  MBVectorTileStyleSet vectorTileStyleSet = new MBVectorTileStyleSet(styleBytes);
  vectorTileDecoder = new MBVectorTileDecoder(vectorTileStyleSet);
}

// create offline datasource from Package Manager
PackageManagerTileDataSource dataSource = new PackageManagerTileDataSource(packageManager);
       
VectorTileLayer baseLayer = new VectorTileLayer(vectorTileDataSource, vectorTileDecoder);
 
mapView.getLayers().add(baseLayer);
</pre>
</div>
<div id="n4">
<pre class="brush: csharp">
// define styling for vector map
UnsignedCharVector styleBytes = AssetUtils.LoadBytes("osmbright.zip");
MBVectorTileDecoder vectorTileDecoder = null;
if (styleBytes != null) {
	// Create style set
	MBVectorTileStyleSet vectorTileStyleSet = new MBVectorTileStyleSet (styleBytes);
	vectorTileDecoder = new MBVectorTileDecoder (vectorTileStyleSet);
} else {
	Log.Error ("Failed to load style data");
}

var baseLayer = new VectorTileLayer(new PackageManagerTileDataSource(packageManager),vectorTileDecoder);
mapView.Layers.Add(baseLayer);
</pre>
</div>
</div>


### Updating packages


There is no special event or method to check package updates, so updates can be checked and controlled by application using following logic. You can call this logic as soon as you feel appropriate. Carto Maps service has OSM offline package updates approximately once a month, different packages can be updated in different point of time, and with different frequency.

1. Use packagemanager *startPackageListDownload* to get server packages
2. Wait for listener's *onPackageListUpdated* event, now server packages were downloaded
3. Use packageManager *getLocalPackages* to get already downloaded packages (local packages)
4. Iterate list of local packages, check from metadata if server package list has newer version of some
5. If there is newer version, then this package is updated
6. Updated package download is like normal first download

During re-download of same package application shows old map until download is complete. So the update can run in background safely.

### Additional notes

* *startPackageListDownload* method does not need to be called every time when package list is required. In fact, once package list is successfully downloaded, it will remain locally available. But refreshing the contents once a day or perhaps once a week is recommended as some older packages may become available once newer versions are uploaded. *getServerPackageListAge* method can be used to check the age of the package list data.

* Package manager keeps persistent task queue of all submitted requests. Even when it is stopped, downloads will automatically resume when it is started next time.

* It is possible to pause downloads by setting task priority to -1. Downloads will resume once priority is set to non-negative number.

* *PackageInfo* class provides two methods for human-readable package names: *getName* and *getNames*. *getNames* takes language argument and will return localized name of the package (assuming the language is supported, currently only major languages such as English, German, French are supported). *getNames* returns list of names, as some packages can be placed under multiple categories. So, using *getNames* should be preferred over *getName*.

* Each package includes **tile mask**. Tile mask basically lists all tiles of the package up to a certain zoom level (currently 10). Tile mask can be used to quickly find a package corresponding to given tile coordinates or to find a package containing given tile.


