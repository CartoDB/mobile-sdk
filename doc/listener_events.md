# Map Listener Events

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

## Implement MapEventListener

Create a new class called **MyMapEventListener** which implements MapEventListner interface.

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

## Initialize Listener

Apply the following code to initialize listener events for map clicks.

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
  {% highlight html %}mapView.setMapEventListener(new MyMapEventListener(mapView, vectorDataSource));

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item">
  {% highlight html %}MapView.MapEventListener = new MapListener (dataSource);

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item">
  {% highlight html %}MyMapEventListener* mapListener = [[MyMapEventListener alloc] init];
  [self setMapEventListener:mapListener];

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item">
  {% highlight html %}COMING SOON...

  {% endhighlight %}
  </div>
  
</div>

## Show/Hide Labels for Map Clicks

MapEventListener contains special functionality that shows, or hides, a small label text over a clicked object. The following label click event rules apply:

- If you click on an object, it creates another map object as a **BalloonPopup**. This contains the text from the metadata of the clicked object

  For this reason, our samples apps include a _Metadata field value_. You can use the metadata value directly, or use the **object unique ID** as the metadata, to query details about the clicked object from the database.

- When a viewer clicks a new location on the map (or click another map object), the original BalloonPopup is deleted, and new one appears at the click location

- The BalloonPopup is added to the DataSource, which is linked existing map layers. The same DataSource is used for other vector elements on a map
