CARTO Mobile SDK 4.0.1
-------------------

This is a maintenance release for 4.0 that includes several important reliability and performance fixes, in addition to
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
