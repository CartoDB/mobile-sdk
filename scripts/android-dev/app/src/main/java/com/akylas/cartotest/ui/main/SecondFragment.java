package com.akylas.cartotest.ui.main;

import android.animation.Animator;
import android.os.Bundle;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.SeekBar;
import android.widget.TextView;
import android.widget.Toast;

import com.akylas.cartotest.R;
import com.carto.components.Options;
import com.carto.components.PanningMode;
import com.carto.components.RenderProjectionMode;
import com.carto.core.IntVector;
import com.carto.core.DoubleVector;
import com.carto.core.MapPos;
import com.carto.core.StringVector;
import com.carto.datasources.HTTPTileDataSource;
import com.carto.graphics.Color;
import com.carto.layers.CartoBaseMapStyle;
import com.carto.layers.CartoOnlineRasterTileLayer;
import com.carto.layers.CartoOnlineVectorTileLayer;
import com.carto.layers.HillshadeRasterTileLayer;
import com.carto.layers.RasterTileLayer;
import com.carto.projections.EPSG4326;
import com.carto.projections.Projection;
import com.carto.ui.MapEventListener;
import com.carto.ui.MapView;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.appcompat.widget.AppCompatSeekBar;
import androidx.fragment.app.Fragment;
import androidx.lifecycle.ViewModelProviders;

public class SecondFragment extends Fragment {
    private final String TAG = "SecondFragment";

    public static SecondFragment newInstance() {
        return new SecondFragment();
    }

    private static final int REQUEST_PERMISSIONS_CODE_WRITE_STORAGE = 1435;
    MapView mapView;
    TileDataSource hillshadeSource;
    MapBoxElevationDataDecoder elevationDecoder;

    // Function to check and request permission
    @SuppressLint("NewApi")
    public void checkStoragePermission(View view) {

        // Checking if permission is not granted
        if (getActivity().checkSelfPermission(Manifest.permission.WRITE_EXTERNAL_STORAGE) == PackageManager.PERMISSION_DENIED) {
            requestPermissions(
                    new String[]{Manifest.permission.WRITE_EXTERNAL_STORAGE},
                    REQUEST_PERMISSIONS_CODE_WRITE_STORAGE);
        } else {
            proceedWithSdCard(view);
        }
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions, @NonNull int[] grantResults) {
        if (requestCode == REQUEST_PERMISSIONS_CODE_WRITE_STORAGE) {
            if (permissions[0].equals(Manifest.permission.WRITE_EXTERNAL_STORAGE)
                    && grantResults[0] == PackageManager.PERMISSION_GRANTED) {
                proceedWithSdCard(this.getView());
            }
        }
    }

