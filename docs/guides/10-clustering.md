## Clustering

CARTO Mobile SDK can dynamically cluster point data if large amounts of points need to be shown without cluttering the MapView. Clusters are formed based on the map zoom level and spatially close points are placed into the same cluster.

Clusters are usually markers which display a location of several objects, and typically indicate the number of markers within each object.

![Cluster sample](http://share.gifyoutube.com/vMPDzX.gif)

CARTO Mobile SDK has built-in cluster feature, which is highly customizable. You can define the following options in your app code:

- Styling the cluster objects
- Dynamically generate cluster object styles. For example, automatically display the number of objects in each cluster
- Define the minimum zoom level for clusters
- Set the minimum distance between objects, before it becomes a cluster
- Indicate the action when clicking on marker. For example, zoom in, or expand the cluster without zooming

**Tip:** The cluster expand feature is useful for small clusters (containing up to four objects inside)

Depending on the device, the Mobile SDK can cluster 100,000 points in less than a second.


### Implementing clustering

Clusters are generated dynamically, based on `VectorDataSource` data that loads the map layer. If using an API, it works as a unique layer with the `ClusteredVectorLayer` method, and includes the following parameters in the a hierarchal order:

1. Select the layer `DataSource`  In most cases,  the `LocalVectorDataSource` function contains all the elements to request the data. It is important that the DataSource displays all elements in a layer, and does not limit it to the current map visualization bbox (bounding box)
2. `ClusterElementBuilder` defines a single method `buildClusterElement`


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
    {% highlight java %}

    // 1. Initialize a local vector data source
    LocalVectorDataSource vectorDataSource1 = new LocalVectorDataSource(mapView.getOptions().getBaseProjection());

    // 2. Create Marker objects and add them to vectorDataSource
    // **Note:** This depends on the _app type_ of your mobile app settings. See AdvancedMap for samples with JSON loading and random point generation

    // 3. Initialize a vector layer with the previous data source
    ClusteredVectorLayer vectorLayer1 = new ClusteredVectorLayer(vectorDataSource1, new MyClusterElementBuilder(this.getApplication()));
    vectorLayer1.setMinimumClusterDistance(20);

    // 4. Add the previous vector layer to the map
    mapView.getLayers().add(vectorLayer1);

    {% endhighlight %}
  </div>

  <div id="tab-csharp">
    {% highlight csharp %}

    // 1. Create overlay layer for markers
    var dataSource = new LocalVectorDataSource(MapView.Options.BaseProjection);

    // 2. Create Marker objects and add them to vectorDataSource.
    // **Note:** This depends on the _app type_ of your mobile app settings. See samples with JSON loading

    // 3. Initialize a vector layer with the previous data source
    var layer = new ClusteredVectorLayer(dataSource, new MyClusterElementBuilder());
    layer.MinimumClusterDistance = 20; // in pixels

    MapView.Layers.Add(layer);

    {% endhighlight %}
  </div>

  <div id="tab-objectivec">
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

    [[mapView getLayers] add:vectorLayer];

    {% endhighlight %}
  </div>

  <div id="tab-swift">
    {% highlight swift %}
  
    // 1. Initialize a local vector data source
    let vectorDataSource1 = NTLocalVectorDataSource(projection: mapView?.getOptions().getBaseProjection())

    // 2. Create Marker objects and add them to vectorDataSource
    // **Note:** This depends on the _app type_ of your mobile app settings.
    // See AdvancedMap for samples with JSON loading and random point generation

    // 3. Initialize a vector layer with the previous data source
    let builder = MyClusterElementBuilder(imageUrl: "marker_black.png")
    let vectorLayer1 = NTClusteredVectorLayer(dataSource: vectorDataSource1, clusterElementBuilder: builder)
    vectorLayer1?.setMinimumClusterDistance(20)

    // 4. Add the previous vector layer to the map
    mapView?.getLayers()?.add(vectorLayer1)

    {% endhighlight %}
  </div>

  <div id="tab-kotlin">
    {% highlight kotlin %}

    // 1. Initialize a local vector data source
    val vectorDataSource1 = LocalVectorDataSource(mapView?.options?.baseProjection)

    // 2. Create Marker objects and add them to vectorDataSource
    // **Note:** This depends on the _app type_ of your mobile app settings.
    // See AdvancedMap for samples with JSON loading and random point generation

    // 3. Initialize a vector layer with the previous data source
    val vectorLayer1 = ClusteredVectorLayer(vectorDataSource1, MyClusterElementBuilder(this.application))
    vectorLayer1.minimumClusterDistance = 20f

    // 4. Add the previous vector layer to the map
    mapView?.layers?.add(vectorLayer1)

    {% endhighlight %}
  </div>
    
</div>

#### Define ClusterElementBuilder

The Cluster Element Builder takes set of original markers (map objects) as input, and returns one object (or another `VectorElement`, such as a Point or BalloonPopup) which dynamically replaces the original marker.

**Note:** It is highly recommended to reuse and cache styles to reduce memory usage. For example, a marker style with a specific number is only created once. Android and iOS samples use platform-specific graphic APIs to generate the bitmap for the marker. .NET example only uses BalloonPopup, which is slower but works the same across all platforms.

<div class="js-tabs-mobilesdk">
  <ul class="tab-navigation">
    <li class="tab-navigationItem">
      <a href="#/0">Java</a>
    </li>
    <li class="tab-navigationItem">
      <a href="#/1">C#</a>
    </li>
    <li class="tab-navigationItem">
      <a href="#/2">Objective-C</a>
    </li>
    <li class="tab-navigationItem">
      <a href="#/3">Swift</a>
    </li>
  </ul>

  <div id="tab-java">
    {% highlight java %}
  
    private class MyClusterElementBuilder extends ClusterElementBuilder {

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

  <div id="tab-csharp">
    {% highlight csharp %}
    public class MyClusterElementBuilder : ClusterElementBuilder
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

  <div id="tab-objectivec">
    {% highlight objc %}

    // .h

    @interface MyMarkerClusterElementBuilder : NTClusterElementBuilder

    @property NSMutableDictionary* markerStyles;

    @end
    
    // .m

    @implementation MyMarkerClusterElementBuilder

    -(NTVectorElement*) buildClusterElement:(NTMapPos*)mapPos elements:(NTVectorElementVector*)elements
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
           
            NSMutableParagraphStyle* style = [[NSParagraphStyle defaultParagraphStyle] mutableCopy];
            [style setAlignment:NSTextAlignmentCenter];
            
            NSDictionary* attr = [NSDictionary dictionaryWithObject:style forKey:NSParagraphStyleAttributeName];
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

  <div id="tab-swift">
    {% highlight swift %}
  
    public class MyClusterElementBuilder : NTClusterElementBuilder {
        
        let markerStyles = NSMutableDictionary()
        
        var imageUrl: String?
        
        convenience init(imageUrl: String) {
            
            self.init()
            self.imageUrl = imageUrl
        }
        
        override public func buildClusterElement(_ mapPos: NTMapPos!, elements: NTVectorElementVector!) -> NTVectorElement! {
            
            var styleKey = String(elements.size())
            
            if (elements.size() > 1000) {
                styleKey = ">1K"
            }
            
            var markerStyle = self.markerStyles.value(forKeyPath: styleKey)
            
            if (elements.size() == 1) {
                markerStyle = (elements.get(0) as! NTMarker).getStyle()
            }
            
            if (markerStyle == nil) {
                
                let image = UIImage(named: imageUrl!)
                UIGraphicsBeginImageContext((image?.size)!)
                
                image?.draw(at: CGPoint(x: 0, y: 0));
                
                let rect = CGRect(x: 0, y: 15, width: (image?.size.width)!, height: (image?.size.height)!)
                UIColor.black.set()

                image?.draw(in: rect.integral, blendMode: CGBlendMode.color, alpha: 1.0)
                let newImage = UIGraphicsGetImageFromCurrentImageContext()
                
                UIGraphicsEndImageContext()
                
                let marker = NTBitmapUtils.createBitmap(from: newImage)
                let builder = NTMarkerStyleBuilder()
                builder?.setBitmap(marker)
                builder?.setSize(30)
                
                builder?.setPlacementPriority(-(Int32(elements.size() as UInt32)))
                markerStyle = builder?.buildStyle()
                
                self.markerStyles.setValue(markerStyle, forKey: styleKey)
            }
            
            let marker = NTMarker(pos: mapPos, style: markerStyle as! NTMarkerStyle!)
            
            let variant = NTVariant(string: String(elements.size()))
            marker?.setMetaData("elements", element: variant)
            
            return marker
        }
    }

    {% endhighlight %}
  </div>

  <div id="tab-kotlin">
    {% highlight kotlin %}
  
    private inner class MyClusterElementBuilder internal constructor(context: Context) : ClusterElementBuilder() {

        val markerStyles = HashMap<Int, MarkerStyle>()
        val markerBitmap: android.graphics.Bitmap

        init {
            val resource = BitmapFactory.decodeResource(context.resources, R.drawable.marker_black)
            markerBitmap = android.graphics.Bitmap.createBitmap(resource)
        }

        override fun buildClusterElement(pos: MapPos, elements: VectorElementVector): VectorElement {

            // 1. Reuse existing marker styles
            var style: MarkerStyle? = markerStyles[elements.size().toInt()]

            if (elements.size().toInt() == 1) {
                style = (elements.get(0) as Marker).style
            }

            if (style == null) {
                val canvasBitmap = markerBitmap.copy(android.graphics.Bitmap.Config.ARGB_8888, true)
                val canvas = android.graphics.Canvas(canvasBitmap)
                val paint = android.graphics.Paint(android.graphics.Paint.ANTI_ALIAS_FLAG)

                paint.textAlign = Paint.Align.CENTER
                paint.textSize = 12f
                paint.color = android.graphics.Color.argb(255, 0, 0, 0)

                val text = Integer.toString(elements.size().toInt())
                val x = (markerBitmap.width / 2).toFloat()
                val y = (markerBitmap.height / 2 - 5).toFloat()

                canvas.drawText(text, x, y, paint)

                val styleBuilder = MarkerStyleBuilder()
                styleBuilder.bitmap = BitmapUtils.createBitmapFromAndroidBitmap(canvasBitmap)
                styleBuilder.size = 30f
                styleBuilder.placementPriority = (-elements.size()).toInt()

                style = styleBuilder.buildStyle()

                markerStyles.put(elements.size().toInt(), style)
            }

            // 2. Create marker for the cluster
            val marker = Marker(pos, style)
            return marker
        }
    }

    {% endhighlight %}
  </div>
    
</div>
