const CartoMobileSDK = {
    onRuntimeInitialized: function onRuntimeInitialized() {
        CartoMobileSDK.FS.mkdir('assets');
        startMap();
    },
};
CModule(CartoMobileSDK);

const imgScale = window.devicePixelRatio > 4 ? 4 : window.devicePixelRatio;

function startMap() {
    CartoMobileSDK.Log.SetShowInfo(false);
    const stencil = !(navigator.vendor === 'Google Inc.' && navigator.userAgentData.mobile);
    const runOnMainThread = false;
    mapView = new CartoMobileSDK.MapView('#canvas', runOnMainThread, stencil);
    const options = mapView.getOptions();
    options.setTiltRange(new CartoMobileSDK.MapRange(40, 90));
    options.setZoomGestures(true);
    // options.setRenderProjectionMode(CartoMobileSDK.RenderProjectionMode.SPHERICAL);
    // setMapListener(mapView);
    mapView.start();

    addRasterLayer(mapView);
    addMBTiles(mapView);

    const pos = new CartoMobileSDK.MapPos(-8238512, 4970883);
    mapView.setZoom(18.0 - window.devicePixelRatio, pos, 2.0);
    mapView.setTilt(60, 2.0);

    localDataSource = createVectorLayer(mapView, new CartoMobileSDK.EPSG3857());
    addMarker(localDataSource);
    addLine(localDataSource);
    addText(localDataSource);
    addPopup(localDataSource);
    setTimeout(() => addPlane(localDataSource), 2500);
}

function addRasterLayer(mapView) {
    const mapUrl = 'https://{s}.basemaps.cartocdn.com/light_all/{zoom}/{x}/{y}@1x.png';
    const httpDataSource = new CartoMobileSDK.HTTPTileDataSource(0.0, 24.0, mapUrl);
    // console.log(httpDataSource, httpDataSource.getBaseURL(), httpDataSource.getMinZoom(), httpDataSource.getMaxZoom(), httpDataSource.getSubdomains());

    const cacheDataSource = new CartoMobileSDK.MemoryCacheTileDataSource(httpDataSource);
    const rasterTileLayer = new CartoMobileSDK.RasterTileLayer(cacheDataSource);
    mapView.getLayers().add(rasterTileLayer);
}

function addMBTiles(mapView) {
    const fetches = [
        fetch('assets/sample.mbtiles').then(response => response.arrayBuffer()),
        fetch('assets/style.zip').then(response => response.arrayBuffer())
    ];
    Promise.all(fetches).then(([mbTilesData, styleData]) => {
        CartoMobileSDK.FS.writeFile('/assets/sample.mbtiles', new Int8Array(mbTilesData));
        CartoMobileSDK.FS.writeFile('/assets/style.zip', new Int8Array(styleData));
    }).then(() => {
        const mbDataSource = new CartoMobileSDK.MBTilesTileDataSource(0, 24, "/assets/sample.mbtiles");
        const styleAsset = CartoMobileSDK.AssetUtils.LoadAsset("style.zip");
        // console.log('styleAsset.size(): ', styleAsset.size());
        const assetPackage = new CartoMobileSDK.ZippedAssetPackage(styleAsset);
        const styleset = new CartoMobileSDK.CompiledStyleSet(assetPackage, "style");
        const tileDecoder = new CartoMobileSDK.MBVectorTileDecoder(styleset);
        const layer = new CartoMobileSDK.VectorTileLayer(mbDataSource, tileDecoder);
        layer.setLabelRenderOrder(CartoMobileSDK.VectorTileRenderOrder.VECTOR_TILE_RENDER_ORDER_LAST);
        setVectorTileListener(layer, tileDecoder);

        mapView.getLayers().add(layer);
    });
}

function createVectorLayer(mapView, proj = new CartoMobileSDK.EPSG3857()) {
    const localDataSource = new CartoMobileSDK.LocalVectorDataSource(proj);
    const vectorLayer = new CartoMobileSDK.VectorLayer(localDataSource);
    vectorLayer.setOpacity(1);
    mapView.getLayers().add(vectorLayer);
    return localDataSource;
}

function addMarker(localDataSource) {
    const pos = new CartoMobileSDK.MapPos(-8237821, 4970805);

    const markerStyleBuilder = new CartoMobileSDK.MarkerStyleBuilder();
    markerStyleBuilder.setSize(20.0);

    const marker = new CartoMobileSDK.Marker(pos, markerStyleBuilder.buildStyle());
    localDataSource.add(marker);
}

function addLine(localDataSource) {
    const poses = new CartoMobileSDK.MapPosVector;
    poses.push_back(new CartoMobileSDK.MapPos(-8237821, 4970805));
    poses.push_back(new CartoMobileSDK.MapPos(-8238832, 4970287));

    const lineStyleBuilder = new CartoMobileSDK.LineStyleBuilder();
    const color = new CartoMobileSDK.Color(255, 0, 0, 125);
    lineStyleBuilder.setColor(color);

    const line = new CartoMobileSDK.Line(poses, lineStyleBuilder.buildStyle());
    localDataSource.add(line);
}

