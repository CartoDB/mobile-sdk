CARTO Mobile SDK 4.1.4
-------------------

This update includes performance and stability improvements,
bug fixes and some minor new features.

### New features:

* Exposed TileUtils class with several static methods as part of public API
* SDK now supports custom service URLs as online source ids


### Fixes/changes:

* Fixed Android HTTP connection class to use specified request method (previously always GET)
* Fixed JNI local reference overflows in Android HTTP connection class (with HTTP servers returning very long lists of headers).
* Removed unneeded iOS dependency of libstdc++.6 in Cocoapod, fixes build issues with iOS 12
* Fixed the issue with delayed layer initialization, layers were not automatically rendered
* Fixed several options not correctly reflected in renderer state when changed after the MapView was initialized
* Fixed infinite loop in TileLayer update method when called with inconsistent state (zero view dimensions)
* Fixed value clamping issue with Torque tiles (all floating point numbers were rounded to integers)
* Optimized CartoCSS compiler with 10% reduced map initialization time and faster tile loading time
* Better error reporting of CartoCSS issues
* SDK now uses default background bitmap in case of vector basemap with no background defined
* Bitmap class decoder now supports automatic ungzipping. This is a fix for wrongly configured HTTP servers that send gzipped images even when this is not included in accepted encodings.
* Fixed CartoNamedMapsService ignoring template parameter values when instantiating named maps
* Fixed several grouped marker symbolizers being represented by a single marker
* Fixed threading issue with online license management causing potential API token missing from initial HTTP requests
* Fixed WebP library embedding on iOS targets (Xamarin/native), WebP symbols were previously exported, causing potential linking conflicts
* Made Xamarin.iOS build compatible with 'Linker behaviour = Link All' mode by explictly preserving symbols used through reflection


CARTO Mobile SDK 4.1.3
-------------------

This update includes performance and stability improvements,
bug fixes and some minor new features.

### New features:

* Added support for TomTom online geocoding services (TomTomOnlineGeocodingService and TomTomOnlineReverseGeocodingService)
* Implemented multilanguage support for offline geocoding classes (getLanguage, setLanguage methods in OSMOfflineGeocodingService and PackageManagerGeocodingService classes)
* Implemented localization support for Pelias geocoding results (getLanguage, setLanguage methods in PeliasOnlineGeocodingService)
* Implemented proper location bias for all geocoding services, 'location radius' is no longer needed for bias to work
* Implemented opacity attribute for layers (setOpacity, getOpacity). Note that when used (opacity < 1.0), then this feature may have significant performance impact.
* Implemented background color and border support for Text vectorelements (TextStyleBuilder class)
* Implemented ‘break lines’ flag for texts (TextStyleBuilder class)
* Added online API key interface to CartoMapsService and CartoSQLService
* Added NTExceptionWrapper class for catching/handling SDK exceptions in Swift


### Fixes/changes:

* Min API level on Android is now 10 for Xamarin
* Performance fix for CARTO Maps API - use cacheable requests when instantiating named and anonymous maps
* Fixed regression in SDK 4.1.x vs 4.0.x - packages with incomplete zoom levels had wrong tilemasks after serialized/deserialized in database
* Fixed bounds calculation for NML models
* Fixed zoom level handling in ‘restricted panning’ mode
* Fixed ‘restricted panning’ mode when tilt is applied
* Fixed tile cache invalidation issue when all packages are removed from PackageManager
* BalloonPopupStyleMargins class getters were not wrapped as properties for dotnet platforms previously, fixed now
* Optimized label handling in VT renderer for zoom levels > 14
* Optimized 3D buildings and transparent layers in VT renderer on GPUs that use tiled rendering
* Distance based filtering in search API is more robust now (for coordinate wrapping, etc)
* Fixed WKTGeometryWriter to NOT use scientific encoding


CARTO Mobile SDK 4.1.2
-------------------

This is a maintenance release for SDK 4.1.x containing mostly fixes
but also some new features. This version deprecates support
for external MapZen services due to the services being closed.

### New features:

* SDK has support for MapBox online geocoding services.
  New classes MapBoxOnlineGeocodingService and MapBoxOnlineReverseGeocodingService can be used for this.
* All MapZen online service (Pelias and Valhalla) wrappers now include additional methods for specifying custom service URLs.
  This feature was added as MapZen closes all online services as of February 2018.
* Added optional ‘restricted panning’ support to avoid zooming/panning outside world map area. If turned on, then  map area is restricted to maximize visible map. This can be turned on/off using Options.setRestrictedPanning method
* Added custom service URL support for Pelias and Mapbox geocoders and Valhalla routing
* API documentation for iOS is using Jazzy tool, instead of Doxygen. This allows us to show both ObjectiveC and Swift syntax for the API.


