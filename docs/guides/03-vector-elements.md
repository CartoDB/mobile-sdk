### Vector Elements

In following examples, **Vector Elements** (Markers, Points, Lines, Polygons, Texts and BalloonPopups) are added by the application.
For each object, the styling is defined and objects are created based on given coordinates.

The examples assume `LocalVectorVectorDataSource` object named 'vectorDataSource1' that
is attached to a `VectorLayer` which is added to a `MapView`.

**Note:** A popup (callout, bubble) which appears when you click on map is a vector element of its own, and should be added using map click listener. For details, see [Add a BalloonPopup](#add-a-balloonpopup).

#### Add a Marker

Add a marker and apply marker styling using the following code:

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
    <li class="Tab js-Tabpanes-navItem--lang">
      <a href="#/3" class="js-Tabpanes-navLink--lang js-Tabpanes-navLink--lang--swift">Swift</a>
    </li>
    <li class="Tab js-Tabpanes-navItem--lang">
      <a href="#/3" class="js-Tabpanes-navLink--lang js-Tabpanes-navLink--lang--kotlin">Kotlin</a>
    </li>
  </ul>

  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--java is-active">
    {% highlight java %}
    // 1. Create marker style
    MarkerStyleBuilder markerStyleBuilder = new MarkerStyleBuilder();
    markerStyleBuilder.setSize(30);
    markerStyleBuilder.setColor(new Color(0xFF00FF00)); // green

    MarkerStyle markerStyle1 = markerStyleBuilder.buildStyle();

    // 2. Add marker
    MapPos pos1 = proj.fromWgs84(new MapPos(24.646469, 59.426939)); // Tallinn
    Marker marker1 = new Marker(pos1, markerStyle1);

    // 3. Add the marker to the datasource
    vectorDataSource1.add(marker1);
    {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--csharp">
    {% highlight csharp %}
    // 1. Create marker style
    MarkerStyleBuilder markerStyleBuilder = new MarkerStyleBuilder();
    markerStyleBuilder.Size = 30;
    markerStyleBuilder.Color = new Color(0, 255, 0, 255); // green
    MarkerStyle markerStyle1 = markerStyleBuilder.BuildStyle();

    // 2. Define marker position and create the marker
    MapPos pos1 = proj.FromWgs84(new MapPos(24.646469, 59.426939)); // Tallinn
    Marker marker1 = new Marker(pos1, markerStyle1);

    // 3. Add the marker to the datasource
    vectorDataSource1.Add(marker1);
    {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--objective-c">
    {% highlight objc %}
    // 1. Create a marker style, using default marker bitmap here
    NTMarkerStyleBuilder* markerStyleBuilder = [[NTMarkerStyleBuilder alloc] init];
    [markerStyleBuilder setSize:30];
    [markerStyleBuilder setColor:[[NTColor alloc] initWithColor:0xFF00FF00]]; // green
    NTMarkerStyle* markerStyle1 = [markerStyleBuilder buildStyle];

    // 2. Define marker position and create the marker
    NTMapPos* pos1 = [proj fromWgs84:[[NTMapPos alloc] initWithX:24.651488 y:59.423581]]; // Tallinn
    NTMarker* marker1 = [[NTMarker alloc] initWithPos:pos1 style:markerStyle1];

    // 3. Add the marker to the data source
    [vectorDataSource1 add:marker1];
    {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--swift">
    {% highlight swift %}
    // 1. Create a marker style, using default marker bitmap here
    let markerStyleBuilder = NTMarkerStyleBuilder();
    markerStyleBuilder?.setSize(30);
    markerStyleBuilder?.setColor(NTColor.init(r: 0, g: 255, b: 0, a: 255)); // green
    let markerStyle1 = markerStyleBuilder?.buildStyle();

    // 2. Define marker position and create the marker
    let pos1 = projection?.fromWgs84(NTMapPos(x: 24.651488, y: 59.423581)); // Tallinn
    let marker1 = NTMarker(pos: pos1, style: markerStyle1);

    // 3. Add the marker to the data source
    vectorDataSource1?.add(marker1);
    {% endhighlight %}
  </div>
    
  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--kotlin">
    {% highlight kotlin %}
    // 1. Create a marker style, using default marker bitmap here
    val markerStyleBuilder = MarkerStyleBuilder()
    markerStyleBuilder.size = 30F
    markerStyleBuilder.color = Color(0, 255, 0, 255) // green
    val markerStyle1 = builder.buildStyle()

    // 2. Define marker position and create the marker
    val pos1 = projection?.fromWgs84(MapPos(24.651488, 59.423581)) // Tallinn
    val marker1 = Marker(pos1, markerStyle1)

    // 3. Add the marker to the data source
    vectorDataSource1.add(marker1)
    {% endhighlight %}
  </div>
    
</div>

##### Example Marker on a Mobile Map

![pin](https://raw.githubusercontent.com/CartoDB/mobile-dotnet-samples/master/images/pin.png)

#### Add a Point

Points are used to indicating specific location points on a map, similar to Markers. However, Points do not have overlapping controls and cannot be use with billboard style version 2.5D. If you have a lot of data (thousands of points) and are not using 2.5D views, use Points as an alternative to Markers. Your rendering time will be significantly faster.

You can add any type of vector objects to the same Layer and `LocalVectorDataSource`. This enables you to reuse settings for a defined Marker. It is recommended to define different Layers and DataSources for managing your objects, as it allows you to:

-  Select and delete all objects of DataSource

-  Specify the drawing order of layers. The drawing order within a single Layer is undefined. For multiple layers, layers that are added are drawn underneath the initial map layer

Add a point and apply point styling using the following code:

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
    <li class="Tab js-Tabpanes-navItem--lang">
      <a href="#/3" class="js-Tabpanes-navLink--lang js-Tabpanes-navLink--lang--swift">Swift</a>
    </li>
    <li class="Tab js-Tabpanes-navItem--lang">
      <a href="#/3" class="js-Tabpanes-navLink--lang js-Tabpanes-navLink--lang--kotlin">Kotlin</a>
    </li>
  </ul>

  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--java is-active">
    {% highlight java %}
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
    {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--csharp">
    {% highlight csharp %}
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
    {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--objective-c">
    {% highlight objc %}
    // 1. Set point position
    NTMapPos* tallinn = [proj fromWgs84:[[NTMapPos alloc] initWithX:24.646469 y:59.426939]];

    // 2. Create style and position for the Point
    NTPointStyleBuilder* pointStyleBuilder = [[NTPointStyleBuilder alloc] init];
    [pointStyleBuilder setColor:[[NTColor alloc] initWithColor:0xFF00FF00]];
    [pointStyleBuilder setSize:16];

    // 3. Create Point, add to datasource with metadata
    NTPoint* point1 = [[NTPoint alloc] initWithPos:tallinn style:[pointStyleBuilder buildStyle]];
    [point1 setMetaDataElement:@"ClickText" element:[[NTVariant alloc] initWithString:@"Point 1"]];

    [vectorDataSource1 add:point1];
    {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--swift">
    {% highlight swift %}
    // 1. Set marker position
    let tallinn = projection?.fromWgs84(NTMapPos(x: 24.646469, y: 59.426939))

    // 2. Create style and position for the Point
    let pointStyleBuilder = NTPointStyleBuilder()
    pointStyleBuilder?.setColor(NTColor(r: 0, g: 255, b: 0, a: 255))
    pointStyleBuilder?.setSize(16)

    // 3. Create Point, add to datasource with metadata
    let point1 = NTPoint(pos: tallinn, style: pointStyleBuilder?.buildStyle())
    point1?.setMetaData("ClickText", element: NTVariant(string: "Point nr 1"));

    vectorDataSource1?.add(point1)
    {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--kotlin">
    {% highlight kotlin %}
    // 1. Set marker position
    val tallinn = projection?.fromWgs84(MapPos(24.646469, 59.426939))

    // 2. Create style and position for the Point
    val pointStyleBuilder = PointStyleBuilder()
    pointStyleBuilder.color = Color(0, 255, 0, 255)
    pointStyleBuilder.size = 16F

    // 3. Create Point, add to datasource with metadata
    val point1 = Point(tallinn, pointStyleBuilder.buildStyle())
    point1.setMetaDataElement("ClickText", Variant("Point nr 1"))

    vectorDataSource1.add(point1)
    {% endhighlight %}
  </div>
  
</div>

#### Add a Line

Lines can be added to the same VectorDataSource. Add a line and apply line styling using the following code:

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
    <li class="Tab js-Tabpanes-navItem--lang">
      <a href="#/3" class="js-Tabpanes-navLink--lang js-Tabpanes-navLink--lang--swift">Swift</a>
    </li>
    <li class="Tab js-Tabpanes-navItem--lang">
      <a href="#/3" class="js-Tabpanes-navLink--lang js-Tabpanes-navLink--lang--kotlin">Kotlin</a>
    </li>
  </ul>

  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--java is-active">
    {% highlight java %}
    // 1. Create line style, and line poses
    LineStyleBuilder lineStyleBuilder = new LineStyleBuilder();
    lineStyleBuilder.setColor(new Color(0xFFFF0000));
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
    line1.setMetaDataElement("ClickText", new Variant("Line nr 1"));
    vectorDataSource1.add(line1);
    {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--csharp">
    {% highlight csharp %}
    // 1. Create line style, and line poses
    var lineStyleBuilder = new LineStyleBuilder();
    lineStyleBuilder.Color = new Color(255, 0, 0, 255); // Red
    lineStyleBuilder.LineJoinType = LineJoinType.LineJoinTypeRound;
    lineStyleBuilder.Width = 8;

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
    {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--objective-c">
    {% highlight objc %}
    // 1. Define line style
    NTLineStyleBuilder* lineStyleBuilder = [[NTLineStyleBuilder alloc] init];
    [lineStyleBuilder setColor:[[NTColor alloc] initWithColor:0xFFFF0000]];
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
    {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--swift">
    {% highlight swift %}
    // 1. Create line style, and line poses
    let lineStyleBuilder = NTLineStyleBuilder()
    lineStyleBuilder?.setColor(NTColor(r: 255, g: 0, b: 0, a: 255))
    lineStyleBuilder?.setLineJoinType(NTLineJoinType.LINE_JOIN_TYPE_ROUND)
    lineStyleBuilder?.setWidth(8)

    // 2. Special MapPosVector must be used for coordinates
    let linePoses = NTMapPosVector()
    let initial = projection?.fromWgs84(NTMapPos(x: 24.645565, y: 59.422074))

    // 3. Add positions
    linePoses?.add(initial)
    linePoses?.add(projection?.fromWgs84(NTMapPos(x: 24.643076, y: 59.420502)));
    linePoses?.add(projection?.fromWgs84(NTMapPos(x: 24.645351, y: 59.419149)));
    linePoses?.add(projection?.fromWgs84(NTMapPos(x: 24.648956, y: 59.420393)));
    linePoses?.add(projection?.fromWgs84(NTMapPos(x: 24.650887, y: 59.422707)));

    // 4. Add a line
    let line1 = NTLine(poses: linePoses, style: lineStyleBuilder?.buildStyle());
    line1?.setMetaData("ClickText", element: NTVariant(string: "Line nr 1"))

    vectorDataSource1?.add(line1)
    {% endhighlight %}
  </div>
    
  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--kotlin">
    {% highlight kotlin %}
    // 1. Create line style, and line poses
    val lineStyleBuilder = LineStyleBuilder()
    lineStyleBuilder.color = Color(255, 0, 0, 255)
    // Define how lines are joined
    lineStyleBuilder.lineJoinType = LineJoinType.LINE_JOIN_TYPE_ROUND
    lineStyleBuilder.width = 8F

    // 2. Special MapPosVector must be used for coordinates
    val linePoses = MapPosVector()
    val initial = projection?.fromWgs84(MapPos(24.645565, 59.422074))

    // 3. Add positions
    linePoses.add(initial)
    linePoses.add(projection?.fromWgs84(MapPos(24.643076, 59.420502)));
    linePoses.add(projection?.fromWgs84(MapPos(24.645351, 59.419149)));
    linePoses.add(projection?.fromWgs84(MapPos(24.648956, 59.420393)));
    linePoses.add(projection?.fromWgs84(MapPos(24.650887, 59.422707)));

    // 4. Add a line
    val line1 = Line(linePoses, lineStyleBuilder.buildStyle());
    line1.setMetaDataElement("ClickText", Variant("Line nr 1"))

    vectorDataSource1.add(line1)
    {% endhighlight %}
  </div>
    
</div>

##### Example Line on a Mobile Map

![line](https://raw.githubusercontent.com/CartoDB/mobile-dotnet-samples/master/images/line.png)

#### Add a Polygon

Add a polygon and apply polygon styling using the following code. The following examples add a polygon with polygon holes:

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
    <li class="Tab js-Tabpanes-navItem--lang">
      <a href="#/3" class="js-Tabpanes-navLink--lang js-Tabpanes-navLink--lang--swift">Swift</a>
    </li>
    <li class="Tab js-Tabpanes-navItem--lang">
      <a href="#/3" class="js-Tabpanes-navLink--lang js-Tabpanes-navLink--lang--kotlin">Kotlin</a>
    </li>
  </ul>

  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--java is-active">
    {% highlight java %}
    // 1. Create polygon style and poses
    PolygonStyleBuilder polygonStyleBuilder = new PolygonStyleBuilder();
    polygonStyleBuilder.setColor(new Color(0xFFFF0000)); // red
    lineStyleBuilder = new LineStyleBuilder();
    lineStyleBuilder.setColor(new Color(0xFF000000)); // black
    lineStyleBuilder.setWidth(1.0f);
    polygonStyleBuilder.setLineStyle(lineStyleBuilder.buildStyle());

    // 2. Define coordinates of outer ring
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

    // 3. Create 2 polygon holes
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
    {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--csharp">
    {% highlight csharp %}
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
    MapPosVector holePoses1 = new MapPosVector();
    holePoses1.Add(proj.FromWgs84(new MapPos(24.658409, 59.420522)));
    holePoses1.Add(proj.FromWgs84(new MapPos(24.658409, 59.420522)));
    holePoses1.Add(proj.FromWgs84(new MapPos(24.662207, 59.418896)));
    holePoses1.Add(proj.FromWgs84(new MapPos(24.662207, 59.417411)));
    holePoses1.Add(proj.FromWgs84(new MapPos(24.659524, 59.417171)));
    holePoses1.Add(proj.FromWgs84(new MapPos(24.657615, 59.419834)));

    MapPosVector holePoses2 = new MapPosVector();
    holePoses2.Add(proj.FromWgs84(new MapPos(24.665640, 59.421243)));
    holePoses2.Add(proj.FromWgs84(new MapPos(24.668923, 59.419463)));
    holePoses2.Add(proj.FromWgs84(new MapPos(24.662893, 59.419365)));

    MapPosVectorVector polygonHoles = new MapPosVectorVector();
    polygonHoles.Add(holePoses1);
    polygonHoles.Add(holePoses2);

    // 4. Add polygon
    Polygon polygon = new Polygon(polygonPoses, polygonHoles, polygonStyleBuilder.BuildStyle());
    polygon.SetMetaDataElement("ClickText", new Variant("Polygon"));

    vectorDataSource1.Add(polygon);
    {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--objective-c">
    {% highlight objc %}
    // 1. Create polygon style
    NTPolygonStyleBuilder* polygonStyleBuilder = [[NTPolygonStyleBuilder alloc] init];
    [polygonStyleBuilder setColor:[[NTColor alloc] initWithColor:0xFFFF0000]];
    lineStyleBuilder = [[NTLineStyleBuilder alloc] init];
    [lineStyleBuilder setColor:[[NTColor alloc] initWithColor:0xFF000000]];
    [lineStyleBuilder setWidth:1.0f];
    [polygonStyleBuilder setLineStyle:[lineStyleBuilder buildStyle]];

    // 2. Define coordinates of outer ring
    NTMapPosVector* polygonPoses = [[MapPosVector alloc] init];
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

    // 3. Define polygon holes. This is two-dimensional array (MapPosVectorVector)
    // because Polygon can have several holes. In this sample there are two
    NTMapPosVector* holePoses1 = [[MapPosVector alloc] init];
    [holePoses1 add:[proj fromWgs84:[[NTMapPos alloc] initWithX:24.658409 y:59.420522]]];
    [holePoses1 add:[proj fromWgs84:[[NTMapPos alloc] initWithX:24.662207 y:59.418896]]];
    [holePoses1 add:[proj fromWgs84:[[NTMapPos alloc] initWithX:24.662207 y:59.417411]]];
    [holePoses1 add:[proj fromWgs84:[[NTMapPos alloc] initWithX:24.659524 y:59.417171]]];
    [holePoses1 add:[proj fromWgs84:[[NTMapPos alloc] initWithX:24.657615 y:59.419834]]];

    NTMapPosVector* holePoses2 = [[MapPosVector alloc] init];
    [holePoses2 add:[proj fromWgs84:[[NTMapPos alloc] initWithX:24.665640 y:59.421243]]];
    [holePoses2 add:[proj fromWgs84:[[NTMapPos alloc] initWithX:24.668923 y:59.419463]]];
    [holePoses2 add:[proj fromWgs84:[[NTMapPos alloc] initWithX:24.662893 y:59.419365]]];

    NTMapPosVectorVector* polygonHoles = [[MapPosVectorVector alloc] init];
    [holes add:holePoses1];
    [holes add:holePoses2];

    // 4. Create polygon, define metadata and add to datasource
    NTPolygon* polygon = [[NTPolygon alloc] initWithGeometry:[[NTPolygonGeometry alloc] initWithPoses:polygonPoses holes:polygonHoles] style:[polygonStyleBuilder buildStyle]];
    [polygon setMetaDataElement:@"ClickText" element:[[NTVariant alloc] initWithString:@"Polygon"]];

    [vectorDataSource1 add:polygon];
    {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--swift">
    {% highlight swift %}
    // 1. Create polygon style and poses
    let polygonStyleBuilder = NTPolygonStyleBuilder()
    polygonStyleBuilder?.setColor(NTColor(r: 255, g: 0, b: 0, a: 255))
    let lineStyleBuilder = NTLineStyleBuilder()
    lineStyleBuilder?.setColor(NTColor(r: 0, g: 0, b: 255, a: 255))
    lineStyleBuilder?.setWidth(1)
    polygonStyleBuilder?.setLineStyle(lineStyleBuilder?.buildStyle())

    // 2. Define coordinates of outer ring
    let polygonPoses = NTMapPosVector()
    let initial = projection?.fromWgs84(NTMapPos(x: 24.650930, y: 59.421659))
    polygonPoses?.add(initial)
    polygonPoses?.add(projection?.fromWgs84(NTMapPos(x: 24.657453, y: 59.416354)))
    polygonPoses?.add(projection?.fromWgs84(NTMapPos(x: 24.661187, y: 59.414607)))
    polygonPoses?.add(projection?.fromWgs84(NTMapPos(x: 24.667667, y: 59.418123)))
    polygonPoses?.add(projection?.fromWgs84(NTMapPos(x: 24.665736, y: 59.421703)))
    polygonPoses?.add(projection?.fromWgs84(NTMapPos(x: 24.661444, y: 59.421245)))
    polygonPoses?.add(projection?.fromWgs84(NTMapPos(x: 24.660199, y: 59.420677)))
    polygonPoses?.add(projection?.fromWgs84(NTMapPos(x: 24.656552, y: 59.420175)))
    polygonPoses?.add(projection?.fromWgs84(NTMapPos(x: 24.654010, y: 59.421472)))

    // 3. Create 2 polygon holes
    let holePoses1 = NTMapPosVector()
    holePoses1?.add(projection?.fromWgs84(NTMapPos(x: 24.658409, y: 59.420522)))
    holePoses1?.add(projection?.fromWgs84(NTMapPos(x: 24.662207, y: 59.418896)))
    holePoses1?.add(projection?.fromWgs84(NTMapPos(x: 24.662207, y: 59.417411)))
    holePoses1?.add(projection?.fromWgs84(NTMapPos(x: 24.659524, y: 59.417171)))
    holePoses1?.add(projection?.fromWgs84(NTMapPos(x: 24.657615, y: 59.419834)))

    let holePoses2 = NTMapPosVector()
    holePoses2?.add(projection?.fromWgs84(NTMapPos(x: 24.665640, y: 59.421243)))
    holePoses2?.add(projection?.fromWgs84(NTMapPos(x: 24.668923, y: 59.419463)))
    holePoses2?.add(projection?.fromWgs84(NTMapPos(x: 24.662893, y: 59.419365)))

    let polygonHoles = NTMapPosVectorVector()
    polygonHoles?.add(holePoses1)
    polygonHoles?.add(holePoses2)

    // 4. Add polygon
    let polygonStyle = polygonStyleBuilder?.buildStyle()
    let polygon = NTPolygon(poses: polygonPoses, holes: polygonHoles, style: polygonStyle)
    polygon?.setMetaData("ClickText", element: NTVariant(string: "Polygon"))

    vectorDataSource1?.add(polygon)
    {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--kotlin">
    {% highlight kotlin %}
    // 1. Create polygon style and poses
    val polygonStyleBuilder = PolygonStyleBuilder()
    polygonStyleBuilder.color = Color(0xFFFF0000.toInt())
    val lineStyleBuilder = LineStyleBuilder()
    lineStyleBuilder.color = Color(0, 0, 255, 255)
    lineStyleBuilder.width = 1F
    polygonStyleBuilder.lineStyle = lineStyleBuilder.buildStyle()

    // 2. Define coordinates of outer ring
    val polygonPoses = MapPosVector()
    val initial = projection?.fromWgs84(MapPos(24.650930, 59.421659))
    polygonPoses.add(initial)
    polygonPoses.add(projection?.fromWgs84(MapPos(24.657453, 59.416354)))
    polygonPoses.add(projection?.fromWgs84(MapPos(24.661187, 59.414607)))
    polygonPoses.add(projection?.fromWgs84(MapPos(24.667667, 59.418123)))
    polygonPoses.add(projection?.fromWgs84(MapPos(24.665736, 59.421703)))
    polygonPoses.add(projection?.fromWgs84(MapPos(24.661444, 59.421245)))
    polygonPoses.add(projection?.fromWgs84(MapPos(24.660199, 59.420677)))
    polygonPoses.add(projection?.fromWgs84(MapPos(24.656552, 59.420175)))
    polygonPoses.add(projection?.fromWgs84(MapPos(24.654010, 59.421472)))

    // 3. Create 2 polygon holes
    val holePoses1 = MapPosVector()
    holePoses1.add(projection?.fromWgs84(MapPos(24.658409, 59.420522)))
    holePoses1.add(projection?.fromWgs84(MapPos(24.662207, 59.418896)))
    holePoses1.add(projection?.fromWgs84(MapPos(24.662207, 59.417411)))
    holePoses1.add(projection?.fromWgs84(MapPos(24.659524, 59.417171)))
    holePoses1.add(projection?.fromWgs84(MapPos(24.657615, 59.419834)))

    val holePoses2 = MapPosVector()
    holePoses2.add(projection?.fromWgs84(MapPos(24.665640, 59.421243)))
    holePoses2.add(projection?.fromWgs84(MapPos(24.668923, 59.419463)))
    holePoses2.add(projection?.fromWgs84(MapPos(24.662893, 59.419365)))

    val polygonHoles = MapPosVectorVector()
    polygonHoles.add(holePoses1)
    polygonHoles.add(holePoses2)

    // 4. Add polygon
    val polygon = Polygon(polygonPoses, polygonHoles, polygonStyleBuilder.buildStyle())
    polygon.setMetaDataElement("ClickText", Variant("Polygon"))
    vectorDataSource1?.add(polygon)
    {% endhighlight %}
  </div>
    
</div>

##### Example Polygon on a Mobile App

![polygon](https://raw.githubusercontent.com/CartoDB/mobile-dotnet-samples/master/images/polygon.png)

#### Add Text

Text style parameters are similar to Markers, as both are Billboards - which are MapView objects that contain the following features:

- Control and prohibit text overlapping, based on priority and the location of where the text appears

- Display text as billboards in 2.5D (tilted) view by defining the `OrientationMode` parameter. There are 3 options: show on ground, and rotate with map (like street names), show on ground (do not rotate with map), or show as billboard (no rotation).

Add text and apply text styling using the following code.

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
    <li class="Tab js-Tabpanes-navItem--lang">
      <a href="#/3" class="js-Tabpanes-navLink--lang js-Tabpanes-navLink--lang--swift">Swift</a>
    </li>
    <li class="Tab js-Tabpanes-navItem--lang">
      <a href="#/3" class="js-Tabpanes-navLink--lang js-Tabpanes-navLink--lang--kotlin">Kotlin</a>
    </li>
  </ul>

  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--java is-active">
    {% highlight java %}
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
    {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--csharp">
    {% highlight csharp %}
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
    {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--objective-c">
    {% highlight objc %}
    // 1. Create text style
    NTTextStyleBuilder* textStyleBuilder = [[NTTextStyleBuilder alloc] init];
    [textStyleBuilder setColor:[[NTColor alloc] initWithColor:0xFFFF0000]];
    [textStyleBuilder setOrientationMode:NT_BILLBOARD_ORIENTATION_FACE_CAMERA];
    // setScaleWithDPI enables higher resolution texts for retina devices,
    // but consumes more memory and is slower if you have many texts on map
    [textStyleBuilder setScaleWithDPI:false];

    // 2. Define text location and add to datasource
    NTMapPos* position = [proj fromWgs84:[[NTMapPos alloc] initWithX:24.653302 y:59.422269]];
    NTText* textpopup1 = [[NTText alloc] initWithPos:position style:[textStyleBuilder buildStyle] text:@"Face camera text"];  
    [textpopup1 setMetaDataElement:@"ClickText" element:[[NTVariant alloc] initWithString:@"Text 1"]];

    [vectorDataSource1 add:textpopup1];
    {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--swift">
    {% highlight swift %}
    // 1. Create text style
    let textStyleBuilder = NTTextStyleBuilder()
    textStyleBuilder?.setColor(NTColor(r: 255, g: 0, b: 0, a: 255))
    textStyleBuilder?.setOrientationMode(NTBillboardOrientation.BILLBOARD_ORIENTATION_FACE_CAMERA)
    // This enables higher resolution texts for retina devices, but consumes more memory and is slower
    textStyleBuilder?.setScaleWithDPI(false)

    // 2. Add text
    let position = projection?.fromWgs84(NTMapPos(x: 24.653302, y: 59.422269))
    let textpopup1 = NTText(pos: position, style: textStyleBuilder?.buildStyle(), text: "Face camera text")
    textpopup1?.setMetaData("ClickText", element: NTVariant(string: "Text nr 1"))

    vectorDataSource1.add(textpopup1)
    {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--kotlin">
    {% highlight kotlin %}
    // 1. Create text style
    val textStyleBuilder = TextStyleBuilder()
    textStyleBuilder.color = Color(0xFFFF0000.toInt())
    textStyleBuilder.orientationMode = BillboardOrientation.BILLBOARD_ORIENTATION_FACE_CAMERA
    // This enables higher resolution texts for retina devices, but consumes more memory and is slower
    textStyleBuilder.isScaleWithDPI = false

    // 2. Add text
    val position = projection?.fromWgs84(MapPos(24.653302, 59.422269))
    val textpopup1 = Text(position, textStyleBuilder.buildStyle(), "Face camera text")
    textpopup1.setMetaDataElement("ClickText", Variant("Text nr 1"))

    vectorDataSource1.add(textpopup1)
    {% endhighlight %}
  </div>  
</div>

##### Example Text on a Mobile Map

![text](https://raw.githubusercontent.com/CartoDB/mobile-dotnet-samples/master/images/text.png)

#### Add a BalloonPopup

A BalloonPopup appears based on click event of an object. You can also add a defined BalloonPopup. Add BalloonPops using the following code. Note that these examples contain several special styling elements, such as:

- Image (_info.png_) as the "Left Image"
- Arrow (_arrow.png_)  as the "Right Image"
- Styled appearance of tuned radius values

**Tip:** To use these styling elements as part of your own project, copy the above .png files, (available from the [Sample Apps](/docs/carto-engine/mobile-sdk/01-getting-started/#sample-apps)), to your local map application project.

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
    <li class="Tab js-Tabpanes-navItem--lang">
      <a href="#/3" class="js-Tabpanes-navLink--lang js-Tabpanes-navLink--lang--swift">Swift</a>
    </li>
        <li class="Tab js-Tabpanes-navItem--lang">
      <a href="#/3" class="js-Tabpanes-navLink--lang js-Tabpanes-navLink--lang--kotlin">Kotlin</a>
    </li>
  </ul>

  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--java is-active">
    {% highlight java %}
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
    {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--csharp">
    {% highlight csharp %}
    // 1. Load bitmaps to show on the label

    // Android code
    Bitmap androidInfoBitmap = BitmapFactory.DecodeResource(Resources, HelloMap.Resource.Drawable.info);
    Carto.Graphics.Bitmap infoBitmap = BitmapUtils.CreateBitmapFromAndroidBitmap(androidInfoBitmap);

    Bitmap androidArrowBitmap = BitmapFactory.DecodeResource(Resources, HelloMap.Resource.Drawable.arrow);
    Carto.Graphics.Bitmap arrowBitmap = BitmapUtils.CreateBitmapFromAndroidBitmap(androidArrowBitmap);

    // iOS code
    var infoBitmap = BitmapUtils.CreateBitmapFromUIImage(UIImage.FromFile("info.png"));
    var arrowBitmap = BitmapUtils.CreateBitmapFromUIImage(UIImage.FromFile("arrow.png"));

    // 2. Add popup

    // Shared code
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
    popup.setMetaDataElement("ClickText", new Variant("Popup caption nr 1"));

    vectorDataSource1.Add(popup);
    {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--objective-c">
    {% highlight objc %}
    // 1. Load bitmaps to show on the label
    UIImage* infoImage = [UIImage imageNamed:@"info.png"];
    UIImage* arrowImage = [UIImage imageNamed:@"arrow.png"];

    // 2. Add popup
    NTBalloonPopupStyleBuilder* balloonPopupStyleBuilder = [[NTBalloonPopupStyleBuilder alloc] init];
    [balloonPopupStyleBuilder setCornerRadius:20];
    [balloonPopupStyleBuilder setLeftMargins:[[NTBalloonPopupMargins alloc] initWithLeft:6 top:6 right:6 bottom:6]];
    [balloonPopupStyleBuilder setLeftImage:infoImage];
    [balloonPopupStyleBuilder setRightImage:arrowImage];
    [balloonPopupStyleBuilder setRightMargins:[[NTBalloonPopupMargins alloc] initWithLeft:2 top:6 right:12 bottom:6]];
    [balloonPopupStyleBuilder setPlacementPriority:1];

    NTMapPos* position = [proj fromWgs84:[[NTMapPos alloc] initWithX:24.655662 y:59.425521]];
    NTBalloonPopup* popup = [[NTBalloonPopup alloc] initWithPos:position
                               style:[balloonPopupStyleBuilder buildStyle]
                               title:@"Popup with pos"
                               desc:@"Images, round"];
    [popup setMetaDataElement:@"ClickText" element:[[NTVariant alloc] initWithString:@"Popup caption 1"]];

    [vectorDataSource1 add:popup];
    {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--swift">
    {% highlight swift %}
    // 1. Load bitmaps to show on the label
    let infoImage = NTBitmapUtils.createBitmap(from: UIImage(named: "info.png"));
    let arrowImage = NTBitmapUtils.createBitmap(from: UIImage(named: "arrow.png"));

    // 2. Add popup
    let builder = NTBalloonPopupStyleBuilder()
    builder?.setCornerRadius(20)
    builder?.setLeftMargins(NTBalloonPopupMargins(left: 6, top: 6, right: 6, bottom: 6))
    builder?.setLeftImage(infoImage)
    builder?.setRightImage(arrowImage)
    builder?.setRightMargins(NTBalloonPopupMargins(left: 2, top: 6, right: 12, bottom: 6))
    builder?.setPlacementPriority(1)

    let position = projection?.fromWgs84(NTMapPos(x: 24.655662, y: 59.425521))
    let popup = NTBalloonPopup(pos: position, style: builder?.buildStyle(), title: "Popup with pos", desc: "Images, round")
    popup?.setMetaData("ClickText", element: NTVariant(string: "Popup caption nr 1"))

    vectorDataSource1.add(popup)
    {% endhighlight %}
  </div>
    
  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--kotlin">
    {% highlight kotlin %}
    // 1. Load bitmaps to show on the label
    val infoImage = BitmapFactory.decodeResource(resources, R.drawable.info)
    val arrowImage = BitmapFactory.decodeResource(resources, R.drawable.arrow)

    // 2. Add popup
    val builder = BalloonPopupStyleBuilder()
    builder.cornerRadius = 20
    builder.leftMargins = BalloonPopupMargins(6, 6, 6, 6)
    builder.leftImage = BitmapUtils.createBitmapFromAndroidBitmap(infoImage)
    builder.rightImage = BitmapUtils.createBitmapFromAndroidBitmap(arrowImage)
    builder.rightMargins = BalloonPopupMargins(2, 6, 12, 6)
    builder.placementPriority = 1

    val position = projection?.fromWgs84(MapPos(24.655662, 59.425521))
    val popup = BalloonPopup(position, builder.buildStyle(), "Popup with pos", "Images, round")
    popup.setMetaDataElement("ClickText", Variant("Popup caption nr 1"))

    vectorDataSource1.add(popup)
    {% endhighlight %}
  </div>
    
</div>

##### Example BalloonPopup on a Mobile Map

![popup](https://raw.githubusercontent.com/CartoDB/mobile-dotnet-samples/master/images/popup.png)

#### Add 3D Model Objects

One special feature of the Mobile SDK is that you can add 3D objects (models) to a mobile map. For example, you can add small moving car or other decorative or informative elements.

**Note:** 3D objects are added to the same `LocalVectorDataSource` as 2D objects. However, 3D objects are only supported using the *Nutiteq 3D Model* (**NML**) format. This custom format is optimized for the multi-resolution of 3D files on mobile apps. _For details about tuning the performance of 3D models, see [`LocalVectorDataSource` Performance](#localvectordatasource-performance)._

The following procedure describes how to setup and add a 3D object to your mobile MapView:

1. Select a _NML file_

    **Tip:** You can retrieve some free samples from [Nutiteq's NML sample page](https://github.com/nutiteq/hellomap3d/wiki/NML-model-samples).

    If you have own model as Collada DAE (or KMZ) format, then you would need **CARTO Mobile 3D converter tools** to convert it to NML, so it can be used in mobile. Please [contact us](mailto:sales@carto.com) for more information.

2. Adjust the file size of the 3D object for rendering

    **Note:** Typically, NML files are smaller than 100K. Anything larger than that takes too long to render. Consider applying lower detail models for your 3D objects. You can then add these models to the **res/raw** folder of your Android app, or as a **resource file** in your iOS project.

3. Load the model file in the code to add it to your map application by using the following code:

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
    <li class="Tab js-Tabpanes-navItem--lang">
      <a href="#/3" class="js-Tabpanes-navLink--lang js-Tabpanes-navLink--lang--swift">Swift</a>
    </li>
    <li class="Tab js-Tabpanes-navItem--lang">
      <a href="#/3" class="js-Tabpanes-navLink--lang js-Tabpanes-navLink--lang--kotlin">Kotlin</a>
    </li>
  </ul>

  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--java is-active">
    {% highlight java %}
    // 1. Load NML model from a file
    BinaryData modelData = AssetUtils.loadAsset("fcd_auto.nml");

    // 2. Set location for model, and create NMLModel object with this
    MapPos position = baseProjection.fromWgs84(new MapPos(24.646469, 59.423939));
    NMLModel model = new NMLModel(position, modelData);
    model.setMetaDataElement("ClickText", new Variant("Single model"));

    // 3. Adjust the size- oversize it by 20*, just to make it more visible (optional)
    model.setScale(20);

    // 4. Add it to normal datasource
    vectorDataSource1.add(model);
    {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--csharp">
    {% highlight csharp %}
    // 1. Load NML model from a file
    var modelData = AssetUtils.LoadAsset("fcd_auto.nml");

    // 2. Set location for model, and create NMLModel object with this
    var position = proj.FromWgs84(new MapPos(24.646469, 59.423939));
    var model = new NMLModel(position, modelData);
    model.SetMetaDataElement("ClickText", new Variant("Single model"));

    // 3. Adjust the size- oversize it by 20x, just to make it more visible (optional)
    model.Scale = 20;

    // 4. Add it to normal datasource
    vectorDataSource1.Add(model);
    {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--objective-c">
    {% highlight objc %}
    // 1. Load NML model from a file
    NTBinaryData* modelData = [NTAssetUtils loadAsset:@"fcd_auto.nml"];

    // 2. Set location for model, and create NMLModel object with this
    NTMapPos* pos = [proj fromWgs84:[[NTMapPos alloc] initWithX:24.646469 y:59.424939]];
    NTNMLModel* model = [[NTNMLModel alloc] initWithPos:pos sourceModelData:modelData];
    [model setMetaDataElement:@"ClickText" element:[[NTVariant alloc] initWithString:@"My nice car"]];    

    // 3. Adjust the size- oversize it by 20*, just to make it more visible (optional)
    [model setScale:20];

    // 4. Add it to normal datasource
    [vectorDataSource1 add:model];
    {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--swift">
    {% highlight swift %}
    // 1. Load NML model from a file (be sure it's targeted by your application)
    let modelData = NTAssetUtils.loadAsset("fcd_auto.nml")

    // 2. Set location for model, and create NMLModel object with this
    let position = projection?.fromWgs84(NTMapPos(x: 24.646469, y: 59.423939))
    let model = NTNMLModel(pos: position, sourceModelData: modelData)
    model?.setMetaData("ClickText", element: NTVariant(string: "Single model"))

    // 3. Adjust the size- oversize it by 20*, just to make it more visible (optional)
    model?.setScale(20)

    // 4. Add it to normal datasource
    vectorDataSource1.add(model)
    {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--kotlin">
    {% highlight kotlin %}
    // 1. Load NML model from a file
    val modelData = AssetUtils.loadAsset("fcd_auto.nml")

    // 2. Set location for model, and create NMLModel object with this
    val position = projection?.fromWgs84(MapPos(24.646469, 59.423939))
    val model = NMLModel(position, modelData);
    model.setMetaDataElement("ClickText", Variant("Single model"))

    // 3. Adjust the size- oversize it by 20*, just to make it more visible (optional)
    model.scale = 20F

    // 4. Add it to normal datasource
    vectorDataSource1.add(model)
    {% endhighlight %}
  </div>
    
</div>

##### Example 3D Model Object on a Mobile Map

![3d animated](https://github.com/CartoDB/mobile-ios-samples/blob/gh-pages/carto-mobile-sdk-animated.gif?raw=true)

##### LocalVectorDataSource Performance

You can define the `spatialIndexType` of the `LocalVectorDataSource` to improve the performance of how a 3D model loads, by defining a spatial index to the DataSource. By default, no spatial index is defined. The following rules apply:

-  Do not define a spatial index if the number of elements is small, below ~1000 points or markers (or even less if you have complex lines and polygons). _This is the default option_

-  Apply `NT_LOCAL_SPATIAL_INDEX_TYPE_KDTREE` as the index type if there are a larger number of elements 

The advantage of defining a spatial index is that CPU usage decreases for large number of objects, improving the map performance of panning and zooming. However, displaying overlays may slightly delay the map response, as the spatial index is not loaded immediately when your move the map, it only moves after some hundred milliseconds. 

The overall maximum number of objects on map is limited to the RAM available for the app. Systems define several hundred MB for iOS apps, and closer to tens of MB for Android apps, but it depends on the device and app settings (as well as the density of the data). It is recommended to test your app with the targeted mobile platform and full dataset for the actual performance. 

The following code describes how to adjust the `LocalVectorDataSource` performance by defining a spatial index:

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
    <li class="Tab js-Tabpanes-navItem--lang">
      <a href="#/3" class="js-Tabpanes-navLink--lang js-Tabpanes-navLink--lang--swift">Swift</a>
    </li>
    <li class="Tab js-Tabpanes-navItem--lang">
      <a href="#/3" class="js-Tabpanes-navLink--lang js-Tabpanes-navLink--lang--kotlin">Kotlin</a>
    </li>
  </ul>

  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--java is-active">
    {% highlight java %}
    LocalVectorDataSource vectorDataSource2 = new LocalVectorDataSource(proj, LocalSpatialIndexType.LOCAL_SPATIAL_INDEX_TYPE_KDTREE);
    {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--csharp">
    {% highlight csharp %}
    var vectorDataSource2 = new LocalVectorDataSource(proj, LocalSpatialIndexType.LocalSpatialIndexTypeKdtree);
    {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--objective-c">
    {% highlight objc %}
    NTLocalVectorDataSource* vectorDataSource2 =
        [[NTLocalVectorDataSource alloc] initWithProjection:proj
                                         spatialIndexType: NTLocalSpatialIndexType::NT_LOCAL_SPATIAL_INDEX_TYPE_KDTREE];
    {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--swift">
    {% highlight swift %}
    let vectorDataSource2 = NTLocalVectorDataSource(projection: projection, spatialIndexType: NTLocalSpatialIndexType.LOCAL_SPATIAL_INDEX_TYPE_KDTREE)
    {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--kotlin">
    {% highlight kotlin %}
    val vectorDataSource2 = LocalVectorDataSource(projection, LocalSpatialIndexType.LOCAL_SPATIAL_INDEX_TYPE_KDTREE)
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
    <li class="Tab js-Tabpanes-navItem--lang is-active">
      <a href="#/0" class="js-Tabpanes-navLink--lang js-Tabpanes-navLink--lang--java">Java</a>
    </li>
    <li class="Tab js-Tabpanes-navItem--lang">
      <a href="#/1" class="js-Tabpanes-navLink--lang js-Tabpanes-navLink--lang--csharp">C#</a>
    </li>
    <li class="Tab js-Tabpanes-navItem--lang">
      <a href="#/2" class="js-Tabpanes-navLink--lang js-Tabpanes-navLink--lang--objective-c">Objective-C</a>
    </li>
    <li class="Tab js-Tabpanes-navItem--lang">
      <a href="#/3" class="js-Tabpanes-navLink--lang js-Tabpanes-navLink--lang--swift">Swift</a>
    </li>
    <li class="Tab js-Tabpanes-navItem--lang">
      <a href="#/3" class="js-Tabpanes-navLink--lang js-Tabpanes-navLink--lang--kotlin">Kotlin</a>
    </li>
  </ul>

  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--java is-active">
    {% highlight java %}
    vectorDataSource2.setGeometrySimplifier(new DouglasPeuckerGeometrySimplifier(1.0f / 320.0f));
    {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--csharp">
    {% highlight csharp %}
    vectorDataSource2.GeometrySimplifier = new DouglasPeuckerGeometrySimplifier(1.0f / 320.0f);
    {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--objective-c">
    {% highlight objc %}
    [vectorDataSource2 setGeometrySimplifier:
      [[NTDouglasPeuckerGeometrySimplifier alloc] initWithTolerance: 1.0f / 320.0f]];
    {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--swift">
    {% highlight swift %}
    vectorDataSource2?.setGeometrySimplifier(NTDouglasPeuckerGeometrySimplifier(tolerance: 1.0 / 320.0))
    {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--kotlin">
    {% highlight kotlin %}
    vectorDataSource2.geometrySimplifier = DouglasPeuckerGeometrySimplifier(1.0f / 320.0f)
    {% endhighlight %}
  </div>
    
</div>

The automatic simplification makes renderingfaster with some additional computation on the CPU.

