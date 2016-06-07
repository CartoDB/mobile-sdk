# MBTiles map data
[MBTiles](http://mapbox.com/developers/mbtiles/) support is included in Nutiteq SDK. MBTiles uses SQLite database format, so you have just one file to be downloaded and managed. Following are some tools to create MBTiles packages:

## a) Raster maps

* [MapTiler](http://www.maptiler.com/) is nice utility to create MBTiles from raster geo files (GeoTIFF, JPG, ECW etc), it is well worth small price to support the developer
* [TileMill](http://mapbox.com/tilemill/) is open source generator of very nice map packages, if source is vector geo file, e.g. Shapefile or PosgGIS geo data.
* [MOBAC](http://mobac.sourceforge.net) - download from variety of free sources (e.g. Bing, OpenStreetMap etc), can even load from WMS with added configuration
* [MBUtil](https://github.com/mapbox/mbutil) enables to create mbtiles from/to TMS-style tile folders, created with e.g. GDAL utility
* [Portable Basemap Server](https://geopbs.codeplex.com/) - free utility for Windows, loads data not only from various commercial servers and custom sources, but also from ESRI formats. Works mainly as WMTS server, but can create MBTiles as extra feature.

## b) Vector maps

* Nutiteq SDK bundled [Package Manager API](/guides/offline-maps) downloads country-wide or bounding box based map packages with **OpenStreetMap data**. These files are not directly visible, as Package Manager downlods the files, you only use the API.
* If you need to create vector map files **from different sources or your own data** (e.g. Shapefiles), then please contact Nutiteq (support@nutiteq.com) - we can provide you converter toolchain, and do free demo conversions.