### Fixes/changes:

* Implemented fine-grained clipping in VT loader - reduces drawing of invisible geometry and improves performance 
* Removed MapZen-specific handling from CartoOnlineTileDataSource
* Smaller built-in style asset due to optimized fonts
* Proper handling of line-placement of markers and texts with polygon geometry
* Fixed C#-specific API wrapping issue: Polygon3DStyleBuilder and Polygon3DStyle SideColor property was not properly wrapped
* SDK includes latest version of CARTO styles, with minor fixes
* Improved text placement along lines in vector tile renderer
* Fixed text wrapping in vector tile renderer when ‘wrap-before: true’ mode was used
* MapZen-specific code is removed from CartoOnlineVectorLayer
* Minor optimizations in vector tile renderer for faster rendering of transparent features


CARTO Mobile SDK 4.1.1
-------------------

This is a maintenance release for SDK 4.1.x containing mostly fixes
but also some new features.

### New features:
* Implemented route matching support in ValhallaOfflineRoutingService and PackageManagerValhallaRoutingService classes
* Included NMLModelLODTree in the build (missing from all previous 4.x builds)
* Added postcode to geocoding responses
* Implemented building-min-height parameter for CartoCSS
* Improved support for offline Valhalla routing with multimodal profile


### Fixes/changes:
* Improved text placement in vector tile renderer with texts that have non-zero vertical offsets
* Improved tilting gesture handling on UWP
* Performance optimizations for MB vector tile decoder
* Pelias Online geocoding fixes
* Text rendering quality improvements
* Improvement of Mapnik XML styling reader
* Fixed building height issue with built-in basemaps when 3d buildings are enabled
* Fixed vector tile layer elements missing at zoom level 24
* Fixed http:// and https:// handling when accessing CartoCSS external resources
* Fixed subtle background rendering issues on iOS (PowerVR) due to insufficient precision in fragment shaders
* Fixed UWP specific issue - do not try to create EGL context when panel size is 0
* Fixed custom HTTP headers being ignored when using HTTPTileDataSource
* Fixed basemap 3D building height calculation
* Fixed z-fighting/flickering issue with overlapping basemap 3D buildings
* Fixed minor rendering issues with NMLModelLODTreeLayer
* Fixed a small memory leak with vector layers containing NMLModels
* Documentation fixes


CARTO Mobile SDK 4.1.0
-------------------

This is a major release containing many new features, fixes and performance
optimizations.

### Key highlights:
* SDK now supports **geocoding** and **reverse geocoding**. For offline geocoding, custom geocoding packages can be used through PackageManager. We have provided country-based packages (bigger countries like US, Germany have split packages) but custom packages based on bounding box can be also used. For online geocoding, SDK includes wrapper class for MapZen Pelias geocoder; your MapZen API key is required for that.
* SDK has optional support for **MapZen Valhalla routing**. This feature requires a special SDK build as the routing engine is fairly complex and makes compiled SDK binaries approximately 30% larger. Compared to the custom built-in routing Valhalla routing packages are univeral -  single package can be used for car, bicycle or walking profiles. We have prepared country-based packages that can be downloaded  using PackageManager. Also, custom packages based on bounding box are supported. For online Valhalla routing, SDK includes wrapper class that uses MapZen Mobility API.
* New **built-in styles** and vector tile structure. This change is backward-incompatible due to two reasons: the old styles are removed from the SDK and new styles require different tile and offline package sources. New styles are better optimized for lower-end devices and have more consistent information density on all zoom levels. Also, new styles are based on view-dependent zoom parameters instead of tile-based zoom parameters, which gives much more pleasant zooming experience and cleaner visuals at fractional zoom levels.
* SDK supports **offline searching** features from various sources (VectorTileDataSource, FeatureCollection, VectorDataSource) via unified search API. The search API supports search requests based on geometry and distance, metadata and custom SQL-like query language.
* The VectorElements appearing on the map can now have **transitioning animations**. This is currently supported for billboards only (markers, texts, popups). Different animations styles are supported and the effects can be customized.
* SDK 4.1 has major **speed and memory usage improvements** when using ClusteredVectorLayer class. Performance can be up to 10x better compared to SDK 4.0.x and memory usage 2x lower.
* Lots of lower level performance and memory usage optimizations, mostly related to vector tiles.