    void proceedWithSdCard(View view) {
        //TileDataSource hillshadeSource = null;
        //try {
        //    hillshadeSource = this.hillshadeSource = new MBTilesTileDataSource("/storage/100F-3415/alpimaps_mbtiles/BDALTIV2_75M_rvb.etiles");
        //} catch (IOException e) {
        //    e.printStackTrace();
        //}
        HTTPTileDataSource hillshadeSource = hillshadeSource = new HTTPTileDataSource(5, 11, "http://192.168.1.158:8080/data/BDALTIV2_75M_rvb/{z}/{x}/{y}.webp");
        final MapBoxElevationDataDecoder decoder = elevationDecoder = new MapBoxElevationDataDecoder();
        final HillshadeRasterTileLayer layer = new HillshadeRasterTileLayer(hillshadeSource, decoder);
        layer.setContrast(0.46f);
        layer.setHeightScale(0.23f);
        layer.setVisibleZoomRange(new MapRange(5, 12));

        layer.setIlluminationDirection(207);
        layer.setHighlightColor(new Color((short) 141, (short) 141, (short) 141, (short) 255));

//        final CheckBox inspectCheckBox = (CheckBox) view.findViewById(R.id.inspectCheckBox); // initiate the Seek bar
//        inspectCheckBox.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
//            @Override
//            public void onCheckedChanged(CompoundButton compoundButton, boolean b) {
//                layer.setInspect(b);
//                mapView.requestRender();
//            }
//        });
        final AppCompatSeekBar contrastSeekBar = (AppCompatSeekBar) view.findViewById(R.id.contrastSeekBar); // initiate the Seek bar
        final TextView textContrast = (TextView) view.findViewById(R.id.textContrast); // initiate the Seek bar
        contrastSeekBar.setProgress((int) (layer.getContrast() * 100.0f));
        textContrast.setText(layer.getContrast() + "");
        contrastSeekBar.setOnSeekBarChangeListener(new AppCompatSeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int i, boolean b) {

                layer.setContrast(i / 100.0f);
                textContrast.setText(layer.getContrast() + "");
                mapView.requestRender();
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {
            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
            }

        });
        final AppCompatSeekBar illuminationDirectionSeekBar = (AppCompatSeekBar) view.findViewById(R.id.illuminationDirectionSeekBar); // initiate the Seek bar
        final TextView textIlluminationDirection = (TextView) view.findViewById(R.id.textIlluminationDirection); // initiate the Seek bar
        illuminationDirectionSeekBar.setProgress((int) layer.getIlluminationDirection());
        textIlluminationDirection.setText(layer.getIlluminationDirection() + "");
        illuminationDirectionSeekBar.setOnSeekBarChangeListener(new AppCompatSeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int i, boolean b) {

                layer.setIlluminationDirection(i);
                textIlluminationDirection.setText(layer.getIlluminationDirection() + "");
                mapView.requestRender();
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {
            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
            }

        });


        final AppCompatSeekBar heightScaleSeekBar = (AppCompatSeekBar) view.findViewById(R.id.heightScaleSeekBar); // initiate the Seek bar
        final TextView textHeightScale = (TextView) view.findViewById(R.id.textHeightScale); // initiate the Seek bar
        heightScaleSeekBar.setProgress((int) (layer.getHeightScale() * 100.0f));
        textHeightScale.setText(layer.getHeightScale() + "");
        heightScaleSeekBar.setOnSeekBarChangeListener(new AppCompatSeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int i, boolean b) {

                layer.setHeightScale(i / 100.0f);
                textHeightScale.setText(layer.getHeightScale() + "");
                mapView.requestRender();
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {
            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
            }

        });


//        Color highlightColor = new Color((short)255, (short)255,(short)255, (short)255);
        final AppCompatSeekBar highlightOpacitySeekBar = (AppCompatSeekBar) view.findViewById(R.id.highlightOpacitySeekBar); // initiate the Seek bar
        final TextView textHighlightOpacity = (TextView) view.findViewById(R.id.testHighlightOpacity); // initiate the Seek bar
        highlightOpacitySeekBar.setProgress(255);
        textHighlightOpacity.setText("255");
        highlightOpacitySeekBar.setOnSeekBarChangeListener(new AppCompatSeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int i, boolean b) {
                Color highlightColor = new Color((short) i, (short) i, (short) i, (short) 255);
                layer.setHighlightColor(highlightColor);
                textHighlightOpacity.setText(i + "");
                mapView.requestRender();
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {
            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
            }

        });

//        layer.setZoomLevelBias(1);
//        layer.setOpacity(0.5f);
        mapView.getLayers().add(layer);

        final TileDataSource finalHillshadeSource = hillshadeSource;
        final TextView textZoom = (TextView) view.findViewById(R.id.zoomText); // initiate the Seek bar
        mapView.setMapEventListener(new MapEventListener() {
            @Override
            public void onMapMoved() {
                super.onMapMoved();
                Log.d(TAG, "onMapMoved " + mapView.getFocusPos());
                textZoom.setText(String.format("%.2f", mapView.getZoom()));
            }

            @Override
            public void onMapClicked(MapClickInfo mapClickInfo) {
                super.onMapClicked(mapClickInfo);
                MapPos clickPos = mapClickInfo.getClickPos();
                Log.d(TAG, "onMapClicked " + clickPos);
//                Log.d(TAG, "test elevation1 " + elevationDecoder.getElevation(finalHillshadeSource,new MapPos(5.754059708520897,45.216144987257785)));
                Log.d(TAG, "elevation " + layer.getElevation(clickPos));

//                Log.d(TAG, "test elevation " + elevationDecoder.getElevation(finalHillshadeSource,new MapPos(5.753729544068004,45.216280596342095)));
//                Log.d(TAG, "test elevation2 " + elevationDecoder.getElevation(finalHillshadeSource,new MapPos(5.753351, 45.202453)));

            }
        });
        // trying
