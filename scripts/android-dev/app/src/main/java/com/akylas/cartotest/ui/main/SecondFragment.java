package com.akylas.cartotest.ui.main;

import android.animation.Animator;
import android.os.Bundle;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.SeekBar;
import android.widget.TextView;
import android.widget.Toast;

import com.akylas.cartotest.R;
import com.carto.components.Options;
import com.carto.components.PanningMode;
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

        Options options = mapView.getOptions();
        options.setZoomGestures(true);
        options.setWatermarkScale(0);
        options.setRestrictedPanning(true);
        options.setSeamlessPanning(true);
        options.setEnvelopeThreadPoolSize(2);
        options.setTileThreadPoolSize(2);
        options.setRotatable(true);
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
