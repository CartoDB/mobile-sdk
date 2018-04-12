## Listening to Events

SDK contains various **Listener** hooks that are used to receive
various events from the SDK. 

There are listeners for user interaction related events, rendering events and
map download events among others.

### Listening to Map Events

An app can register a custom `MapEventListener` to receive notifications
about various events, such as:

* Map is being moved or zoomed (`onMapMoved`)

* Map has reached 'stable state', all tiles and data has been loaded displayed (`onMapStable`)

* Map has been clicked (`onMapClicked`)

**NOTE**: If you wish to listen to vector element clicks, you need to implement `VectorElementEventListener` and attach it to a `Vectorlayer`.

#### Implementing MapEventListener

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
    <li class="Tab js-Tabpanes-navItem--lang">
      <a href="#/3" class="js-Tabpanes-navLink--lang js-Tabpanes-navLink--lang--kotlin">Kotlin</a>
    </li>
  </ul>

  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--java is-active">
    {% highlight java %}

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
        }

        @Override
        public void onMapClicked(MapClickInfo mapClickInfo) {

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

            BalloonPopupStyle style = styleBuilder.buildStyle();

            MapPos wgs84Clickpos = mapView.getOptions().getBaseProjection().toWgs84(clickPos);
            String msg = String.format(Locale.US, "%.4f, %.4f", wgs84Clickpos.getY(), wgs84Clickpos.getX());

            BalloonPopup clickPopup = new BalloonPopup(clickPos, style, clickMsg, msg);

            vectorDataSource.add(clickPopup);
            oldClickLabel = clickPopup;
        }
    }

    {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--csharp">
    {% highlight c## %}

    public class MyMapListener : MapEventListener
    {
        MapView mapView;
        LocalVectorDataSource dataSource;
		
        BalloonPopup _oldClickLabel;

        public MyMapListener(MapView mapView, LocalVectorDataSource dataSource)
        {
            this.mapView = mapView;
            this.dataSource = dataSource;
        }

        public override void OnMapClicked(MapClickInfo mapClickInfo)
        {
            // A note about iOS: DISABLE 'Optimize PNG files for iOS' option in iOS build settings,
            // otherwise icons can not be loaded using AssetUtils/Bitmap constructor as Xamarin converts
            // PNGs to unsupported custom format.

            // Remove old click label
            if (_oldClickLabel != null)
            {
                dataSource.Remove(_oldClickLabel);
                _oldClickLabel = null;
            }

            var styleBuilder = new BalloonPopupStyleBuilder();
            // Make sure this label is shown on top all other labels
            styleBuilder.PlacementPriority = 10;

            // Check the type of the click
            ClickType type = mapClickInfo.ClickType;
                	
            string clickMsg = "Unknown click Type";

            if (type == ClickType.ClickTypeSingle)
            {
                clickMsg = "Single map click!";
            }
            else if (type == ClickType.ClickTypeLong)
            {
                clickMsg = "Long map click!";
            }
            else if (type == ClickType.ClickTypeDouble)
            {
                clickMsg = "Double map click!";
            }
            else if (type == ClickType.ClickTypeDual)
            {
                clickMsg = "Dual map click!";
            }

            MapPos position = mapClickInfo.ClickPos;
            BalloonPopupStyle style = styleBuilder.BuildStyle();

            MapPos wgs84Position = mapView.Options.BaseProjection.ToWgs84(position);
            string description = "" + wgs84Position.Y + " - " + wgs84Position.X;
            var clickPopup = new BalloonPopup(position, style, clickMsg, description);

            dataSource.Add(clickPopup);
            _oldClickLabel = clickPopup;
        }

        public override void OnMapMoved()
        {

        }
    }

    {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--objective-c">
    {% highlight objc %}

    @interface  MyMapEventListener : NTMapEventListener

    @property NTMapView* mapView;
    @property NTLocalVectorDataSource* vectorDataSource;

    @property NTBalloonPopup* oldClickLabel;

    -(void)setMapView:(NTMapView*)mapView vectorDataSource:(NTLocalVectorDataSource*)vectorDataSource;
    -(void)onMapMoved;
    -(void)onMapClicked:(NTMapClickInfo*)mapClickInfo;

    @end

    @implementation MyMapEventListener

    -(void)setMapView:(NTMapView*)mapView vectorDataSource:(NTLocalVectorDataSource*)vectorDataSource
    {
        self.mapView = mapView;
        self.vectorDataSource = vectorDataSource;
    }

    -(void)onMapMoved
    {
        // called very often, even just console logging can lag map movement animation
    }

    -(void)onMapClicked:(NTMapClickInfo*)mapClickInfo
    {
        if (self.oldClickLabel != nil) {
            [self.vectorDataSource remove:self.oldClickLabel];
            self.oldClickLabel = nil;
        }
        
        NSString* clickMsg;
        // Check the type of the click
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
        
        NSString* description = [NSString stringWithFormat:@"%f, %f", [wgs84Clickpos getY], [wgs84Clickpos getX]];
        
        NTBalloonPopupStyleBuilder* builder = [[NTBalloonPopupStyleBuilder alloc] init];
        [builder setPlacementPriority:10];
        NTBalloonPopupStyle* style = [builder buildStyle];
        
        NTBalloonPopup* popup = [[NTBalloonPopup alloc] initWithPos:clickPos style:style title:clickMsg desc:description];
        
        [self.vectorDataSource add:popup];
        self.oldClickLabel = popup;
    }

    @end

    {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--swift">
    {% highlight swift %}
  
    public class MyMapEventListener : NTMapEventListener {
        var mapView: NTMapView?
        var vectorDataSource: NTLocalVectorDataSource?

        var oldClickLabel: NTBalloonPopup?
        
        convenience init(mapView: NTMapView?, vectorDataSource: NTLocalVectorDataSource?) {
            
            self.init()
            
            self.mapView = mapView
            self.vectorDataSource = vectorDataSource
        }
        
        public override func onMapMoved() {
            
            // This is called every time the map moves, and we can calculate positions as such:
            
            var topLeft = mapView?.screen(toMap: NTScreenPos(x: 0, y: 0))
            
            let width: Float = Float((mapView?.frame.width)!)
            let height: Float = Float((mapView?.frame.width)!)
            
            var bottomRight = mapView?.screen(toMap: NTScreenPos(x: width, y: height))
        }
        
        public override func onMapClicked(_ mapClickInfo: NTMapClickInfo!) {
            
            // Remove old click label
            if (oldClickLabel != nil) {
                vectorDataSource?.remove(oldClickLabel)
                oldClickLabel = nil
            }
            
            let clickPos = mapClickInfo?.getClickPos()
            
            let styleBuilder = NTBalloonPopupStyleBuilder();
            // Make sure this label is shown on top all other labels
            styleBuilder?.setPlacementPriority(10)
            let style = styleBuilder?.buildStyle()
            
            // Check the type of the click
            var clickMsg = ""
            let clickType = mapClickInfo?.getClickType()
            
            if (clickType == NTClickType.CLICK_TYPE_SINGLE) {
                clickMsg = "Single map click!"
            } else if (clickType == NTClickType.CLICK_TYPE_LONG) {
                clickMsg = "Long map click!"
            } else if (clickType == NTClickType.CLICK_TYPE_DOUBLE) {
                clickMsg = "Double map click!"
            } else if (clickType == NTClickType.CLICK_TYPE_DUAL) {
                clickMsg = "Dual map click!"
            }
            
            let wgs84Clickpos = mapView?.getOptions().getBaseProjection()?.toWgs84(clickPos)
            let msg = String(describing: wgs84Clickpos?.getY()) + String(describing: wgs84Clickpos?.getX())
            
            let clickPopup = NTBalloonPopup(pos: clickPos, style: style, title: clickMsg, desc: msg)
            
            vectorDataSource?.add(clickPopup)
            oldClickLabel = clickPopup
        }
    }

    {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--kotlin">
    {% highlight kotlin %}
  
    class MyMapEventListener(var mapView: MapView?, var vectorDataSource: LocalVectorDataSource?) : MapEventListener() {
        var mapView: MapView? = null
        var vectorDataSource: LocalVectorDataSource? = null

        var oldClickLabel: BalloonPopup? = null

        override fun onMapMoved() {

            // This is called every time the map moves, and we can calculate positions as such:

            var topLeft = mapView?.screenToMap(ScreenPos(0F, 0F))

            val width: Float = mapView!!.width.toFloat()
            val height: Float = mapView!!.height.toFloat()

            var bottomRight = mapView?.screenToMap(ScreenPos(width, height))
        }

        override fun onMapClicked(mapClickInfo: MapClickInfo?) {

            // Remove old click label
            if (oldClickLabel != null) {
                vectorDataSource?.remove(oldClickLabel)
                oldClickLabel = null
            }

            val clickPos = mapClickInfo?.clickPos

            val styleBuilder = BalloonPopupStyleBuilder();
            // Make sure this label is shown on top all other labels
            styleBuilder.placementPriority = 10
            val style = styleBuilder.buildStyle()

            // Check the type of the click
            var clickMsg = ""
            val clickType = mapClickInfo?.clickType

            if (clickType == ClickType.CLICK_TYPE_SINGLE) {
                clickMsg = "Single map click!"
            } else if (clickType == ClickType.CLICK_TYPE_LONG) {
                clickMsg = "Long map click!"
            } else if (clickType == ClickType.CLICK_TYPE_DOUBLE) {
                clickMsg = "Double map click!"
            } else if (clickType == ClickType.CLICK_TYPE_DUAL) {
                clickMsg = "Dual map click!"
            }

            val wgs84Clickpos = mapView?.options?.baseProjection?.toWgs84(clickPos)
            val msg = wgs84Clickpos?.y.toString() + wgs84Clickpos?.x

            val clickPopup = BalloonPopup(clickPos, style, clickMsg, msg)

            vectorDataSource?.add(clickPopup)
            oldClickLabel = clickPopup
        }
    }

    {% endhighlight %}
  </div>
    
</div>

`MyMapEventListener` contains special functionality that shows, or hides, a small label text over a clicked object. The following label click event rules apply:

- If you click on an object, it creates another map object as a **BalloonPopup**. This contains the text from the metadata of the clicked object

  For this reason, our samples apps include a _Metadata field value_. You can use the metadata value directly, or use the **object unique ID** as the metadata, to query details about the clicked object from the database.

- When a viewer clicks a new location on the map (or click another map object), the original BalloonPopup is deleted, and new one appears at the click location

- The BalloonPopup is added to the DataSource, which is linked existing map layers. The same DataSource is used for other vector elements on a map


#### Initializing Listener

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
    <li class="Tab js-Tabpanes-navItem--lang">
      <a href="#/3" class="js-Tabpanes-navLink--lang js-Tabpanes-navLink--lang--kotlin">Kotlin</a>
    </li>
  </ul>

  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--java is-active">
    {% highlight java %}
  
    mapView.setMapEventListener(new MyMapEventListener(mapView, source));

    {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--csharp">
    {% highlight c## %}
  
    MapView.MapEventListener = new MyMapListener(MapView, source);

    {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--objective-c">
    {% highlight objc %}
  
    MyMapEventListener* listener = [[MyMapEventListener alloc] init];
    [listener setMapView:mapView vectorDataSource:vectorDataSource];
    [mapView setMapEventListener:listener];

    {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--swift">
    {% highlight swift %}
  
    mapView?.setMapEventListener(MyMapEventListener(mapView: mapView, vectorDataSource: source))

    {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--kotlin">
    {% highlight swift %}

    mapView?.mapEventListener = MyMapEventListener(mapView, source)

    {% endhighlight %}
  </div>
    
</div>
