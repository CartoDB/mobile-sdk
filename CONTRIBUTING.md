## How to contribute:

* [Add issues](https://github.com/CartoDB/mobile-sdk/issues) for found bugs and new feature ideas. Check first if issue for specific thing does not already exist
* See readme.md for compilation guides
* Use Pull Requests to contribute with own code additions

Additional developer channels and more detailed guidelines will be added soon.

## Submitting contributions

Before opening a pull request (or submitting a contribution) you will need to sign a Contributor License Agreement (CLA) before making a submission, [learn more here](https://carto.com/contributing).

After that, there are several rules you should follow when a new pull request is created:

- Title has to be descriptive. If you are fixing a bug don't use the ticket title or number.
- Explain what you have achieved in the description and choose a reviewer (it has to be a CARTO team member) of your code. If you have doubts, just ask for one.

## Directories

* all/native - c++ sources for all platforms, main part of the SDK
* all/modules - swig configuration files
* android - Android-specific proxies (Java, C#) code, swig wrapper config and helpers
* assets - images and style files sources used for map rendering. Converted to c++ headers
* doc - documentation, published in CARTO docs site
* dotnet - .NET-specific C# proxy code
* ios - iOS-specific proxies (Objective-C, C#, Swift) code, swig wrapper config and helpers
* libs-carto - parts of SDK code which is used as internal library, have no dependencies to main SDK code and can be technically used as stand-alone code.
* libs-carto/cartocss - CartoCSS parser
* libs-carto/mapnikvt and vt - Mapnik Vector Tile reader and renderer
* libs-carto/nml - Our specific 3D format and 3D tile format (NML) readers and renderers
* libs-carto/routing - Offline routing using our specific route packages
* scripts - scripts for building SDK, cmake and others
* winphone - Windows Phone-specific C# proxies, swig config and native helpers
