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

<div id="tabs1">
  <ul>
    <li><a href="#i1"><span>iOS ObjC</span></a></li>
    <li><a href="#a1"><span>Android Java</span></a></li>
    <li><a href="#n1"><span>.NET C#</span></a></li>
  </ul>
<div id="i1">
<pre class="brush: objc">

// MyMapEventListener.h:

#import &lt;Nuti/Nuti.h&gt;

@interface  MyMapEventListener : NTMapEventListener
-(void)onMapMoved;
-(void)onMapClicked:(NTMapClickInfo*)mapClickInfo;
-(void)onVectorElementClicked:(NTVectorElementsClickInfo*)vectorElementsClickInfo;
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
</pre>
</div>
<div id="a1">
<pre class="brush: java">
// MyMapEventListener.java : 
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

</pre>
</div>
<div id="n1">
<pre class="brush: csharp">
// MapListener.cs :
// using statements skipped

namespace HelloMap
{

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
}

</pre>
</div>
</div>

### 2. Initialize listener

<div id="tabs2">
  <ul>
    <li><a href="#i2"><span>iOS ObjC</span></a></li>
    <li><a href="#a2"><span>Android Java</span></a></li>
    <li><a href="#n2"><span>.NET C#</span></a></li>
  </ul>
<div id="i2">
<pre class="brush: objc">
// 1. Create a map event listener
MyMapEventListener* mapListener = [[MyMapEventListener alloc] init];
[self setMapEventListener:mapListener];
	
</pre>
</div>
<div id="a2">
<pre class="brush: java">
 mapView.setMapEventListener(new MyMapEventListener(mapView, vectorDataSource));

</pre>
</div>
<div id="n2">
<pre class="brush: csharp">
 // Create and set map listener
 mapView.MapEventListener = new MapListener (dataSource);

</pre>
</div>
</div>

<script>
		$( "#tabs1" ).tabs();
		$( "#tabs2" ).tabs();
		$( "#tabs3" ).tabs();
		$( "#tabs4" ).tabs();
  		$( "#tabs5" ).tabs();
</script>