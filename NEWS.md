CARTO Mobile SDK 4.3.4
-------------------

### Changes/fixes:

* Fixed out of range memory access issues when packing large VT geometries
* Fixed an issue in VT line clipping implementation causing missing initial vertices in border cases
* Optimizations when converting GeoJSON data to vector tile format (GeoJSONVectorTileDataSource)


CARTO Mobile SDK 4.3.4RC1
-------------------

### Changes/fixes:

* Added support for setting routing parameters to SGREOfflineRoutingService (setRoutingParameter, getRoutingParameter methods)
* Added 'placement-priority' support for vector tile labels, allowing setting priorities for individual labels
* Added onSurfaceChanged event to MapRendererListener. This method is called when map is resized.
* Reduced rendering artifacts of wide dashed lines in vector tile renderer
* Better precision when compressing vector tile coordinates, fixes rare visual artifacts
* Fixed critical Xamarin iOS synchronization redrawing/disposing issues, causing exceptions
* Fixed VectorTileLayer rendering issue related to opacity handling
* Fixed watermark options being ignored after initial rendering
* Fixed non-opaque highlight/shadow color handling in HillshadeRasterTileLayer
* Additional safety checks in Android bitmap conversions


CARTO Mobile SDK 4.3.3
-------------------

### Changes/fixes:

* Fixed regression in label ray-hit detection routine when using globe mode


CARTO Mobile SDK 4.3.3RC2
-------------------

### Changes/fixes:

* Fixed critical content scaling issue on iPhone Plus devices
* Started using API 30 as compilation target on Android


CARTO Mobile SDK 4.3.3RC1
-------------------

### Changes/fixes:

* Fixed PersistentCacheTileDataSource not working with large cache files
* Faster initialization of PeristentCacheTileDataSource with large database files
* Tweaks and fixes to vector tile feature click detection, marker images are now used to detect transparent pixels


CARTO Mobile SDK 4.3.2
-------------------

### Changes/fixes:

* Fixed PersistentCacheTileDataSource not working with large cache files
* Changed PersistentCacheTileDataSource to be more conservative when estimating cache file size


CARTO Mobile SDK 4.3.2RC2
-------------------

### Changes/fixes:

* Fixed multiline RTL text formatting in VT renderer


CARTO Mobile SDK 4.3.2RC1
-------------------

### Changes/fixes:

* Added two new properties to HillshadeRasterTileLayer: shadow color and highlight color
* Minor optimization: avoid tile reloading when listener is disconnected from the layer.
* Slighlty higher background thread priority for tile/data loading tasks
* Added dynamic thread creation to CancelableThreadPool when all workers are busy with lower priority tasks. 
* Fixed transform/orientation being ignored when flipping vector tile labels
* Enabled SIMD optimizations for WebP image library for slight performance boost


CARTO Mobile SDK 4.3.1
-------------------

### Changes/fixes:

* Fixed a crashing issue with ClusteredVectorLayer
* Minor documentation updates


CARTO Mobile SDK 4.3.1RC1
-------------------

### Changes/fixes:

* Added HillshadeRasterTileLayer to the SDK. It can be used to add additional height-based shading to the map.
* Added getTileFilterMode/setTileFilterMode methods to RasterTileLayer. This allows to choose raster tile filtering mode between point, bilinear and bicubic filters.
* Changed lighting vector calculation for globe mode - the lighting vector is now always based on the local tangent frame of focus point
* Deprecated compressToPng method in Bitmap class, use compressToPNG instead
* Fixed issues with HTTPTileDataSource when multiple tile download threads were used on iOS, by making HTTPClient thread safe
* Fixed potential native crash when thread pool is downsized
* Fixed thread race between layers and renderers when GL context was lost
* Fixed compatibility issues with older GPUs not supporting high precision in fragment shaders
* Slightly better error reporting for CartoCSS errors
* Slightly better error reporting for PNG reading errors