//        double[] points = {44.9404296875,6.191139221191406,44.94038772583008,6.191171169281006,44.94017791748047,6.191186904907227,44.940086364746094,6.191176891326904,44.93997573852539,6.191236972808838,44.93993377685547,6.191226005554199,44.93975830078125,6.191294193267822,44.93962097167969,6.191326141357422,44.93955612182617,6.191355228424072,44.939453125,6.191371917724609,44.93930435180664,6.191524028778076,44.93928146362305,6.191608905792236,44.93923568725586,6.191670894622803,44.939178466796875,6.191803932189941,44.93910217285156,6.191903114318848,44.93891143798828,6.192008018493652,44.9388427734375,6.192142963409424,44.93875503540039,6.192234039306641,44.938663482666016,6.192309856414795,44.93854904174805,6.19241189956665,44.938446044921875,6.192460060119629,44.93836212158203,6.192471027374268,44.93815231323242,6.1925530433654785,44.93794250488281,6.192533016204834,44.93782424926758,6.1926589012146,44.937740325927734,6.1928019523620605,44.93763732910156,6.19287109375,44.937564849853516,6.192831993103027,44.93754196166992,6.19290018081665,44.937496185302734,6.192921161651611,44.93749237060547,6.192957878112793,44.93750762939453,6.193059921264648,44.937469482421875,6.193058013916016,44.93745040893555,6.193091869354248,44.937461853027344,6.193151950836182,44.93742752075195,6.193192005157471,44.9373893737793,6.193164825439453,44.937416076660156,6.193043231964111,44.93745422363281,6.192934989929199,44.93741989135742,6.1929030418396,44.937355041503906,6.1929931640625,44.93726348876953,6.192963123321533,44.93717956542969,6.192934036254883,44.93714904785156,6.192899227142334,44.93708419799805,6.192975044250488,44.937034606933594,6.192798137664795,44.93703842163086,6.192720890045166,44.93706130981445,6.19260311126709,44.9371337890625,6.1925048828125,44.93704605102539,6.192429065704346};
//        MapPosVector poses = new MapPosVector();
//        for (int i = 0; i < points.length; i+=2) {
//            poses.add(new MapPos(points[i+1], points[i]));
//        }

        final Options options = mapView.getOptions();

        final Button modeButton = (Button) view.findViewById(R.id.modeButton); // initiate the Seek bar
        modeButton.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
                // Code here executes on main thread after user presses button
                if (options.getRenderProjectionMode() == RenderProjectionMode.RENDER_PROJECTION_MODE_SPHERICAL) {
                    options.setRenderProjectionMode(RenderProjectionMode.RENDER_PROJECTION_MODE_PLANAR);
                } else {
                    options.setRenderProjectionMode(RenderProjectionMode.RENDER_PROJECTION_MODE_SPHERICAL);
                }
            }
        });

        Projection projection = options.getBaseProjection();
        ValhallaOfflineRoutingService routingService;
        try {
            routingService = new ValhallaOfflineRoutingService("/storage/100F-3415/alpimaps_mbtiles/france.vtiles");
//            routingService.connectElevationDataSource(hillshadeSource, elevationDecoder);
            routingService.setProfile("bicycle");

            LocalVectorDataSource localSource = new LocalVectorDataSource(projection);
            VectorLayer vectorLayer = new VectorLayer(localSource);
            mapView.getLayers().add(vectorLayer);
            MapPosVector vector = new MapPosVector();
            vector.add(new MapPos(5.729, 45.192));
            vector.add(new MapPos(6.675, 45.506));
            RoutingRequest request = new RoutingRequest(projection, vector);
            VariantObjectBuilder variantbuilder = new VariantObjectBuilder();
            VariantObjectBuilder variantbuilder2 = new VariantObjectBuilder();
            variantbuilder2.setLong("use_hills", 0);
            variantbuilder.setVariant("bicycle", variantbuilder2.buildVariant());
            request.setCustomParameter("costing_options", variantbuilder.buildVariant());
            RoutingResult result = routingService.calculateRoute(request);
            MapPosVector pointsWithAltitude = new MapPosVector();
            if (result != null) {
                MapPosVector points = result.getPoints();
                Log.d(TAG, "showing route " + points.size());
                DoubleVector elevations = layer.getElevations(points);
                for (int i = (int) (elevations.size() - 1); i >= 0; i--) {
                    MapPos point = points.get(i);
                    Log.d(TAG, "elevations2 " + i + "  " + point.getX() + "  " +  point.getY() + "  " +  elevations.get(i));
//                    int computedElevation = layer.getElevation(point);
                    pointsWithAltitude.add(new MapPos(point.getX(), point.getY(), elevations.get(i)));
                }
                LineStyleBuilder builder = new LineStyleBuilder();
                builder.setWidth(4);
                builder.setColor(new Color((short) 255, (short) 0, (short) 0, (short) 255));
                Line line = new Line(pointsWithAltitude, builder.buildStyle());
                localSource.add(line);
            }
//            Log.d(TAG, "elevations " + elevations.toString());


        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    @Nullable
    @Override
    public View onCreateView(@NonNull LayoutInflater inflater, @Nullable ViewGroup container,
                             @Nullable Bundle savedInstanceState) {
        Log.d(TAG, "onCreateView");

        View view = inflater.inflate(R.layout.second_fragment, container, false);


        final MapView mapView = (MapView) view.findViewById(R.id.mapView);
        MapView.registerLicense("XTUMwQ0ZRQ0RLZEM4Z1dMdkc1TDZkZy83RlN3Z0V2aTB5d0lVSlEwbGZNZjV5bDJLMnlPWXFJYWpVWmhuQWtZPQoKYXBwVG9rZW49OWYwZjBhMDgtZGQ1Mi00NjVkLTg5N2YtMTg0MDYzODQxMDBiCnBhY2thZ2VOYW1lPWNvbS5ha3lsYXMuY2FydG90ZXN0Cm9ubGluZUxpY2Vuc2U9MQpwcm9kdWN0cz1zZGstYW5kcm9pZC00LioKd2F0ZXJtYXJrPWNhcnRvZGIK", this.getContext());
//        HTTPTileDataSource source =  new HTTPTileDataSource(1, 20, "http://{s}.tile.openstreetmap.fr/osmfr/{z}/{x}/{y}.png");
        final TextView textZoom = (TextView) view.findViewById(R.id.zoomText); // initiate the Seek bar

        mapView.setMapEventListener(new MapEventListener(){
            @Override
            public void onMapMoved() {
                super.onMapMoved();
                Log.d(TAG, "onMapMoved " + mapView.getFocusPos());
                textZoom.setText(String.format("%.2f", mapView.getZoom()));
            }
        });




//        HTTPTileDataSource source = new HTTPTileDataSource(1, 20, "https://1.base.maps.cit.api.here.com/maptile/2.1/maptile/newest/normal.day.grey/{z}/{x}/{y}/512/png8?app_id=9QKPJz6sIj9MkeeUmpfc&app_code=iD7QuqOFDMJS_nNtlKdp1A");
//        StringVector subdomains = new StringVector();
//        subdomains.add("a");
//        subdomains.add("b");
//        subdomains.add("c");
//        source.setSubdomains(subdomains);
//        final RasterTileLayer rasterlayer = new RasterTileLayer(source);
        final CartoOnlineVectorTileLayer rasterlayer = new CartoOnlineVectorTileLayer(CartoBaseMapStyle.CARTO_BASEMAP_STYLE_VOYAGER);

        final AppCompatSeekBar opacitySeekBar = (AppCompatSeekBar) view.findViewById(R.id.opacitySeekBar); // initiate the Seek bar
        final TextView textOpacity = (TextView) view.findViewById(R.id.textOpacity); // initiate the Seek bar
        opacitySeekBar.setProgress(100);
        textOpacity.setText(rasterlayer.getOpacity() + "");
        opacitySeekBar.setOnSeekBarChangeListener(new AppCompatSeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int i, boolean b) {
                rasterlayer.setOpacity(i / 100.0f);
                textOpacity.setText(rasterlayer.getOpacity() + "");
                mapView.requestRender();
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {
            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
            }

        });


        HTTPTileDataSource source2 =  new HTTPTileDataSource(6, 12, "http://192.168.1.45:8080/data/BDALTIV2_75M_rvb/{z}/{x}/{y}.webp");
//        HTTPTileDataSource source2 = new HTTPTileDataSource(1, 15, "https://api.mapbox.com/v4/mapbox.terrain-rgb/{z}/{x}/{y}.pngraw?access_token=pk.eyJ1IjoiYWt5bGFzIiwiYSI6IkVJVFl2OXMifQ.TGtrEmByO3-99hA0EI44Ew");
        final HillshadeRasterTileLayer layer = new HillshadeRasterTileLayer(source2);
//        layer.setHighlightColor(new Color((short)0, (short)137,(short)36, (short)255));

//        final CheckBox inspectCheckBox = (CheckBox) view.findViewById(R.id.inspectCheckBox); // initiate the Seek bar
//        inspectCheckBox.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
//            @Override
//            public void onCheckedChanged(CompoundButton compoundButton, boolean b) {
//                layer.setInspect(b);
//                mapView.requestRender();
//            }
//        });
        final  AppCompatSeekBar exagerationSeekBar = (AppCompatSeekBar) view.findViewById(R.id.exagerationSeekBar); // initiate the Seek bar
        final TextView textExaggeration = (TextView) view.findViewById(R.id.textExaggeration); // initiate the Seek bar
        layer.setContrast(0.75f);
        exagerationSeekBar.setProgress(75);
        textExaggeration.setText(layer.getContrast() + "");
        exagerationSeekBar.setOnSeekBarChangeListener(new AppCompatSeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int i, boolean b) {

                layer.setContrast(i / 100.0f);
                textExaggeration.setText(layer.getContrast() + "");
                mapView.requestRender();
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {
            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
            }

        });


        final  AppCompatSeekBar illuminationDirectionSeekBar = (AppCompatSeekBar) view.findViewById(R.id.illuminationDirectionSeekBar); // initiate the Seek bar
        final TextView textIlluminationDirection = (TextView) view.findViewById(R.id.textIlluminationDirection); // initiate the Seek bar
        layer.setHeightScale(0.28f);
        illuminationDirectionSeekBar.setProgress(28);
        textIlluminationDirection.setText(layer.getHeightScale() + "");
        illuminationDirectionSeekBar.setOnSeekBarChangeListener(new AppCompatSeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int i, boolean b) {

                layer.setHeightScale(i / 100.0f);
                textIlluminationDirection.setText(layer.getHeightScale() + "");
                mapView.requestRender();
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {
            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
            }

        });
        layer.setZoomLevelBias(1);
