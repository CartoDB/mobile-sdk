# Ground Overlays

Carto Mobile SDK since 3.1 supports *Ground Overlays* - bitmaps (PNG, JPG etc) which are put to map to your defined location. Most commonly this is used for **indoor floorplans**, but it can be used also for other cases.

<img src = "/images/ground_overlay.png" alt = "Sample bitmap" width="550">

Your bitmap must be have defined **geographical map coordinates**, and this is done using *Ground Control Points*. Currently we support exactly 3 or 4 points defined on bitmap and real world, with following rules:

* With 3 control points you set location, size and rotation of bitmap. This does *linear affine transformation*.
* With 4 control points you can have *perspective transformation*.
* If you have more control points in your data, then you should select 3/4 best ones, and use these.
* Control points must be defined in app code. SDK does not read this automatically from source file metadata. So if you need GeoTIFF, ESRI World File, MapInfo TAB, Geospatial PDF or another already referenced data, then you will get this from GIS Extension (see below).

Key limitation with Carto standard SDK package is that **whole bitmap must fit to device memory** (RAM), so maximum size could be something like 2000x2000 pixels or so, depending on target device. For larger rasters you can use **Carto Mobile SDK GIS Extension** which allows to show any size bitmaps, even hundreds of megabytes, and these are read directly from common GIS raster formats, e.g. GeoTIFF, BSB, ECW, MrSID, JPEG2000 etc, and source data can be in different coordinate systems. Please ask from sales@nutiteq.com if you need this.

Following sample assumes that you have bitmap file **jefferson-building-ground-floor.jpg** in your application project: under *assets* in Android, anywhere in project in iOS. Here we use only one geographical coordinate, and we know size of the building and that it is exactly heading to north, so we can calculate other ground points with the code. Now four ground control points are set to the corners of the bitmap, which gives usually most accurate result. 

<div id="tabs1">
  <ul>
    <li><a href="#i1"><span>iOS ObjC</span></a></li>
    <li><a href="#a1"><span>Android Java</span></a></li>
    <li><a href="#n1"><span>.NET C#</span></a></li>
  </ul>
<div id="i1">
<pre class="brush: objc">
// Load ground overlay bitmap
NTBitmap* overlayBitmap = [[NTBitmap alloc] initWithCompressedData:[NTAssetUtils loadBytes:@"jefferson-building-ground-floor.jpg"] pow2Padding:NO];

// Create two vector containing geographical positions and corresponding raster image pixel coordinates
NTMapPos* pos = [proj fromWgs84:[[NTMapPos alloc] initWithX:-77.004590 y:38.888702]];
double sizeNS = 110, sizeWE = 100;

NTMapPosVector* mapPoses = [[NTMapPosVector alloc] init];
[mapPoses add:[[NTMapPos alloc] initWithX:[pos getX]-sizeWE y:[pos getY]+sizeNS]];
[mapPoses add:[[NTMapPos alloc] initWithX:[pos getX]+sizeWE y:[pos getY]+sizeNS]];
[mapPoses add:[[NTMapPos alloc] initWithX:[pos getX]+sizeWE y:[pos getY]-sizeNS]];
[mapPoses add:[[NTMapPos alloc] initWithX:[pos getX]-sizeWE y:[pos getY]-sizeNS]];

NTScreenPosVector* bitmapPoses = [[NTScreenPosVector alloc] init];
[bitmapPoses add:[[NTScreenPos alloc] initWithX:0 y:0]];
[bitmapPoses add:[[NTScreenPos alloc] initWithX:0 y:[overlayBitmap getOrigHeight]]];
[bitmapPoses add:[[NTScreenPos alloc] initWithX:[overlayBitmap getOrigWidth] y:[overlayBitmap getOrigHeight]]];
[bitmapPoses add:[[NTScreenPos alloc] initWithX:[overlayBitmap getOrigWidth] y:0]];

// Create bitmap overlay raster tile data source
NTBitmapOverlayRasterTileDataSource* rasterDataSource = [[NTBitmapOverlayRasterTileDataSource alloc] initWithMinZoom:0 maxZoom:20 bitmap:overlayBitmap projection:proj mapPoses:mapPoses bitmapPoses:bitmapPoses];
NTRasterTileLayer* rasterLayer = [[NTRasterTileLayer alloc] initWithDataSource:rasterDataSource];
[[self.mapView getLayers] add:rasterLayer];

// Apply zoom level bias to the raster layer.
// By default, bitmaps are upsampled on high-DPI screens.
// We will correct this by applying appropriate bias
float zoomLevelBias = log([[self.mapView getOptions] getDPI] / 160.0f) / log(2);
[rasterLayer setZoomLevelBias:zoomLevelBias * 0.75f];
[rasterLayer setTileSubstitutionPolicy:NT_TILE_SUBSTITUTION_POLICY_VISIBLE];