function addText(localDataSource) {
    const builder = new CartoMobileSDK.TextStyleBuilder();
    builder.setOrientationMode(CartoMobileSDK.BillboardOrientation.BILLBOARD_ORIENTATION_FACE_CAMERA_GROUND);

    const text = new CartoMobileSDK.Text(new CartoMobileSDK.MapPos(-8238832, 4970287), builder.buildStyle(), "Face camera ground text");
    localDataSource.add(text);
}

function addPopup(localDataSource) {
    const fetches = [
        fetch(`assets/info@${imgScale}.png`).then(response => response.arrayBuffer()),
        fetch(`assets/arrow@${imgScale}.png`).then(response => response.arrayBuffer())
    ];
    Promise.all(fetches).then(([infoPNG, arrowPNG]) => {
        CartoMobileSDK.FS.writeFile(`/assets/info@${imgScale}.png`, new Int8Array(infoPNG));
        CartoMobileSDK.FS.writeFile(`/assets/arrow@${imgScale}.png`, new Int8Array(arrowPNG));
    }).then(() => {
        const infoPNG = CartoMobileSDK.BitmapUtils.LoadBitmapFromAssets(`info@${imgScale}.png`);
        const arrowPNG = CartoMobileSDK.BitmapUtils.LoadBitmapFromAssets(`arrow@${imgScale}.png`);
    
        const builder = new CartoMobileSDK.BalloonPopupStyleBuilder();
        builder.setDescriptionWrap(false);
        builder.setPlacementPriority(1);
        builder.setLeftImage(infoPNG);
        builder.setRightImage(arrowPNG);
        builder.setRightMargins(new CartoMobileSDK.BalloonPopupMargins(2, 6, 12, 6));
        // builder.setTitleColor(new CartoMobileSDK.Color(255, 0, 0, 255));
        // builder.setTitleFontSize(24);
        // builder.setDescriptionFontSize(24);

        const popup = new CartoMobileSDK.BalloonPopup(new CartoMobileSDK.MapPos(-8237821, 4970287), builder.buildStyle(), "This title will be wrapped if there's not enough space on the screen.", "ActivityData is set to be truncated with three dots, unless the screen is really really big.");
        localDataSource.add(popup);
    
    });
}

function setMapListener(mapView) {
    const Listener = CartoMobileSDK.MapEventListener.extend("MapEventListener", {
        onMapIdle: function() {
            console.log('onMapIdle');
        },
        onMapMoved: function() {
            console.log('onMapMoved');
        },
        onMapStable: function() {
            console.log('onMapStable');
        },
        onMapInteraction: function(mapInteractionInfo) {
            console.log('onMapInteraction', mapInteractionInfo.isTiltAction());
        },
        onMapClicked: function(mapClickInfo) {
            console.log('onMapClicked');
        },
    });
    const listener = new Listener;
    mapView.setMapEventListener(listener);
}

function setVectorTileListener(vectorTileLayer, tileDecoder) {
    const Listener = CartoMobileSDK.VectorTileEventListener.extend("VectorTileEventListener", {
        onVectorTileClicked: function(c) {
            const name = c.getFeature().getProperties().getObjectElement("name").getString();
            tileDecoder.setStyleParameter("selectedRoomName", name);
            console.log('onVectorTileClicked', name, c.getClickPos().getX());
        },
    });
    const listener = new Listener;
    vectorTileLayer.setVectorTileEventListener(listener);
}

function addPlane(localDataSource) {
    const modelPath = 'norwegian_737_800.nml';
    fetch('assets/' + modelPath).then(response => response.arrayBuffer()).then(data => {
        CartoMobileSDK.FS.writeFile('/assets/'+modelPath, new Int8Array(data));
    }).then(() => {
        const modelStyleBuilder = new CartoMobileSDK.NMLModelStyleBuilder();
        modelStyleBuilder.setModelAsset(CartoMobileSDK.AssetUtils.LoadAsset(modelPath));

        const pos = new CartoMobileSDK.MapPos(3649495, 4852002, 100);
        const model = new CartoMobileSDK.NMLModel(pos, modelStyleBuilder.buildStyle());
        model.setScale(10);
        localDataSource.add(model);
        setTimeout(() => animate(model, -8237016, 4969476, 100, -8239763, 4971937, 6000), 2000);
        var audio = new Audio('assets/plane.wav');
        setTimeout(() => audio.play(), 4000);
    })
}

function animate(model, posX, posY, posZ, pos2X, pos2Y, timeInMs) {
    let id;
    let startTime = 0;
    let func;
    func = (timestamp) => {
        if (!startTime) {
            startTime = timestamp;
            requestAnimationFrame(func);
            return;
        }
        let totalTime = timestamp - startTime;
        if (totalTime > timeInMs) totalTime = timeInMs;

        const x = (pos2X - posX) * (totalTime / timeInMs) + posX;
        const y = (pos2Y - posY) * (totalTime / timeInMs) + posY;

        model.setPos(new CartoMobileSDK.MapPos(x, y, posZ));

        if (totalTime < timeInMs) {
            requestAnimationFrame(func);
        }
    };

    requestAnimationFrame(func);
}