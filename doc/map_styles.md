# Vector Styles

Usage of vector-based base map enables you to re-style map according to your needs: set colors, transparency, line styles (width, patterns, casings, endings), polygon patterns, icons, text placements, fonts and many other vector data parameters. Nutiteq uses Mapnik (http://mapnik.org) XML style description language for customizing the visual style of vector tiles. Our styling is optimized for mobile, so we add some style parameters, and ignore some others, but generally you can mostly reuse your existing Mapnik XML or CartoCSS styling files and tools (like TileMill/Mapbox Studio).

Vector styling is applied in mobile client side, and the style files are usually bundled with application installer. Application can change the styling anytime without reloading vector map data, so you can download map data once, and change styling from "day mode" to "night mode" with no new downloads.

Nutiteq map rendering implementation is intended for real-time rendering and, as a result, several limitations apply.

### Ready-made styles
See [Downloads](/downloads#Vector style files) for ready-made styles by Nutiteq. These are compatible with our vector tile sources.

### Mapnik style format
Mapnik map style definition is a common file format for map styles, it is based on XML. It is originally done for Mapnik, but is used by other softwares like Nutiteq SDK. File format specification is in [XMLConfigReference](https://github.com/mapnik/mapnik/wiki/XMLConfigReference) document. There are several ways you can use the styles, from simplest to most advanced:

 1. Use Nutiteq provided styles as they are
 1. Modify style.xml inside sample style to tweak it
 1. Create own style using some tool which edits Mapnik styles. Probably currently the best tool is free MapBox Studio, which uses CartoCSS as primary style definition. We don't use CartoCSS in Nutiteq SDK, but Studio but can export also Mapnik XML styles. However, these style files needs a bit modification to be compatible with Nutiteq SDK. 

### Creating style package

Nutiteq vector styles are distributed as zip-archives. All style-related files/folders must be placed into a single zip file.
The most important part of the style is a style definition file, usually named _project.xml_. This file contains style descriptions of all layers and it usually references other files, like fonts, icons, pattern bitmaps which should be placed in various subfolders.

### Limitations/incompatibilities

* There are no built-in fonts, fonts must be explicitly added to the project

* Loading SVG icons is not supported, such icons should be converted to PNG format (Mapnik provides _svg2png_ utility)

* Original layer ordering is not always preserved, texts are always drawn on top of 2d geometry, 3d buildings are drawn on top of texts

* Layer opacity works per element, not per layer as in Mapnik. For non-overlapping elements, the result will be same. For overlapping elements there are likely artifacts.

* _comp-op_ feature is not supported (neither layer/symbolizer). Per symbolizer _comp-op_ support is under investigation

* _line-join_ parameter is ignored, only _miter_ line join is used. 

* _GroupSymbolizer_ and _RasterSymbolizer_ are not supported, support is under investigation

* Text characters are rendered one by one. If characters overlap, halo of one character may cover glyph of another character. Workaround is to increase spacing or decrease halo radius.

* This list is not final. Mapnik XML is not official standard, and mostly due to performance penalty on the mobile Nutiteq SDK does not implement 100% of the tags and features of Mapnik. If you need some of the not implemented styling options, please contact our support. 

### Performance hints 

* **Multiple symbolizers per layer may have very large performance hit**. If possible, move each symbolizer into separate layer.

* _BuildingSymbolizer_ requires expensive OpenGL frame buffer read-back operation and may perform very poorly on some devices (original iPad Retina)

* It is best to keep all bitmaps with power-of-two dimensions, this increases performance.

### Nutiteq-specific extension to Mapnik style files

#### _NutiParameters_

_NutiParameters_ describe additional parameters that can be used in styles and controlled in the code (from _MBVectorTileDecoder_).
Parameters are typed, have default values and can be used as variables within _nuti_ namespace in the style (for example, *[nuti::lang]*).
Some parameters may have _ prefix in their name. Such variables are reserved and should not be updated directly by the application.

Simple example of _NutiParameters_ section in style xml file:

`<NutiParameters>`
`  <NutiParameter name="lang" type="string" value="en" />`
`</NutiParameters>`

#### Metavariables

Metavariables add support for dynamic variable look up. Variable names may depend on other variables. For example, *[name_[nuti::lang]]*.
If the value of *nuti::lang* is 'en', this metavariable expression would be equal to *[name_en]* expression.

#### Conditional operator

Conditional operator ?: adds support for simple control flow management. It is similar to C language conditional operator and can be used in all 
expressions. For example:

`[nuti::lang] == 'en' ? 'English' : 'Other'`

#### 3D texts and markers

Mapnik _MarkersSymbolizer_ and _TextSymbolizer_ support additional value _nutibillboard_ for _placement_ parameter. This will make texts and
markers act as billboards (always facing the viewer) even when screen is tilted. This option can be used to give markers and texts more
'3d-like' look. For example,

`<MarkersSymbolizer placement="nutibillboard" fill="#666666" file="icon/[maki]-12.svg" />`

### Supported symbolizers and parameters

The following list contains all supported symbolizers and parameters, as of version 3.2.2:

* PointSymbolizer: file, opacity, allow-overlap, ignore-placement, transform

* BuildingSymbolizer: fill, fill-opacity, height, geometry-transform

* LineSymbolizer: stroke, stroke-width, stroke-opacity, stroke-linejoin, stroke-linecap, stroke-dasharray, geometry-transform

* LinePatternSymbolizer: file, fill, opacity, geometry-transform

* PolygonSymbolizer: fill, fill-opacity, geometry-transform

* PolygonPatternSymbolizer: file, fill, opacity, geometry-transform

* MarkersSymbolizer: file, placement, marker-type, fill, opacity, width, height, spacing, allow-overlap, ignore-placement, transform

* TextSymbolizer: name, face-name, fontset-name, placement, size, spacing, fill, opacity, halo-fill, halo-opacity, halo-radius, allow-overlap, minimum-distance, text-transform, orientation, dx, dy, wrap-width, wrap-before, character-spacing, line-spacing, horizontal-alignment, vertical-alignment

* ShieldSymbolizer: name, face-name, fontset-name, placement, size, spacing, fill, opacity, halo-fill, halo-opacity, halo-radius, allow-overlap, minimum-distance, text-transform, orientation, dx, dy, wrap-width, wrap-before, character-spacing, line-spacing, horizontal-alignment, vertical-alignment, file, shield-dx, shield-dy, unlock-image
