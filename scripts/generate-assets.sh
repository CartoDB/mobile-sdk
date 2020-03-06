#!/bin/bash

cd ../assets
xxd -i default-background.png > ../all/native/assets/DefaultBackgroundPNG.h
xxd -i default-line.png > ../all/native/assets/DefaultLinePNG.h
xxd -i default-marker.png > ../all/native/assets/DefaultMarkerPNG.h
xxd -i default-point.png > ../all/native/assets/DefaultPointPNG.h
xxd -i default-polygon.png > ../all/native/assets/DefaultPolygonPNG.h
xxd -i default-nmlmodel.nml > ../all/native/assets/DefaultNMLModelNML.h
xxd -i evaluation-watermark.png | sed 's/unsigned/static unsigned/' > ../all/native/assets/EvaluationWatermarkPNG.h
xxd -i expired-watermark.png | sed 's/unsigned/static unsigned/' > ../all/native/assets/ExpiredWatermarkPNG.h
xxd -i carto-watermark.png | sed 's/unsigned/static unsigned/' > ../all/native/assets/CartoWatermarkPNG.h
xxd -i cartostyles-v2.zip | sed 's/unsigned/static unsigned/' > ../all/native/assets/CartoStylesV2ZIP.h
