# Carto Mobile SDK readme

## Dependencies
Get the prepackaged dependencies from github CartoDB/mobile-external-libs
repository (https://github.com/CartoDB/mobile-external-libs). Add links
to external-libs/prebuilt at the project root directory level.

In addition, boost library should be downloaded and configured as described
in mobile-external-libs documentation.

Special swig version (swig-2.0.11-nutiteq) is needed for generating 
language-specific wrappers, this can be downloaded from https://bitbucket.org/nutiteq/swig-2.0.11-nutiteq

## Building
Use swigpp-XXX.py scripts for generating the wrapper and proxies for target platforms
and build-YYY.py scripts for building the SDK.
