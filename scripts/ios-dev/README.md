# iOS development environment

Here are basic instructions how to set up an iOS development environment.

### 1. Check out dependent subprojects.

Follow the general instructions from the main page

### 2. Generate SWIG wrappers/proxies

```
python swigpp-objc.py --profile nmlmodellodtree+valhalla
```

### 3. Compile the SDK in debug mode

```
python build-ios.py --profile nmlmodellodtree+valhalla --configuration Debug
```

### 4. Use any project (for example, hellomap from mobile-ios-samples)

Remember to link to the generated library (placed under 'dist/ios/CartoMobileSDK').

You should be able to step into the native methods when debugging the test app.