[self.mapView setFocusPos:pos durationSeconds:0];
[self.mapView setZoom:15.5f durationSeconds:0];
</pre>
</div>
<div id="a1">
<pre class="brush: java">
Projection proj = mapView.getOptions().getBaseProjection();

// Load ground overlay bitmap
com.nutiteq.graphics.Bitmap overlayBitmap = new com.nutiteq.graphics.Bitmap(AssetUtils.loadBytes("jefferson-building-ground-floor.jpg"), false);

// Create two vector containing geographical positions and corresponding raster image pixel coordinates
MapPos pos = proj.fromWgs84(new MapPos(-77.004590, 38.888702));
double sizeNS = 110, sizeWE = 100;

MapPosVector mapPoses = new MapPosVector();
mapPoses.add(new MapPos(pos.getX()-sizeWE, pos.getY()+sizeNS));
mapPoses.add(new MapPos(pos.getX()+sizeWE, pos.getY()+sizeNS));
mapPoses.add(new MapPos(pos.getX()+sizeWE, pos.getY()-sizeNS));
mapPoses.add(new MapPos(pos.getX()-sizeWE, pos.getY()-sizeNS));

ScreenPosVector bitmapPoses = new ScreenPosVector();
bitmapPoses.add(new ScreenPos(0, 0));
bitmapPoses.add(new ScreenPos(0, overlayBitmap.getOrigHeight()));
bitmapPoses.add(new ScreenPos(overlayBitmap.getOrigWidth(), overlayBitmap.getOrigHeight()));
bitmapPoses.add(new ScreenPos(overlayBitmap.getOrigWidth(), 0));

// Create bitmap overlay raster tile data source
BitmapOverlayRasterTileDataSource rasterDataSource = new BitmapOverlayRasterTileDataSource(0, 20, overlayBitmap, proj, mapPoses, bitmapPoses);
RasterTileLayer rasterLayer = new RasterTileLayer(rasterDataSource);
mapView.getLayers().add(rasterLayer);

// Apply zoom level bias to the raster layer.
// By default, bitmaps are upsampled on high-DPI screens.
// We will correct this by applying appropriate bias
float zoomLevelBias = (float) (Math.log(mapView.getOptions().getDPI() / 160.0f) / Math.log(2));
rasterLayer.setZoomLevelBias(zoomLevelBias * 0.75f);
rasterLayer.setTileSubstitutionPolicy(TileSubstitutionPolicy.TILE_SUBSTITUTION_POLICY_VISIBLE);

mapView.setFocusPos(pos, 0);
mapView.setZoom(15.5f, 0);
</pre>
</div>
<div id="n1">
<pre class="brush: csharp">
// Load ground overlay bitmap
var overlayBitmap = new Bitmap (AssetUtils.LoadBytes("jefferson-building-ground-floor.jpg"), false);

// Create two vector containing geographical positions and corresponding raster image pixel coordinates
var pos = proj.FromWgs84(new MapPos(-77.004590, 38.888702));
var sizeNS = 110;
var sizeWE = 100;

var mapPoses = new MapPosVector();
mapPoses.Add(new MapPos(pos.X-sizeWE, pos.Y+sizeNS));
mapPoses.Add(new MapPos(pos.X+sizeWE, pos.Y+sizeNS));
mapPoses.Add(new MapPos(pos.X+sizeWE, pos.Y-sizeNS));
mapPoses.Add(new MapPos(pos.X-sizeWE, pos.Y-sizeNS));

var bitmapPoses = new ScreenPosVector();
bitmapPoses.Add(new ScreenPos(0, 0));
bitmapPoses.Add(new ScreenPos(0, overlayBitmap.OrigHeight));
bitmapPoses.Add(new ScreenPos(overlayBitmap.OrigWidth, overlayBitmap.OrigHeight));
bitmapPoses.Add(new ScreenPos(overlayBitmap.OrigWidth, 0));

// Create bitmap overlay raster tile data source
var rasterDataSource = new BitmapOverlayRasterTileDataSource(0, 20, overlayBitmap, proj, mapPoses, bitmapPoses);
var rasterLayer = new RasterTileLayer(rasterDataSource);
mapView.Layers.Add(rasterLayer);

// Apply zoom level bias to the raster layer.
// By default, bitmaps are upsampled on high-DPI screens.
// We will correct this by applying appropriate bias
float zoomLevelBias = (float) (Math.Log(mapView.Options.DPI / 160.0f) / Math.Log(2));
rasterLayer.ZoomLevelBias = zoomLevelBias * 0.75f;
rasterLayer.TileSubstitutionPolicy = TileSubstitutionPolicy.TileSubstitutionPolicyVisible;

mapView.SetFocusPos(pos, 0);
mapView.SetZoom(15.5f, 0);

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