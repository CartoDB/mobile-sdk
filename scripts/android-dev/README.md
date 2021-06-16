# Android development environment

Here are basic instructions how to set up an Android development environment.

### 1. Check out dependent subprojects

Follow the general instructions from the main page

### 2. Use 'valhalla+nmlmodellodtree' profile to generate SWIG wrappers/proxies

```
python swigpp-java.py --profile valhalla+nmlmodellodtree
```

Note that other profiles require changes in 'carto_mobile_sdk/build.gradle' file, simply update the
value of 'profiles' variable in that file.

### 3. Load the project into Android studio

The SDK will be automatically compiled when you try to run the project.

You should be able to step into the native methods when debugging the test app.
