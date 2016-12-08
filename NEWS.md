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
