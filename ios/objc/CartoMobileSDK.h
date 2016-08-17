/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTOMOBILESDK_H_
#define _CARTOMOBILESDK_H_

#import "NTOptions.h"
#import "NTLayers.h"

#import "NTMapBounds.h"
#import "NTMapEnvelope.h"
#import "NTMapPos.h"
#import "NTScreenPos.h"
#import "NTScreenBounds.h"
#import "NTMapRange.h"
#import "NTMapTile.h"
#import "NTMapVec.h"
#import "NTTileData.h"

#import "NTAssetTileDataSource.h"
#import "NTBitmapOverlayRasterTileDataSource.h"
#import "NTHTTPTileDataSource.h"
#import "NTMemoryCacheTileDataSource.h"
#import "NTPersistentCacheTileDataSource.h"
#import "NTLocalVectorDataSource.h"

#import "NTFeature.h"
#import "NTFeatureCollection.h"
#import "NTLineGeometry.h"
#import "NTPointGeometry.h"
#import "NTPolygonGeometry.h"
#import "NTMultiGeometry.h"
#import "NTMultiLineGeometry.h"
#import "NTMultiPointGeometry.h"
#import "NTMultiPolygonGeometry.h"
#import "NTGeometrySimplifier.h"
#import "NTDouglasPeuckerGeometrySimplifier.h"
#import "NTGeoJSONGeometryReader.h"
#import "NTGeoJSONGeometryWriter.h"

#import "NTFrustum.h"
#import "NTBitmap.h"
#import "NTColor.h"
#import "NTViewState.h"

#import "NTSolidLayer.h"
#import "NTRasterTileLayer.h"
#import "NTTileLoadListener.h"
#import "NTUTFGridEventListener.h"
#import "NTVectorElementEventListener.h"
#import "NTVectorLayer.h"
#import "NTVectorTileLayer.h"
#import "NTTorqueTileLayer.h"
#import "NTClusteredVectorLayer.h"
#import "NTClusterElementBuilder.h"

#import "NTEPSG3857.h"

#import "NTCullState.h"

#import "NTBalloonPopupStyleBuilder.h"
#import "NTBalloonPopupStyle.h"
#import "NTLabelStyleBuilder.h"
#import "NTLabelStyle.h"
#import "NTLineStyleBuilder.h"
#import "NTLineStyle.h"
#import "NTMarkerStyleBuilder.h"
#import "NTMarkerStyle.h"
#import "NTPointStyleBuilder.h"
#import "NTPointStyle.h"
#import "NTPolygon3DStyleBuilder.h"
#import "NTPolygon3DStyle.h"
#import "NTPolygonStyleBuilder.h"
#import "NTPolygonStyle.h"
#import "NTPopupStyleBuilder.h"
#import "NTPopupStyle.h"
#import "NTTextStyleBuilder.h"
#import "NTTextStyle.h"
#import "NTGeometryCollectionStyle.h"
#import "NTGeometryCollectionStyleBuilder.h"

#import "NTMapRenderer.h"
#import "NTMapRendererListener.h"
#import "NTRendererCaptureListener.h"

#import "ui/MapView.h"
#import "NTMapClickInfo.h"
#import "NTMapEventListener.h"
#import "NTVectorElementClickInfo.h"

#import "NTAssetUtils.h"
#import "NTBitmapUtils.h"
#import "NTLog.h"

#import "NTBalloonPopup.h"
#import "NTCustomPopup.h"
#import "NTCustomPopupHandler.h"
#import "NTGeometryCollection.h"
#import "NTLabel.h"
#import "NTLine.h"
#import "NTMarker.h"
#import "NTNMLModel.h"
#import "NTPoint.h"
#import "NTPolygon3D.h"
#import "NTPolygon.h"
#import "NTPopup.h"
#import "NTText.h"

#import "NTAssetPackage.h"
#import "NTZippedAssetPackage.h"
#import "NTCompiledStyleSet.h"
#import "NTCartoCSSStyleSet.h"
#import "NTVectorTileDecoder.h"
#import "NTMBVectorTileDecoder.h"
#import "NTTorqueTileDecoder.h"

#import "NTCartoMapsService.h"
#import "NTCartoSQLService.h"
#import "NTCartoVisLoader.h"
#import "NTCartoVisBuilder.h"

#ifdef _CARTO_LICENSEMANAGER_SUPPORT
#import "NTCartoOnlineTileDataSource.h"
#import "NTCartoOnlineRasterTileLayer.h"
#import "NTCartoOnlineVectorTileLayer.h"
#import "NTCartoOnlineRoutingService.h"
#endif

#ifdef _CARTO_OFFLINE_SUPPORT
#import "NTMBTilesTileDataSource.h"
#endif

#ifdef _CARTO_PACKAGEMANAGER_SUPPORT
#import "NTPackageManagerTileDataSource.h"
#import "NTPackageManagerRoutingService.h"

#import "NTPackageInfo.h"
#import "NTPackageStatus.h"
#import "NTPackageTileMask.h"
#import "NTPackageManager.h"
#import "NTCartoPackageManager.h"
#endif

#ifdef _CARTO_ROUTING_SUPPORT
#import "NTRoutingInstruction.h"
#import "NTRoutingRequest.h"
#import "NTRoutingResult.h"
#import "NTRoutingService.h"
#import "NTOSRMOfflineRoutingService.h"
#endif

#ifdef _CARTO_NMLMODELLODTREE_SUPPORT
#import "NTOnlineNMLModelLODTreeDataSource.h"
#import "NTSqliteNMLModelLODTreeDataSource.h"
#import "NTNMLModelLODTreeLayer.h"
#endif

#ifdef _CARTO_EDITABLE_SUPPORT
#import "NTEditableVectorLayer.h"
#import "NTVectorEditEventListener.h"
#endif

#ifdef _CARTO_WKBT_SUPPORT
#import "NTWKTGeometryReader.h"
#import "NTWKTGeometryWriter.h"
#import "NTWKBGeometryReader.h"
#import "NTWKBGeometryWriter.h"
#endif

#ifdef _CARTO_GDAL_SUPPORT
#import "NTOGRVectorDataSource.h"
#import "NTGDALRasterTileDataSource.h"
#import "NTStyleSelector.h"
#import "NTStyleSelectorBuilder.h"
#endif

#endif
 