CARTO Mobile SDK 4.3.0
-------------------

### Changes/fixes:

* Fixed linking issue with Xamarin iOS build
* Minor optimizations for Android build when using JNI
* Minor logging cleanup
* Documentation changes


CARTO Mobile SDK 4.3.0RC3
-------------------

### Changes/fixes:

* Changed shading of building symbolizers in VT renderer. The lighting is now NOT applied to the top of the building. This matches the behavior of Mapnik.
* Enabled support for rendering buildings with negative height in VT renderer
* Fixed cyclical resource manager referencing, causing memory leaks on Android
* Fixed potential timing related crashes happening when disconnecting layers from MapRenderer
* Fixed a deadlock regression in 4.3.0RC2 when bitmap texture cache was released
* Fixed an issue in layer removal code that could cause removing more layers than intended


CARTO Mobile SDK 4.3.0RC2
-------------------

### Changes/fixes:

* Fixed memory leak when switching render projection
* Thread safety fixes when adding/removing layers to the map
* Fixed memory leak in iOS implementation of HTTPClient
* Throw exception instead of crashing when null ptr is used as an argument for Bitmap constructor
* Fixed very high memory consumption when calling getServerPackages method in PackageManager class
* Optimized internal representation of tilemask, reduced memory usage by 5x
* Fixed RasterTileListener not working (regression in SDK 4.2.x vs 4.1.x)
* Fixed issue with font shaping when '\n' symbol is used in text
* Fixed texture coordinate artifacts when using dashed lines in VT renderer
* Removed unneeded error from the log when translating CartoCSS expressions ('Unsupported text expression type ..')
* Updated the way CartoCSS 'marker-feature-id' and 'text-feature-id' properties treat null/zero values and empty strings: now using these special values forces SDK to generate 'auto id'
* Fixed potential vector tile rendering issues on devices that supported OpenGL Vertex Array extension
* Optimized resource usage when layers are removed from the map, the resources are released sooner than before, resulting in smaller application memory footprint
* Fixed rare display corruption issues when OpenGL surface was lost and layers were being removed from the map
* Fixed styling issues with VectorLayers when bitmaps were shared between different vector element styles
* Implemented proper 'line-cap' support for dashed lines
* Added 'custom parameters' option to GeocodingRequest and ReverseGeocodingRequest classes. Custom parameters can be used to customize specific parameters of geocoding engines.


CARTO Mobile SDK 4.3.0RC1
-------------------

This version is a major update and brings several new features and optimizations. Note that due to the inclusion of Valhalla 3,
then binaries of the SDK are considerably larger on Android compared to SDK 4.2.x.

### Key highlights:

* Valhalla 3 routing support. Valhalla 2 routing was supported in SDK 4.1.x and removed from SDK 4.2.x. This release brings Valhalla back but with new major version and lots of improvements. Note that previous Valhalla 2 offline packages are incompatible with Valhalla 3 and can not be used.
* Support for building the SDK with Metal rendering backend on iOS, instead of OpenGLES. This is currently still experimental, as it generates larger binaries and is a bit slower.

### New features:

* A fully featued matchRoute API for matching points to routing network and extracting routing attributes. 
* Added custom metadata support for Layer class (getMetaData, setMetaData, containsMetaDataKey methods in Layer class)
* Support for rendering basemap Point-of-Interests, API for directly controlling POI/building rendering mode (setPOIRenderMode/getPOIRenderMode methods in CartoVectorTileLayer class)
* API for controlling the render style of basemap buildings (setBuildingRenderMode/getBuildingRenderMode methods in CartoVectorTileLayer class)
* Added 'custom parameters' option to RoutingRequest and RouteMatchingRequest classes. Custom parameters can be used to customize routing schemas of specific routing engines.
* New helper classes FeatureBuilder and VectorTileFeatureBuilder
* Moved matchRoute method to base RoutingService interface
* Moved setProfile/getProfile methods to base RoutingService interface
* Moved setLanguage and setAutocomplete methods to base GeocodingService interface.
* Added setMaxResults and getMaxResults methods to base GeocodingService interface.
* Moved setLanguage method to base ReverseGeocodingService interface.
* Added setClickRadius and getClickRadius methods to VectorTileLayer
* Added setMaxResults and getMaxResults methods to all search services. Note that searches are now capped, thus applications may need to configure the limit appropriately.
* Added 'uppercase', 'lowercase', 'length', 'concat', 'match', 'replace' functions to CartoCSS compiler.
* Added 'regexp_ilike' operator to the search API query language to perform case-insensitive substring matching
* Added support for ARM64 UWP target, removed deprecated ARM UWP target.

