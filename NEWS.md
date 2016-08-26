4.0.0 preview (2016-XX-XX)
-------------------
### NOTICE
This is preview version of CARTO Mobile SDK 4.x. It is based on [Nutiteq Maps SDK](https://developer.nutiteq.com) 3.3.0, so the changes are given based on this.

### Bugfixes
* This is rolling snapshot version, new bugfixes and builds are done every day

### New features 
* New 'services' module that gives integration with CARTO online services (Maps services, SQL API, high level VisJSON map configuration)
* JSON serializing/deserializing support and JSON based vector element metadata
* Revamped tile layer support, with more shared features between all tile layers including generic UTF grid support for vector/raster tile layers
* Improved GeoJSON support, supporting GeoJSON features and feature collections
* Improved and more compliant CartoCSS support for vector tiles
* Additional styling options for vector overlays (lines, 3D polygons)
* Event handling by layer specific listeners
* Full Collada standard material support in NML models
* Usage of exceptions to signal about most common error cases, for example, file access errors, null pointers, out of range indexing
* Faster vector basemap rendering
* Faster and higher quality vector overlay rendering (especially lines)

### Removed features
* Windows Phone 8.1 is no longer supported, as the platform is generally deprecated, only Windows Phone 10
* NMLModelLODTree support is now removed from standard package, available as enterprise option
* Basic CartoCSS styling support is removed from styles module, full CartoCSS is available for vector tiles
