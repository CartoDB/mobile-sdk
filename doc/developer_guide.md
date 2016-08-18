# Developer Guide to SDK Features

The following advanced features can be customized by mobile app developers.

[Listening to events](#listening-to-events) | 
[Clusters to display multiple points of interest as objects](#clusters) |
[Ground overlays to project a bitmap image over a basemap](#ground-overlays) | 
[Vector styles for vector map tiles](#vector-styles)  |

## Listening to Events

Once you have implemented and set the *MapEventListener* interface for a MapView, the following listener events are generated as parameters:

* **onMapMoved** - map moving, zooming, tilting, rotating. This event is called for each pixel movement, so any changed to this parameter can make map panning animation visibly slower

* **onMapClicked(MapClickInfo)** - map clicked, provides info about click:

  * **ClickType** - results include `CLICK_TYPE_SINGLE`, `CLICK_TYPE_LONG`, `CLICK_TYPE_DOUBLE` or `CLICK_TYPE_DUAL`

  * **ClickPos** - geographical click position (in map projection)

* **onVectorElementClicked(VectorElementsClickInfo)** - any vector object on map was clicked (could be line,polygon,point,marker,balloon popup or a 3D model)

  * **ClickType** - same description as above, results include `CLICK_TYPE_SINGLE`, `CLICK_TYPE_LONG`, `CLICK_TYPE_DOUBLE` or `CLICK_TYPE_DUAL`

  * **VectorElementClickInfos** - sorted list of clicked objects, nearest is first. Each ClickInfo contains:

    * **ClickPos** - geographical click location

    * **ElementClickPos** - click locaton projected to element, different from ClickPos if click was not precisely on object (which it never really is)

    * **Distance** - distance from the vector element to the camera

    * **Order** - order of element within VectorElementClickInfos

    * **VectorElement** - the actual clicked element

### Implement MapEventListener

Create a new class called **MyMapEventListener** which implements MapEventListner interface.

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
  </ul>

  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--java is-active">
  {% highlight java %}

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

  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--csharp">
  {% highlight c# %}

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

  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--objective-c">
  {% highlight objc %}

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

  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--swift">
  {% highlight swift %}COMING SOON...

  {% endhighlight %}
  </div>
  
</div>

### Initialize Listener

Apply the following code to initialize listener events for map clicks.

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
  </ul>

  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--java is-active">
  {% highlight java %}mapView.setMapEventListener(new MyMapEventListener(mapView, vectorDataSource));

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--csharp">
  {% highlight c# %}MapView.MapEventListener = new MapListener (dataSource);

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--objective-c">
  {% highlight objc %}MyMapEventListener* mapListener = [[MyMapEventListener alloc] init];
  [self setMapEventListener:mapListener];

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--swift">
  {% highlight swift %}COMING SOON...

  {% endhighlight %}
  </div>
  
</div>

### Show/Hide Labels for Map Clicks

MapEventListener contains special functionality that shows, or hides, a small label text over a clicked object. The following label click event rules apply:

- If you click on an object, it creates another map object as a **BalloonPopup**. This contains the text from the metadata of the clicked object

  For this reason, our samples apps include a _Metadata field value_. You can use the metadata value directly, or use the **object unique ID** as the metadata, to query details about the clicked object from the database.

- When a viewer clicks a new location on the map (or click another map object), the original BalloonPopup is deleted, and new one appears at the click location

- The BalloonPopup is added to the DataSource, which is linked existing map layers. The same DataSource is used for other vector elements on a map

## Clusters

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

### API Methods for Clusters

Clusters are generated dynamically, based on `VectorDataSource` data that loads the map layer. If using an API, it works as a unique layer with the `ClusteredVectorLayer` method, and includes the following parameters in the a hierarchal order:

1. Select the layer `DataSource`

    In most cases,  the `LocalVectorDataSource` function contains all the elements to request the data. It is important that the DataSource displays all elements in a layer, and does not limit it to the current map visualization bbox (bounding box)

2. `ClusterElementBuilder` defines a single method `buildClusterElement`

### Define Cluster Data and Layer

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
  </ul>

  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--java is-active">
  {% highlight java %}

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

  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--csharp">
  {% highlight c# %}var proj = new EPSG3857();

// 1. Create overlay layer for markers
      var dataSource = new LocalVectorDataSource(proj);

// 2. Create Marker objects and add them to vectorDataSource.
      // **Note:** This depends on the _app type_ of your mobile app settings. See samples with JSON loading

      var layer = new ClusteredVectorLayer(dataSource, new MyClusterElementBuilder());
      layer.MinimumClusterDistance = 20; // in pixels

      MapView.Layers.Add(layer);

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--objective-c">
  {% highlight objc %}

// 1. Initialize a local vector data source
      NTProjection* proj = [[mapView getOptions] getBaseProjection];
      NTLocalVectorDataSource* vectorDataSource = [[NTLocalVectorDataSource alloc] initWithProjection:proj];
    
// 2. Create Marker objects and add them to vectorDataSource.
      // **Note:** This depends on the _app type_ of your mobile app settings. See AdvancedMap for samples with JSON loading and random point generation
    
// 3. Create element builder
      MyMarkerClusterElementBuilder* clusterElementBuilder = [[MyMarkerClusterElementBuilder alloc] init];
    
// 4. Initialize a vector layer with the previous data source
      NTClusteredVectorLayer* vectorLayer = [[NTClusteredVectorLayer alloc] initWithDataSource:vectorDataSource clusterElementBuilder:clusterElementBuilder];
    
// 5. Add the previous vector layer to the map
      [[mapView getLayers] add:vectorLayer];

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--swift">
  {% highlight swift %}COMING SOON...

  {% endhighlight %}
  </div>
  
</div>

### Define ClusterElementBuilder

The Cluster Element Builder takes set of original markers (map objects) as input, and returns one object (or another `VectorElement`, such as a Point or BalloonPopup) which dynamically replaces the original marker.

**Note:** It is highly recommended to reuse and cache styles to reduce memory usage. For example, a marker style with a specific number is only created once. Android and iOS samples use platform-specific graphic APIs to generate the bitmap for the marker. .NET example only uses BalloonPopup, which is slower but works the same across all platforms.

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
  </ul>

  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--java is-active">
  {% highlight java %}private class MyClusterElementBuilder extends ClusterElementBuilder {

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

  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--csharp">
  {% highlight c# %}public class MyClusterElementBuilder : ClusterElementBuilder
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

  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--objective-c">
  {% highlight objc %}

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

  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--swift">
  {% highlight swift %}COMING SOON...

  {% endhighlight %}
  </div>
  
</div>

## Ground Overlays

Ground overlays projects a bitmap (PNG, JPG) image of a defined coordinate over a basemap. For example, a ground overlay bitmap may be used to show an indoor floorplan over a building map.

<span class="wrap-border"><img src="/docs/img/layout/mobile/ground_overlay.jpg" alt="Example of mobile ground overlays" /></span>

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

### Code Samples

This example uses only one geographical coordinate. The building size is known, and the building direction is facing north. This allows us to calculate other ground points with the code. Four ground control points are set to the corners of the bitmap, which typically returns the most accurate result. 

**Tip:** The following sample code assumes that you have the **jefferson-building-ground-floor.jpg** bitmap file as part of your application project.

- For Android, this image is located under *assets*
- In iOS, it can be located anywhere in your project

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
  </ul>

  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--java is-active">
  {% highlight java %}com.carto.graphics.Bitmap overlayBitmap = BitmapUtils.loadBitmapFromAssets("jefferson-building-ground-floor.jpg");

// 1. Create two vector containing geographical positions and corresponding raster image pixel coordinates
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

// 2. Create bitmap overlay raster tile data source
    BitmapOverlayRasterTileDataSource rasterDataSource = new BitmapOverlayRasterTileDataSource(0, 20, overlayBitmap, proj, mapPoses, bitmapPoses);
    RasterTileLayer rasterLayer = new RasterTileLayer(rasterDataSource);
    mapView.getLayers().add(rasterLayer);

// 3. Apply zoom level bias to the raster layer
    // - By default, bitmaps are upsampled on high-DPI screens
// 4. Correct this by applying appropriate bias
    float zoomLevelBias = (float) (Math.log(mapView.getOptions().getDPI() / 160.0f) / Math.log(2));
    rasterLayer.setZoomLevelBias(zoomLevelBias * 0.75f);
    rasterLayer.setTileSubstitutionPolicy(TileSubstitutionPolicy.TILE_SUBSTITUTION_POLICY_VISIBLE);

    mapView.setFocusPos(pos, 0);
    mapView.setZoom(16f, 0);

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--csharp">
  {% highlight csharp %}var overlayBitmap = BitmapUtils.LoadBitmapFromAssets("jefferson-building-ground-floor.jpg");

// 1. Create two vector geographical positions, and corresponding raster image pixel coordinates
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

// 2. Create bitmap overlay raster tile data source
      var rasterDataSource = new BitmapOverlayRasterTileDataSource(0, 20, overlayBitmap, proj, mapPoses, bitmapPoses);
      var rasterLayer = new RasterTileLayer(rasterDataSource);
      MapView.Layers.Add(rasterLayer);

// 3. Apply zoom level bias to the raster layer
      // - By default, bitmaps are upsampled on high-DPI screens

// 4. Correct this by applying appropriate bias
      float zoomLevelBias = (float)(Math.Log(MapView.Options.DPI / 160.0f) / Math.Log(2));
      rasterLayer.ZoomLevelBias = zoomLevelBias * 0.75f;
      rasterLayer.TileSubstitutionPolicy = TileSubstitutionPolicy.TileSubstitutionPolicyVisible;

      MapView.SetFocusPos(pos, 0);
      MapView.SetZoom(16f, 0);

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--objective-c">
  {% highlight objc %}

// 1. Load ground overlay bitmap
      NTBitmap *overlayBitmap = [NTBitmapUtils loadBitmapFromAssets:@"jefferson-building-ground-floor.jpg"];
    
// 2. Create two vector geographical positions, and corresponding raster image pixel coordinates
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
    
// 3. Create bitmap overlay raster tile data source
      NTBitmapOverlayRasterTileDataSource* rasterDataSource = [[NTBitmapOverlayRasterTileDataSource alloc] initWithMinZoom:0 maxZoom:20 bitmap:overlayBitmap projection:proj mapPoses:mapPoses bitmapPoses:bitmapPoses];
      NTRasterTileLayer* rasterLayer = [[NTRasterTileLayer alloc] initWithDataSource:rasterDataSource];
      [[mapView getLayers] add:rasterLayer];
    
// 4. Apply zoom level bias to the raster layer
      // - By default, bitmaps are upsampled on high-DPI screens

// 5. Correct this by applying appropriate bias
      float zoomLevelBias = log([[mapView getOptions] getDPI] / 160.0f) / log(2);
      [rasterLayer setZoomLevelBias:zoomLevelBias * 0.75f];
      [rasterLayer setTileSubstitutionPolicy:NT_TILE_SUBSTITUTION_POLICY_VISIBLE];
      
      [mapView setFocusPos:pos durationSeconds:0];
      [mapView setZoom:16f durationSeconds:0];

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--swift">
  {% highlight swift %}COMING SOON...

  {% endhighlight %}
  </div>
  
</div>

## Vector Styles

When you create a basemap, the Mobile SDK package enables you to select one of our pre-defined, built-in basemap styles:

 * CARTO_BASEMAP_STYLE_DEFAULT - general street map
 * CARTO_BASEMAP_STYLE_DARK - CARTO Dark Matter style
 * CARTO_BASEMAP_STYLE_GRAY - CARTO Positron style

For the Enterprise account plans,  you can customize basemap styling by setting the colors, transparency, line styles (width, patterns, casings, endings), polygon patterns, icons, text placements, fonts, and other vector data parameters. 

CARTO uses [Mapnik](http://mapnik.org) XML style description language for customizing the visual style of vector tiles. Our styling is optimized for mobile and contain some unique style parameters. In general, you can reuse your existing Mapnik XML, or CartoCSS, styling files and tools (such as TileMill/Mapbox Studio).

Vector styling is applied in the mobile client, where the style files are bundled with the application installer. The application can change the styling anytime, without reloading vector map data. This enables you to download map data once, and change styling from "day mode" to "night mode" with no new downloads.

CARTO map rendering implementation is intended for real-time rendering. As a result, several limitations apply.

### Mapnik Style Format

Mapnik style definitions are a common file format for map styles, based on XML. It was originally created for Mapnik, but is used by other software, such as our Mobile SDK. The file format specifications are located in this [XMLConfigReference](https://github.com/mapnik/mapnik/wiki/XMLConfigReference) document. There are several ways you can apply these styles:

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

### Creating Vector Style Packages

CARTO vector styles are distributed as zip-archives. All style-related files and folders must be placed into a single zip file.

The most important part of the style is the style definition file, typically named _project.xml_. This file contains style descriptions for all layers, and usually references other files, such as fonts, icons, and pattern bitmaps (which should be placed in various subfolders).

### Vector Style Performance 

When using vector styles, the following recommendations are suggested for optimal performance:

- **Multiple symbolizers per layer may have very large performance hits**. If possible, move each symbolizer into separate layer

- `BuildingSymbolizer` requires an expensive OpenGL frame buffer read-back operation, and may perform very poorly on some devices (such as the original iPad Retina)

- To increase performance, it is suggested to use power-of-two dimensions for bitmaps

### SDK Extensions for Mapnik XML style files

The following CARTO specific extensions are specific to Mapnik XML style files.

#### NutiParameters

_NutiParameters_ describe additional parameters that can be used in styles and controlled in the code (from `MBVectorTileDecoder`).

- Parameters are typed, have default values and can be used as variables within _nuti_ namespace in the style (for example, `[nuti::lang]`)

- Some parameters may have _ prefix in their name. Such variables are reserved and should not be updated directly by the application

The following is a simple example of _NutiParameters_ section, located in the style xml file:

{% highlight xml %} 
`<NutiParameters>`
`  <NutiParameter name="lang" type="string" value="en" />`
`</NutiParameters>`
{% endhighlight %}

#### Metavariables

Metavariables add support for dynamic variable look-up. Variable names may depend on other variables. For example, `[name_[nuti::lang]]`.

If the value of `nuti::lang` is 'en', this metavariable expression would be equal to *[name_en]* expression.

#### Conditional Operator

Conditional operator ?: adds support for simple control flow management. It is similar to C language conditional operator and can be used in all expressions. For example:

`[nuti::lang] == 'en' ? 'English' : 'Other'`

#### 3D Texts and Markers

Mapnik `MarkersSymbolizer` and `TextSymbolizer` support additional values _nutibillboard_ for _placement_ parameter. This enables texts and markers to act as billboards (always facing the viewer), even when screen is tilted. This option can be used to give markers and texts more '3D-like' appearance. For example,

`<MarkersSymbolizer placement="nutibillboard" fill="#666666" file="icon/[maki]-12.svg" />`

### Supported Symbolizers and Parameters

The following lists contains all supported symbolizers and parameters for vector styles:

- `PointSymbolizer`: file, opacity, allow-overlap, ignore-placement, transform

- `BuildingSymbolizer`: fill, fill-opacity, height, geometry-transform

- `LineSymbolizer`: stroke, stroke-width, stroke-opacity, stroke-linejoin, stroke-linecap, stroke-dasharray, geometry-transform

- `LinePatternSymbolizer`: file, fill, opacity, geometry-transform

- `PolygonSymbolizer`: fill, fill-opacity, geometry-transform

- `PolygonPatternSymbolizer`: file, fill, opacity, geometry-transform

- `MarkersSymbolizer`: file, placement, marker-type, fill, opacity, width, height, spacing, allow-overlap, ignore-placement, transform

- `TextSymbolizer`: name, face-name, fontset-name, placement, size, spacing, fill, opacity, halo-fill, halo-opacity, halo-radius, allow-overlap, minimum-distance, text-transform, orientation, dx, dy, wrap-width, wrap-before, character-spacing, line-spacing, horizontal-alignment, vertical-alignment

- `ShieldSymbolizer`: name, face-name, fontset-name, placement, size, spacing, fill, opacity, halo-fill, halo-opacity, halo-radius, allow-overlap, minimum-distance, text-transform, orientation, dx, dy, wrap-width, wrap-before, character-spacing, line-spacing, horizontal-alignment, vertical-alignment, file, shield-dx, shield-dy, unlock-image

### CartoCSS Extensions for Vector Styles

`Metavariables` and `nutiparameters` are also available as CartoCSS style extensions.