### Changes/fixes:

* setGeometryTagFilters, getGeometryTagFilters methods in RoutingRequest are deprecated and will be removed in future versions. Instead use more general setPointParameter/getPointParameter methods with 'geometry_tag_filter' parameter name.
* Labels from different VectorTileLayer instances that have 'allow-overlap' flag set to false no longer overlap each other. This changes previous behavior where each VectorTileLayer did not affect other layers.
* SDK does not throw exception anymore when package manager device keys do not match, this fixes issues with TestFlight on iOS
* Tweaked and optimized offline geocoder, mostly affects autocomplete mode
* Better reporting of online Valhalla routing errors
* Added ferry instruction types (enter/leave ferry) to RoutingAction enum
* Fixed search API issues with tiles and non-closed polygons
* Tweaked rendering of lines with round join types to look smoother, especially when used with thin lines
* Suppressed GLKView deprecation warnings on iOS
* Additional NPE safety in OnlineNMLModelLODTreeDataSource
* Fixed native crash when loading 0-sized image files
* Minor improvements to CartoCSS error reporting.
* Made Mapnik-level string expression parsing recursive, fixes subtle issues with complex expressions
* Better SVG compatibility with RGBA color support


CARTO Mobile SDK 4.2.2
-------------------

### Changes/fixes:

* Fixed iOS specific compilation warning in NTExceptionWrapper.h ("This function declaration is not a prototype")
* Disabled LTO on iOS builds (fixes issue with bitcode generation on iOS platform)


CARTO Mobile SDK 4.2.2RC2
-------------------

### Changes/fixes:

* Fixed vector tile click radius of points if 'allow-overlapping' flag was set to true
* Fixed name wrapping of setWatermarkPadding method in Options class on iOS (was setWatermarkPaddingX, now setWatermarkPadding)
* Clearer error reporting when parsing CartoCSS styles
* Improvements and tweaks to text-on-line rendering in vector tiles


CARTO Mobile SDK 4.2.2RC1
-------------------

### Changes/fixes:

* Additional synchronization for iOS events to prevent potential GL calls while app is paused
* Fixed wrong rendering of vector tile labels using 'point-placement' mode 
* Fixed vector tile label transformation handling
* Speed and memory usage optimizations for vector tile labels
* Minor improvements to CartoCSS error reporting


CARTO Mobile SDK 4.2.1
-------------------

### Changes/fixes:

* Optimized symbol tables in Android .so libraries so SDK is now 5% smaller
* Fixed a potential NPE crash in VT glyph rendering code


CARTO Mobile SDK 4.2.1RC2
-------------------

### Changes/fixes:

* Tweaks to built-in styles to better prioritise rendering of low rank street names
* Better Mapnik compatibility by supporting linestring geometry in PolygonSymbolizer, PolygonPatternSymbolizer and BuildingSymbolizer
* Minor tweaks to line placement clipping against frustum in VT renderer
* Use constant padding around labels, fixes obscure issues with label click area for long texts
* Fixed issue with label click handling - due to label geometry merging wrong geometry was returned in certain cases


CARTO Mobile SDK 4.2.1RC1
-------------------

### Changes/fixes:

