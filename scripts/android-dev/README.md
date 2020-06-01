# Android development environment

Here are basic instructions how to set up an Android development environment.

### 1. Check out dependent subprojects

Follow the general instructions from the main page

### 2. Use 'lite' profile to generate SWIG wrappers/proxies

```
python swigpp-java.py --profile lite
```

Note that other profiles require changes in 'carto_mobile_sdk/build.gradle' file, namely 'SDK_CPP_DEFINES' 
must contain the corresponding 'defines' values from '../build/sdk_profiles.json'

### 3. Load the project into Android studio

The SDK will be automatically compiled when you try to run the project.

You should be able to step into the native methods when debugging the test app.