//        layer.setOpacity(0.5f);

        final Options options = mapView.getOptions();

        options.setZoomGestures(true);
        options.setRestrictedPanning(true);
        options.setSeamlessPanning(true);
        options.setRotatable(true);
        options.setRenderProjectionMode(RenderProjectionMode.RENDER_PROJECTION_MODE_SPHERICAL);
        options.setPanningMode(PanningMode.PANNING_MODE_STICKY);
        options.setBaseProjection(new EPSG4326());
        mapView.getLayers().add(rasterlayer);
        mapView.getLayers().add(layer);
        mapView.setFocusPos(new MapPos(6.151881, 45.142932), 0);
        mapView.setZoom(11, 0);
        com.carto.utils.Log.setShowInfo(true);
        com.carto.utils.Log.setShowDebug(true);
        com.carto.utils.Log.setShowWarn(true);
        com.carto.utils.Log.setShowError(true);
        return view;
    }

    @Override
    public void onActivityCreated(@Nullable Bundle savedInstanceState) {
        Log.d(TAG, "onActivityCreated");
        super.onActivityCreated(savedInstanceState);
    }

    @Override
    public void onDestroyView() {
        Log.d(TAG, "onDestroyView");
        super.onDestroyView();
    }

    @Override
    public void onDestroy() {
        Log.d(TAG, "onDestroy");
        super.onDestroy();
    }

    @Override
    public void onPause() {
        Log.d(TAG, "onPause");
        super.onPause();
    }

    @Override
    public void onStop() {
        Log.d(TAG, "onStop");
        super.onStop();
    }

    @Override
    public Animator onCreateAnimator(int transit, boolean enter, int nextAnim) {
        Animator result = super.onCreateAnimator(transit, enter, nextAnim);
        Log.d(TAG, "onCreateAnimator " + result);
        return result;
    }

}