### API changes:
* The new built-in styles (Voyager, Positron, Darkmatter) use different data schema and are not compatible with *nutiteq.osm* source. Instead, "**carto.streets**"  source must be used. This applies to both online tiles and offline map packages. The old styles (Dark, Grey, Nutibright) and data source continue to work for now, but are no longer included in the SDK and must be downloaded/applied separately. Offline map packages are not updated for nutiteq.osm source.
* The old nutibright, dark and grey styles are no longer included in the SDK and as a result the following CartoBaseMapStyles are removed:  CARTO_BASEMAP_STYLE_DEFAULT, CARTO_BASEMAP_STYLE_GREY, CARTO_BASEMAP_STYLE_DARK. Instead, new styles CARTO_BASEMAP_STYLE_VOYAGER,  CARTO_BASEMAP_STYLE_POSITRON, CARTO_BASEMAP_STYLE_DARKMATTER should be used.
* Public constructors from various vector element Style classes are now hidden, these classes can now be instanced only through corresponding StyleBuilders.
* Removed unsafe clone method from StyleBuilder.
* Removed public constructors for internal 'UI info' classes.
* Removed public constructors for Frustum class
* New CartoStyles package with following changes:
  1) default language is now "en" (before "local")
  2) 'buildings3d' style parameter is no longer used, instead 'buildings' style parameter can be used to control rendering of buildings (0=no buildings, 1=2D buildings, 2=3D buildings)
* Tilemasks used by the offline packages have stricter semantics now and PACKAGE_TILE_STATUS_PARTIAL tile status  is now deprecated (never used by the SDK) and will be removed in the later versions.


### Detailed list of new features:
* New 'geocoding' module that includes following generic classes/interfaces: GeocodingRequest, GeocodingResult, GeocodingService, ReverseGeocodingRequest, ReverseGeocodingService. The module also includes several classes for offline geocoding/reverse geocoding: OSMOfflineGeocodingService, OSMOfflineReverseGeocodingService, PackageManagerGeocodingService, PackageManagerReverseGeocodingService.  For online geocoding the module includes PeliasGeocodingService and PeliasReverseGeocodingService classes.
* The routing module includes three new classes for Valhalla routing: PackageManagerValhallaRoutingService, ValhallaOfflineRoutingService, ValhallaOnlineRoutingService.  These classes are only included in Valhalla-supporting builds.
* New 'search' module for searching features from various sources. The module includes following classes: SearchRequest, FeatureCollectionSearchService, VectorElementSearchService and VectorTileSearchService.  These classes can be used to search features from loaded geojson collections, vector data sources and vector tile data sources.
* Billboards now support fade-in/fade-out animations. AnimationStyle objects can be now attached to billboard   StyleBuilder objects and the specified animations will be used when billboard appear/disappear.
* PackageManager now includes two additional methods: isAreaDownloaded and suggestPackage. These methods can be used to detect is the view area is downloaded for offline use and if not, to get the best package for the area.
* SDK now support optional zoom gestures. Options class includes setZoomGestures/isZoomGestures methods,  when zoom gestures are turned on, SDK automatically interprets double tap as a zoom-in action and two finger tap as a zoom-out action. By default, zoom gestures are not enabled.
* Implemeted RasterTileClickEventListener class for receiving click events on raster tile layers. SDK provides  click coordinates and the raster tile color at the click point.
* Implemented simulateClick method for Layer class. This method can be used to programatically call event handlers of the layer.
* Implemented automatic background/sky color calculation for VectorTileLayers. If background/sky image is not explicitly defined using Options, then appropriate background/sky image is generated by the SDK.  This provides much better experience with dark styles compared vs SDK 4.0.x.
* Implemented setClearColor/getClearColor for Options class to specify background color of the MapView. This can be used to enable partially transparent map views.
* CartoOnlineDataSource has now support for 'water masks' and coarse water tiles are automatically detected and no longer requested from the server, thus reducing latency and providing better user experience.
* Added getDataExtent method TileDataSource class.  SDK uses the datasource extent information when generating tiles and this results in much lower memory usage in some cases (local raster overlays, for example).
* Added getDataExtent method VectorDataSource class.
* Exposed screenToMap and mapToScreen methods of MapRenderer with explicit ViewState argument.
* Added new helper classes VariantArrayBuilder and VariantObjectBuilder for building Variant instances.
* Added containsObjectKey method to Variant
* The performance of the clustering (ClusteredVectorLayer) is improved up to 10x. Also, the memory usage  of the clustering is now 2x lower. Due to the improvements, clusters of 100k points should works well  even on lowend devices.
* Optimized memory usage of LocalVectorDataSource setAll/addAll methods.
* ClusteredVectorLayer now monitors which attributes of elements change and avoids unnecessary costly reclustering.
* Added option to disable clustering animations via setAnimatedClusters method
* Added new option for faster clustering: ClusterElementBuilder includes additional buildClusterElement method (with cluster position and 'count' arguments). ClusterElementBuilder can specify ClusterBuilderMode which determines which of the two buildClusterElement method gets called.
* Lower level vector tile text rendering uses now SDF (Signed Distance Field) glyph representation which gives
  crisper texts especially on high-DPI devices. Also, memory usage of glyph atlas textures is reduced.   Additionally, the rendering artifacts of vector tile texts with large halos and overlapping glyphs are now fixed.
