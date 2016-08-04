# Clusters

In many cases you may have so many Points of Interest (POIs) that these cannot be shown on map as individual markers - map view would be cluttered and eventually also slow or even run out of memory. One of the methods to resolve the issue would be to replace individual objects with *clusters* - Markers which show location of several objects. Often marker clusters shows number of objects which each marker includes.

![Cluster sample](http://share.gifyoutube.com/vMPDzX.gif)

Carto Mobile SDK has built-in cluster feature, which is highly customizable. You can define in you app code:

* style for cluster objects
* generate cluster object style dynamically, so it has e.g. number of objects in it
* define minimum zoom level for clusters
* set minimum distance between objects before it becomes cluster
* decide action for clicking on marker - e.g. zoom in, or expand cluster right away (without zoom). The latter works for small clusters (up to 4 objects inside)


Clusters are generated dynamically, based on VectorDataSource data, during loading of the map view with the layer. From API point of view it works as a special Layer: *ClusteredVectorLayer*, and this creation of it you give two parameters:

1. **DataSource** to be used. In most cases it would be *LocalVectorDataSource* which has already all the elements. It is important the the DataSource gives all elements of layer, not limiting it to current map view bounding box.
2. **ClusterElementBuilder** implementation, which defines single method **buildClusterElement**.

## 1. Define data and layer
<div id="tabs1">
  <ul>
    <li><a href="#i1"><span>iOS ObjC</span></a></li>
    <li><a href="#a1"><span>Android Java</span></a></li>
    <li><a href="#n1"><span>.NET C#</span></a></li>
  </ul>
<div id="i1">
<pre class="brush: objc">
// Initialize a local vector data source
NTProjection* proj = [[self.mapView getOptions] getBaseProjection];
NTLocalVectorDataSource* vectorDataSource = [[NTLocalVectorDataSource alloc] initWithProjection:proj];

// Now create Marker objects and add them to vectorDataSource.
// TODO: this depends on your app! See AdvancedMap for samples with JSON loading and random point generation

// Create element builder
MyMarkerClusterElementBuilder* clusterElementBuilder = [[MyMarkerClusterElementBuilder alloc] init];

// Initialize a vector layer with the previous data source
NTClusteredVectorLayer* vectorLayer = [[NTClusteredVectorLayer alloc] initWithDataSource:vectorDataSource clusterElementBuilder:clusterElementBuilder];

// Add the previous vector layer to the map
[[self.mapView getLayers] add:vectorLayer];

</pre>
</div>
<div id="a1">
<pre class="brush: java">

// Initialize a local vector data source
LocalVectorDataSource vectorDataSource1 = new LocalVectorDataSource(baseProjection);

// Now create Marker objects and add them to vectorDataSource.
// TODO: this depends on your app! See AdvancedMap for samples with JSON loading and random point generation

// Initialize a vector layer with the previous data source
ClusteredVectorLayer vectorLayer1 = new ClusteredVectorLayer(vectorDataSource1, new MyClusterElementBuilder(this.getApplication()));
vectorLayer1.setMinimumClusterDistance(20);

// Add the previous vector layer to the map
mapView.getLayers().add(vectorLayer1);

</pre>
</div>
<div id="n1">
<pre class="brush: csharp">

var proj = new EPSG3857 ();

// Create overlay layer for markers
var dataSource = new LocalVectorDataSource (proj);

// Now create Marker objects and add them to vectorDataSource.
// TODO: this depends on your app! See samples with JSON loading

var overlayLayer = new ClusteredVectorLayer (dataSource, new MyClusterElementBuilder());
overlayLayer.MinimumClusterDistance = 20; // in pixels
mapView.Layers.Add (overlayLayer);

</pre>
</div>
</div>




## 2. Define ClusterElementBuilder

Essentially Cluster Element Builder takes set of original markers (map objects) as input, and returns one Marker (or another VectorElement, like Point or BalloonPopup) which is dynamically replacing the original ones.

What makes it more complicated is that in the ClusterElementBuilder we strongly suggest to reuse (cache) styles to reduce memory usage significantly. So marker style with specific number is only created once. Android and iOS samples use platform-specific graphics APIs to generate bitmap for the marker. .NET example just uses BalloonPopup, which is slower but works same across platforms.

<div id="tabs2">
  <ul>
    <li><a href="#i2"><span>iOS ObjC</span></a></li>
    <li><a href="#a2"><span>Android Java</span></a></li>
    <li><a href="#n2"><span>.NET C#</span></a></li>
  </ul>
<div id="i2">
<pre class="brush: objc">
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
        //[markerStyleBuilder setHideIfOverlapped:NO];
        [markerStyleBuilder setPlacementPriority:-(int)[elements size]];
        markerStyle = [markerStyleBuilder buildStyle];
        [self.markerStyles setValue:markerStyle forKey:styleKey];
    }
    
    NTMarker* marker = [[NTMarker alloc] initWithPos:mapPos style:markerStyle];
    [marker setMetaDataElement:@"elements" element:[@([elements size]) stringValue]];
    return marker;
}

@end


</pre>
</div>
<div id="a2">
<pre class="brush: java">

private static class MyClusterElementBuilder extends ClusterElementBuilder {
		@SuppressLint("UseSparseArrays")
		private Map < Integer, MarkerStyle >  markerStyles = new HashMap < Integer, MarkerStyle > ();
		private android.graphics.Bitmap markerBitmap;
		
		MyClusterElementBuilder(Application context) {
			markerBitmap = android.graphics.Bitmap.createBitmap(BitmapFactory.decodeResource(context.getResources(), R.drawable.marker_black));
		}

		@Override
		public VectorElement buildClusterElement(MapPos pos, VectorElementVector elements) {
			// Try to reuse existing marker styles
			MarkerStyle style = markerStyles.get((int) elements.size());
			if (elements.size() == 1) {
				style = ((Marker) elements.get(0)).getStyle();
			}
			if (style == null) {
				android.graphics.Bitmap canvasBitmap = markerBitmap.copy(android.graphics.Bitmap.Config.ARGB_8888, true);
				android.graphics.Canvas canvas = new android.graphics.Canvas(canvasBitmap); 
				android.graphics.Paint paint = new android.graphics.Paint(android.graphics.Paint.ANTI_ALIAS_FLAG);
				paint.setTextAlign(Align.CENTER);
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

			// Create marker for the cluster
			Marker marker = new Marker(pos, style);
			return marker;
		}
	}
    

</pre>
</div>
<div id="n2">
<pre class="brush: csharp">

class MyClusterElementBuilder : ClusterElementBuilder
	{
		BalloonPopupStyleBuilder balloonPopupStyleBuilder;

		public MyClusterElementBuilder() {
			balloonPopupStyleBuilder = new BalloonPopupStyleBuilder();
			balloonPopupStyleBuilder.CornerRadius = 3;
			balloonPopupStyleBuilder.TitleMargins = new BalloonPopupMargins (6, 6, 6, 6);
			balloonPopupStyleBuilder.LeftColor = new Color(240,230,140,255);
		}

		public override VectorElement BuildClusterElement(MapPos pos, VectorElementVector elements) {
			var popup = new BalloonPopup(
				pos,
				balloonPopupStyleBuilder.BuildStyle(),
				elements.Count.ToString(), "");
			return popup;
		}

	}

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