* The SDK can now be used without calling registerLicense method of MapView class if CARTO basemap services are not needed. In 4.2.1 and later versions we are showing normal CARTO watermark instead of evaluation watermark in this case.
* Added MapTilerOnlineTileDataSource class that can be used for MapTiler or OpenMapTiles tiles
* Added getGeometryTagFilters/setGeometryTagFilters methods to RoutingRequest; they can be used to filter routing endpoints. This is currently supported only when using SGRE routing engine.
* ValhallaOnlineRoutingService is now included in the standard SDK build. It was available in 4.1.x versions but removed from 4.2.0.
* Added clear methods to VariantArrayBuilder and VariantObjectBuilder classes
* Changed the behavior or MapView screenToMap and mapToScreen methods if called before view size is initialized - the SDK now returns NaNs
* CartoPackageManager constructor now throws an exception if it is instantiated without a valid license
* protected loadConfiguration method in CartoOnlineTileDataSource is no longer exposed
* Fixed MapView background clearing issue with Android Q beta versions


CARTO Mobile SDK 4.2.0
-------------------

### Changes/fixes:

* Added support for 'marker-feature-id', 'text-feature-id' and 'shield-feature-id' CartoCSS properties for uniquely identifying labels
* Fixed regression in 4.2.0RC2 vs RC1 regarding VectorTile hit results ordering
* Fixed render projection switching issues in 4.2.0RC1/RC2
* Fixed kinetic rotation clamping issue in 4.2.0RC1/RC2
* Fixed culling related performance issue in ClusteredVectorLayer
* Guards against null pointer exceptions in ClusteredVectorLayer when interfacing with custom builder
* Better handling of horizontal offsetting in TileRenderer


CARTO Mobile SDK 4.2.0RC2
-------------------

### Changes/fixes:

* Added BalloonPopupButton and related classes so that basic interactivity can be added to BalloonPopups
* Major SGRE optimizations: replaced one-to-one routing engine with many-to-many routing engine, using optimized data structures for routing
* Fixed/improved label ordering in vector tile renderer: prefer bigger labels over smaller ones
* Fixed geometry simplifier attached to LocalVectorDataSource causing a crash
* Fixed multiple issues with billboard sorting and ray casting.
* When calculating actual ray hit with billboard or point, SDK now uses actual bitmap to detect if the clicked pixel is transparent
* Implemented more consistent ordering of vector elements
* Changed billboard rendering to ignore depth testing. Better fit with 3D objects.
* Fixed potential rendering issue with GeometryCollections when switching between planar/spherical rendering mode
* Fixed ray-intersection code with Polygon3D, use the closest intersection point, not the first found point
* Fixed subtle flickering in ClusteredVectorLayer animations
* Minor performance optimization by using platform-optimized zlib
* Fixed getElementClickPos method of PopupClickInfo to return click coordinates as pixel coordinates, not normalized-to-size coordinates
* Fixed issue in SDK4.2.0RC1 that caused map rotation to change when setting focus position in globe view mode
* Fixed GeometryCollectionRenderer to accept both clockwise and counterclockwise oriented polygons
* Documentation fixes


CARTO Mobile SDK 4.2.0RC1
-------------------

This version is a major update and brings lots of new features and optimizations. Some features present in older releases are removed or deprecated in this version.

### Key highlights:

* Globe view support. Maps can be displayed in planar mode (as in previous versions) or in globe view mode.
* EPSG4326 support. WGS84 coordinates can be directly used without needing to convert them to EPSG3857.
* Indoor 3D routing by using GeoJSON input and custom routing profiles. We pulled experimental versions with this into 4.1.x releases, but have since made some changes and stabilized it.
* On-the-fly conversion GeoJSON to vector tiles, so that CartoCSS can be used for styling.
* Faster basemaps with several rendering optimizations.
* Better compatibility with Swift on iOS. SDK does not require bridging header anymore and can be simply 'imported'.
* Faster networking on iOS, by better utilizing OS-provided caching.
* Increased security, all basemap services use HTTPS connection by default.
* Startup time on Android has been significantly reduced. Previously low-end devices required more than a second to load the native SDK component. This loading time is reduced by at least 5 times.
* Basemap style parsing and loading is now faster due to smaller font assets and due to internal optimizations.
* SDK is considerable smaller due to several factors:
  - We have removed offline Valhalla routing support from the SDK. It is still available in the repository and SDK can be built with it.
  - We have removed some font assets from the SDK, so Arabic and few other scripts need external fonts.
  - We use carefully tuned compilation flags that produce smaller native binaries on all platforms.
* All SDK components are now open-source. In previous versions we kept one small component (LicenseManager) private, so custom builds could not connect to online services provided by CARTO. Now this restriction is removed.
* Improvements to build scripts, making compiling the SDK easier and less frustrating experience.


### New features:

* Added EPSG4326 projection. This allows to use longitude/latitude coordinates in the SDK directly, without the need to convert them first.
* New class GeoJSONVectorTileDataSource - provides on-the-fly conversion from GeoJSON layers to vector tiles. This is useful for indoor mapping and allows to use SDKs vector tile renderer with CartoCSS styling.
* New class SGRERoutingService for indoor routing. Additional details can be found in Wiki.
* New class MergedMBVTTileDataSource that merges two MapBox Vector Tile sources into one.
* Added addFallbackFont method to VectorTileDecoder class. This can be used to supply universal fallback font (as binary .TTF asset) for basemaps.
* Added setRenderProjection/getRenderProjection methods to Options class, for switching between planar and globe mode.
* Implemented 3D coordinate support for VectorElements. Previously only billboards handled Z coordinate properly, while using non-zero Z coordinate for polygons or lines produced undefined and usually wrong results.
* Added setZBuffering/isZBuffering methods to VectorLayer. Z buffering may be needed if 3D coordinates are used for lines or polygons.
* Added NMLModelStyle and NMLModelStyleBuilder classes for constructing style instances for NMLModels.
* New HTTP connection class for iOS that works better with device proxy settings and provides better download concurrency.
* Added setSkyColor, getSkyColor to Options class
* Added getMidrange method to MapRange
* Added shrinkToIntersection method to MapBounds
* CartoCSS improvements, 'marker-clip' support, 'north-pole-color', 'south-pole-color' map settings support

### Deprecated features:

* NMLModel constructors with explicit model assets are now deprecated. Use constructors with NMLModelStyle argument instead.


### Removed features:

* Built-in map styles are now smaller and load faster due to fewer built-in fonts. Arabic and few eastern scripts that were displayed in previous versions now require custom font assets. These can be supplied to VectorTileDecoder using addFallbackFont method.
* Removed setSkyBitmap/getSkyBitmap methods from Options class. Sky bitmap usage was poorly documented and relied too much on internal implementation. Use setSkyColor instead of setSkyBitmap.
* simplify method is no longer exposed in GeometrySimplifier class and its subclasses.
* Frustum class is removed from the SDK.
* ViewState class does not expose getCameraPos, getFocusPos, getUpVec, getFrustum methods starting from version 4.2.
* setProjectionMode/getProjectionMode methods are removed ViewState class. Setting projection mode never really worked.
* Removed fromInternalScale method from Projection. This method was never expected to be part of public API and was not useful for applications.
* ValhallaOnlineRoutingService, ValhallaOfflineRoutingService and PackageManagerValhallaRoutingService classes are removed from the public build. SDK used customized version of Valhalla that is not compatible with the latest official Valhalla versions and the library made SDK binaries considerably larger. Valhalla support is still present in the code, it is possible to build a custom version supporting these classes.
* CartoVisBuilder and CartoVisLoader classes are removed from the SDK. These classes provided experimental 'vizjson' support, but were never really complete. 'vizjson' is now deprecated by CARTO.


### Changes:

