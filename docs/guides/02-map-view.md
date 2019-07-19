## Map View 

CARTO SDK **MapView** is the central map object to be added to your application screen - Storyboard or Layout. Technically it is platform-specific, so some details could be different on different platforms, but in general it provides same features cross-platform.

Following figure provides summary of MapView most used properties, methods and relationships with other SDK classes:

![SDK overview](../../img/mobile-api-overview.png)

### MapView manipulation

First step with MapView is always RegisterLicense, this is needed to connect to our map servers. Typical next step is to add your Layers, which are shown in same order as they are added. You can change layer order later, make layers invisible also. Typically you have basemap layer, such as `CartoOnlineVectorTileLayer`. Brand new map view is empty, with no map data.

MapView has direct methods to zoom, pan, rotate and tilt map. Also you can request coordinates of the map view etc. 

### MapView options

CARTO SDK MapView has various configuration options and settings, some of them are generic enough to be useful for almost any mobile app. SDK overview above lists some of them:

* Parameters to limit interaction: zooming, panning rotating, tilting, touches
* Finetune performance - number of threads/workers to load data. Set bigger for multi-core devices
* Set custom map and sky background patterns and colors
* etc

### Listening to Events

SDK contains various **Listener** hooks that are used to receive various events from the SDK. 

There are listeners for user interaction related events, rendering events and map download events among others:
* `MapEventListener` - map touches and movements - attach it to your `MapView`. See below for details
* `VectorElementEventListener` - listen Vector Element touches/clicks, attach it to the related `VectorLayer`.

#### Implementing MapEventListener

An app can implement a custom `MapEventListener` to receive notifications about various events, such as:

* Map is being moved or zoomed (`onMapMoved`)
* Map has reached 'stable state', all tiles and data has been loaded displayed (`onMapStable`)
* Map has been clicked (`onMapClicked`)


Create a new class called **MyMapEventListener** which implements MapEventListner interface.


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
<div id="tab-csharp">
  {% highlight csharp linenos %}
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
<div id="tab-objectivec">
    {% highlight objc linenos %}
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
<div id="tab-swift">
    {% highlight swift linenos %}
  
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
<div id="tab-kotlin">
    {% highlight kotlin linenos %}
  
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

- If you click on an object, it creates another map object as a **BalloonPopup**. This contains the text from the metadata of the clicked object. For this reason, our samples apps include a _Metadata field value_. You can use the metadata value directly, or use the **object unique ID** as the metadata, to query details about the clicked object from the database.
- When a viewer clicks a new location on the map (or click another map object), the original BalloonPopup is deleted, and new one appears at the click location
- The BalloonPopup is added to the DataSource, which is linked existing map layers. The same DataSource is used for other vector elements on a map


#### Initializing Listener

Apply the following code to initialize listener events for map clicks.

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
  
    mapView.setMapEventListener(new MyMapEventListener(mapView, source));

    {% endhighlight %}
</div>
<div id="tab-csharp">
    {% highlight csharp linenos %}
  
    MapView.MapEventListener = new MyMapListener(MapView, source);

    {% endhighlight %}
</div>
<div id="tab-objectivec">
    {% highlight objc linenos %}
  
    MyMapEventListener* listener = [[MyMapEventListener alloc] init];
    [listener setMapView:mapView vectorDataSource:vectorDataSource];
    [mapView setMapEventListener:listener];

    {% endhighlight %}
</div>
<div id="tab-swift">
    {% highlight swift linenos %}
  
    mapView?.setMapEventListener(MyMapEventListener(mapView: mapView, vectorDataSource: source))

    {% endhighlight %}
</div>

<div id="tab-kotlin">
    {% highlight swift linenos %}

    mapView?.mapEventListener = MyMapEventListener(mapView, source)

    {% endhighlight %}
  </div>
</div>