* Better support for shared dictionaries for offline packages to reduce package sizes.
* Added addFeatureCollection method to LocalVectorDataSource
* CartoVectorTileLayer includes static createTileDecoder method that can used to instantiate VectorTileDecoder from built-in styles.
* Added isOpen method to PersistentCacheTileDataSource.
* PersistentCacheTileDataSource now support asynchronous tile download/cache prefill (startDownloadArea method). An optional listener can be used to monitor tile download progress.
* Implemented setVectorTileBufferSize method for CartoMapsService. This method can be used to tweak tile sizes/fix rendering artifacts  when using vector tiles from CARTO Maps API.
* Reduced memory consumption when large vector tiles are used
* iOS: added support for converting 16 bits-per-component UIImages
* UWP: Added mouse wheel support for zooming.
* Optimizations for GeoJSONGeometryReader, loading large geojson files is now approximately 2x faster
* Implemented ClickSize property for MarkerStyle, this allow enlarging of the click area when very small markers are used.
* Faster loading of complex vector tiles, SDK now optimizes CartoCSS styling rules.
* Optimized memory usage of complex Polygon vector elements (up to 25% in complex cases).
* New classes VectorTileFeature and VectorTileFeatureCollection that are used by the new search API
* CartoCSS: Implemented 'pow' operator
* CartoCSS: Added support for metavariables
* Implemented more optimizations in CartoCSS for various degenerate rendering rules: empty text expressions, zero size features, etc
* Added SideColor property to Polygon3DStyle/builder classes. Previously single Color was always used for all faces of the 3D polygon.
* Added toString method to BinaryData
* CartoCSS feature: comp-op support for markers
* CartoCSS: text-size attribute is now evaluated per-frame, allowing to use smooth text size interpolation based on zoom level
* CartoCSS: enabled PointSymbolizer support
* CartoCSS: parser now supports meta-variables


### Fixes:
* Fixed equals/hash implementation for several built-in classes. Previously both methods provided unreliable results.
* Tile layer preloading tweaks - avoid cache trashing and constant refreshing in rare cases, reduce preloading dataset size
* SDK does not show harmless 'failed to decode tile' warning for empty tiles anymore
* Fixed subtle case of duplicate Layer instance handling in Layers container
* SDK allows vector element to be attached to only a single data source, violating this results in an exception now
* Fixed Windows Phone/UWP related pointer handling, previous version assumed MapView control to be at (0, 0) coordinates in the window
* Fixed touch handling issues on Windows Phone when more than 2 fingers are used
* Fixed regression in SDK 4.0.2 vs 4.0.0 when rendering vector tile lines with null width
* CartoCSS: fixed handling of shield-text-opacity and shield-text-transform
* Fixed multigeometry bounds calculations
* Fixed alpha channel handling when translating color interpolation expressions from CartoCSS to rendering library


CARTO Mobile SDK 4.0.2
-------------------

Maintenance release for CARTO Mobile SDK 4.0.x

### Fixes/Changes:
* Enabled stack protector for Android builds for better app security
* Implemented null pointer checks throwing exceptions for various Layers methods, previously such cases could result in native level crashes
* Implemented workaround for Xamarin/Android multithreading issues - native threads were sometimes not automatically registered when managed delegates are called from multiple threads
* Fixed issues with online licenses when license server was unreliable and took long time to respond
* Fixed app token issues with CARTO named map services
* Fixed SDK log filters being ignored/not working
* Fixed CartoCSS marker-transform handling for non-overlapping points
* Fixed VectorTileLayer click detection when custom transform was applied
* Fixed layer background not being properly set when VectorTileDecoder was updated


CARTO Mobile SDK 4.0.1
-------------------

This is a maintenance release for 4.0.x that includes several important reliability and performance fixes, in addition to
some minor new features.

