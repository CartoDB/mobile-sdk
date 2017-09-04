# Custom Offline data

## Types

CartoMobileSDK supports several different types of custom sourcs: 

* mbtiles
* geojson
* Map preloader

In addition to custom sources, your data can be stored on CARTO servers

* tile downloader
* map preloader

### MBTiles

MBTiles is a file format for storing map tiles in a single file. It is, technically, a SQLite database. Latest format version as of 21 October 2011 is 1.1, next version (1.2) is in development. [wiki.openstreetmap.org](http://wiki.openstreetmap.org/wiki/MBTiles)


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

// Copy this method to your class
protected TileDataSource createTileDataSource() {

    // Offline map data source
    String mbTileFile = "rome_ntvt.mbtiles";

    try {
        String localDir = getExternalFilesDir(null).toString();
        copyAssetToSDCard(getAssets(), mbTileFile, localDir);

        String path = localDir + "/" + mbTileFile;
        Log.i(MapApplication.LOG_TAG,"copy done to " + path);
        MBTilesTileDataSource vectorTileDataSource = new MBTilesTileDataSource(0, 19, path);

        return vectorTileDataSource;

    } catch (IOException e) {
        Log.e(MapApplication.LOG_TAG, "mbTileFile cannot be copied: " + mbTileFile);
        Log.e(MapApplication.LOG_TAG, e.getLocalizedMessage());
    }

	return null;
}

// This one as well
public void copyAssetToSDCard(AssetManager assetManager, String fileName, String toDir) throws IOException {

    InputStream in = assetManager.open(fileName);
    File outFile = new File(toDir, fileName);

    // NB! Remember to check if storage is available and has enough space

    if (outFile.exists()) {
        // File already exists, no need to recreate
        return;
    }

    OutputStream out = new FileOutputStream(outFile);
    copyFile(in, out);
    in.close();
    in = null;
    out.flush();
    out.close();
    out = null;
}

// Then initialize your layer as such:
addBaseLayer(CartoBaseMapStyle.CARTO_BASEMAP_STYLE_VOYAGER);

TileDataSource source = createTileDataSource();

// Get decoder from current layer,
// so we wouldn't need a style asset to create a decoder from scratch
MBVectorTileDecoder decoder = (MBVectorTileDecoder)((VectorTileLayer)mapView.getLayers().get(0)).getTileDecoder();

// Remove default baselayer
mapView.getLayers().clear();

// Add our new layer
VectorTileLayer layer = new VectorTileLayer(source, decoder);
mapView.getLayers().insert(0, layer);

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--csharp">
  {% highlight csharp %}
  
// Copy this method to your class
TileDataSource CreateTileDataSource()
{
	// offline map data source
	string fileName = "<bundled-asset-name>";

	try
	{
		string directory = GetExternalFilesDir(null).ToString();
		string path = directory + "/" + fileName;

		Assets.CopyAssetToSDCard(fileName, path);
		Log.Debug("Copy done to " + path);

		MBTilesTileDataSource source = new MBTilesTileDataSource(0, 14, path);

		return new MemoryCacheTileDataSource(source);
	}
	catch (IOException e)
	{
		Log.Debug("MbTileFile cannot be copied: " + fileName);
		Log.Debug("Message" + e.LocalizedMessage);
	}

	return null;
}

// And then you can simply call it as such:
			AddOnlineBaseLayer(CartoBaseMapStyle.CartoBasemapStyleDefault);

// Get decoder from current layer,
// so we wouldn't need a style asset to create a decoder from scratch
MBVectorTileDecoder decoder = (MBVectorTileDecoder)(MapView.Layers[0] as VectorTileLayer).TileDecoder;

// Remove default baselayer
MapView.Layers.Clear();

// Do the actual copying and source creation on another thread so it wouldn't block the main thread
System.Threading.Tasks.Task.Run(delegate
{
	TileDataSource source = CreateTileDataSource();

	var layer = new VectorTileLayer(source, decoder);

	// However, actual layer insertion should be done on the main thread
	RunOnUiThread(delegate
	{
		MapView.Layers.Insert(0, layer);
	});
});
			
  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--objective-c">
  {% highlight objc %}

// Copy this function to your class
- (NTTileDataSource*)createTileDataSource
{
    NSString *name = @"<your-file-name>";
    NSString *extension = @"mbtiles";
    
    // file-based local offline datasource
    NSString* source = [[NSBundle mainBundle] pathForResource:name ofType:extension];
    
    NTTileDataSource* vectorTileDataSource = [[NTMBTilesTileDataSource alloc] initWithMinZoom:0 maxZoom:14 path:source];
    return vectorTileDataSource;
}

// And then call it and create the layer as such:

// Get the base projection set in the base class
NTProjection* projection = [[self.mapView getOptions] getBaseProjection];
NTTileDataSource *source= [self createTileDataSource];

NTCartoOnlineVectorTileLayer *baseLayer = [[NTCartoOnlineVectorTileLayer alloc]initWithStyle:NT_CARTO_BASEMAP_STYLE_VOYAGER];
NTVectorTileDecoder *decoder = [baseLayer getTileDecoder];
    
NTVectorTileLayer *layer = [[NTVectorTileLayer alloc]initWithDataSource:source decoder:decoder];
[[self.mapView getLayers] add:layer];
    
  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--swift">
  {% highlight swift %}
  
let baseLayer = NTCartoOnlineVectorTileLayer(style: .CARTO_BASEMAP_STYLE_VOYAGER);
let decoder = baseLayer?.getTileDecoder()
    
// Do the actual copying and source creation on another thread so it wouldn't block the main thread
DispatchQueue.global().async {
    
    let path = Bundle.main.path(forResource: "<your-file-name", ofType: "mbtiles")
    let source = NTMBTilesTileDataSource(minZoom: 0, maxZoom: 14, path: path)
    let layer = NTVectorTileLayer(dataSource: source, decoder: decoder)
    
    DispatchQueue.main.async {
        // But add the layer on the main thread
        self.contentView.map.getLayers().add(layer)
    }
}

  {% endhighlight %}
  </div>

  <div class="Carousel-item js-Tabpanes-item--lang js-Tabpanes-item--lang--kotlin">
  {% highlight kotlin %}
  
val baseLayer = CartoOnlineVectorTileLayer(CartoBaseMapStyle.CARTO_BASEMAP_STYLE_VOYAGER)
val decoder = baseLayer.tileDecoder

// Do the actual copying and source creation on another thread so it wouldn't block the main thread
// This requires the anko coroutines library:
// compile "org.jetbrains.anko:anko-sdk25-coroutines:$anko_version"
doAsync {

    val filename = "<your-file-name>.mbtiles"
    val directory = getExternalFilesDir(null).toString()
    val path = directory + "/" + filename

    // Copy assets to SD card
    val input = assets.open(filename)
    val outFile = File(filename, directory)

    if (!outFile.exists()) {
        val output = FileOutputStream(outFile)
        input.copyTo(output)
        input.close()
        output.close()
    }

    val source = MBTilesTileDataSource(0, 14, path)
    val layer = VectorTileLayer(source, decoder)
    
    runOnUiThread {
        contentView!!.map.layers.add(layer)
    }
}

  {% endhighlight %}
  </div>
</div>

### GeoJson