* All online connections to CARTO services are secure by default. Previously some non-critical services used plaintext connections, causing problems with some newer devices (Android 9) having strict security settings.
* MapView screenToMap now returns NaNs in coordinates if mapping from a given pixel is not possible (tilted map when using sky coordinates, for example)
* EPSG3857 toWgs84 does not return longitude in range -180..180 if the input X coordinate is outside of projection bounds.
* Default panning bounds is now ((-inf, -inf), (inf, inf)) instead of EPSG3857 bounds as in previous versions.
* Sky rendering implementation and default sky color has changed
* Restricted panning mode implementation and behaviour has slightly changed
* All internal fields of wrapped SDK classes on Android are now marked 'transient' and are never serialized. In previous versions trying to serialize/deserialize SDK classes caused native crashes during subsequent GC cycle. The new behviour should result in NPEs and not hard crashes.
* Algorithm for placing text on lines in vector tile renderer is re-implemented and should fix previously distorted placements
* iOS HTTP network stack now uses NSURLSession API for better performance and compatibility. Note that this may cause issues with custom HTTP datasources that do not use secure protocol.
* Much faster handling of [view::zoom] parameter in CartoCSS expressions
* Slightly more compact internal vector tile representation for rendering, gives better tile cache utilization and faster performance


### Fixes:

* setColor, setBitmap, setBitmapScale methods in SolidLayer class properly update the view when called.
* Fixed a memory leak in Java-specific BinaryData constructor taking byte array argument
* Fixed setPreserveEGLContextOnPause not properly invoked in Android MapView class
* Improved compatibility with Android devices with very old GPUs
* Minor search API query language fixes, better support for unicode strings
* Fixed vertex array binding issues with NMLModel rendering
* Fixed minor glyph rendering issues causing glyphs to be slightly blurry under tilted view.
* Minor CartoCSS fixes related to patterned symbolizer support
* Fixed OrderedTileDataSource handling of 'replace with parent' flag


CARTO Mobile SDK 4.1.6
-------------------

This update includes performance and stability improvements,
bug fixes and some minor new features. A new routing engine is introduced 
as an experimental feature.

### New features:

* Added experimental indoor routing support via SGREOfflineRoutingService class.


### Fixes/changes:

* A reworked implementation of HTTP connection worker for iOS that fixes airplane mode switching issues.
* ValhallaOnlineRoutingService now connects to MapBox online service instead of defunct MapZen online service
* Added matchRoute method to ValhallaOnlineRoutingServices
* Added 'wheelchair' routing profile support for Valhalla routing services
* Optimized MBTilesTileDataSource constructor with no explicit minZoom and maxZoom arguments, zoom range is now read first from 'metadata' table. If this fails, full table scan is performed.
* getDataExtent method of MBTilesTileDataSource is now more robust for bad values in 'metadata' table
* GeometryCollectionStyle can now be used when importing FeatureCollection consisting of normal points, lines, polygons to LocalVectorDataSource
* Fixed OrderedTileDataSource getMaxZoom method implementation
* AssetPackage class can now be subclassed from applications
* SDK now handles empty vector tiles as a general case, renders them with background color, not as transparent tiles. 
* Compatibility fix for CartoOnlineVectorTileDataSource by handling 404 code according to server changes (display empty ground tile)
* Added missing header to iOS umbrella header (NTCombinedTileDataSource.h)


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
* Exposed CartoVectorTileDecoder constructor for better integration with CARTO vector overlays
* Added additional CartoOnlineVectorTile constructor with explicit source and built-in style enumeration parameters
* Added countVisibleFeatures method to TorqueTileLayer
* Added comp-op support to points, markers, texts and shields
* Increased internal visible tile cache size by 4x, for really large overlay datasets (does not affect memory usage in normal cases)
* MBTilesDataSource and OfflineNMLModelLODTreeDataSource classes now open database in read-only mode (previously in read-write mode)
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