### New features and changes:
* Added Layer visibility control API to CartoVectorTileDecoder (setLayerVisible, isLayerVisible methods)
* Implemented 'screen' and ‘clear’ comp-op support for CartoCSS/vector tile rendering
* Rendering of vector tile layers with multiple line/polygon symbolisers is now optimized as a special case, this is usually done with a single draw call
* Changed moveToFitBounds behaviour - from now SDK does not change zoom level if single point is used for MapBounds
* Better error reporting for CARTO SQL API, including error logging and error parsing
* Minor optimizations in vector tile renderer
* implemented timeout for online license update procedure
* forward-compatible changes for future features in online tile service and offline packages
* Exposed `CartoVectorTileDecoder` constructor for better integration with CARTO vector overlays
* Added additional `CartoOnlineVectorTile` constructor with explicit source and built-in style enumeration parameters
* Added `countVisibleFeatures` method to `TorqueTileLayer`
* Added comp-op support to points, markers, texts and shields
* Increased internal visible tile cache size by 4x, for really large overlay datasets (does not affect memory usage in normal cases)
* `MBTilesDataSource` and `OfflineNMLModelLODTreeDataSource` classes now open database in read-only mode (previously in read-write mode)
* More precise label coverage analysis for transformed labels

### Fixes:
* Fixed Torque tile usage  in MapsService API due to malformed URL
* Fixed deadlock with indirect texts fields in Text and BalloonPopup objects
* Fixed feature batching related issue in vector tile renderer that caused high number of draw calls and low performance
* Fixed 'multiply' comp-op handling with non-opaque alpha values
* Fixed parameter name typo in CartoCSS (instead of 'polygon-pattern-comp-op', 'polygon-pattern-op' was used)
* Fixed performance issue on iOS with empty Text objects
* CartoCSS compatibility fixes for handling negative line widths and marker sizes
* Minor memory usage, speed optimizations
* Added missing NTCartoVectorTileDecoder to iOS umbrella header
* Fixed CartoVectorTileDecoder layer ordering issues
* Fixed regression regarding VisJSON vector sublayer grouping; visibility and attribute info was previously lost
* Fixed handling of zero size ellipse markers in CartoCSS
* Fixed vector tile click detection issues  
* Fixed rare cases on iOS when screen remained black after returning from background state
* Heavily distorted texts are no longer displayed on the map
* Fixed bad_weak_ptr exception when using PersistentCacheTileDataSource
* Fixed crash with some Xamarin Android versions when MapView finalizer is called
* Fixed license registration issues on Windows Phone targets
* Fixed vector tile layers in layergroup ignoring 'visibility' attribute
* Fixed billboard sorting issues causing flickering with overlapping markers/texts/popups
* Implemented clamping for CartoCSS opacity values for better compatibility


CARTO Mobile SDK 4.0.0
-------------------

CARTO Mobile SDK is built on top of [*Nutiteq Maps SDK 3.3*](http://developer.nutiteq.com), and includes over 100 API related improvements, performance updates and fixes. The new API is not compatible with Nutiteq SDK 3.3, but most apps can be converted relatively quickly and most changes are only related to class/module naming. See [Upgrading from Nutiteq SDK](https://github.com/CartoDB/mobile-sdk/wiki/Upgrading-from-Nutiteq) for more details.

Release notes for next releases can be found from [Releases section](https://github.com/CartoDB/mobile-sdk/releases).

### New features and improvements:
* New 'services' module that gives integration with CARTO online services (Maps services, SQL API, high level VisJSON map configuration)
* JSON serializing/deserializing support and JSON based vector element metadata
* Revamped tile layer support, with more shared features between all tile layers including generic UTF grid support for vector/raster tile layers and many other tweaking options
* Vector editing is now available in all builds (Nutiteq SDK included this only in special GIS builds)
* Improved GeoJSON support, supporting GeoJSON features and feature collections
* Improved and more compliant CartoCSS support for vector tiles with 2 times faster CartoCSS parsing/compiling speed
* Additional styling options for vector overlays (lines, 3D polygons)
* Event handling by layer specific listeners
* Full Collada standard material support in NML models
* Usage of exceptions to signal about most common error cases, for example, file access errors, null pointers, out of range indexing
* Faster vector basemap rendering with better text quality
* Faster and higher quality vector overlay rendering (especially lines)
* Click detection and feature introspection for vector tiles

### Removed features:
* Windows Phone 8.1 is no longer supported, as the platform is generally deprecated, only Windows Phone 10 is now supported
* Basic CartoCSS styling support is removed from styles module, full CartoCSS is available for vector tiles
