# About

This is package to create Nutiteq SDK 3.x Xamarin component for the store


# How to compile package

 1. build iOS/Android DLL, MDB and XML (documentation) files using ../scripts/build-xamarin.py script
 1. add samples to samples/folders (see yaml). Same as in github.com/nutiteq/hellomap3d-dotnet project
 2. update component.yaml - version number
 3. in same folder where is component.yaml run:
  `mono ../../xamarin-component.exe package`

 
# Installing locally (for testing)

win:
  `xamarin-component.exe install NutiteqMapsSDK-3.2.4.xam`

mac:
  `mono ../../xamarin-component.exe install NutiteqMapsSDK-3.2.4.xam`


# References
* Official guide: http://developer.xamarin.com/guides/cross-platform/advanced/submitting_components/component_submission_quickstart/ - here you get latest *xamarin-component.exe*

# TODO
* remove duplicate DLLs from samples and package

