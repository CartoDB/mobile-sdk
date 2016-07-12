#ifndef _OPTIONS_I
#define _OPTIONS_I

#pragma SWIG nowarn=325
#pragma SWIG nowarn=401

%module Options

!proxy_imports(carto::Options, core.MapBounds, core.MapRange, core.MapVec, core.ScreenPos, graphics.Bitmap, graphics.Color, projections.Projection)

%{
#include "components/Options.h"
#include "components/Exceptions.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <cartoswig.i>

!shared_ptr(carto::Options, components.Options)

%import "core/MapBounds.i"
%import "core/MapRange.i"
%import "core/MapVec.i"
%import "core/ScreenPos.i"
%import "graphics/Bitmap.i"
%import "graphics/Color.i"
%import "projections/Projection.i"

%attribute(carto::Options, int, FieldOfViewY, getFieldOfViewY, setFieldOfViewY)
%attribute(carto::Options, bool, KineticZoom, isKineticZoom, setKineticZoom)
%attribute(carto::Options, bool, Rotatable, isRotatable, setRotatable)
%attribute(carto::Options, bool, UserInput, isUserInput, setUserInput)
%attribute(carto::Options, bool, ClickTypeDetection, isClickTypeDetection, setClickTypeDetection)
%attribute(carto::Options, bool, KineticPan, isKineticPan, setKineticPan)
%attribute(carto::Options, bool, KineticRotation, isKineticRotation, setKineticRotation)
%attribute(carto::Options, bool, SeamlessPanning, isSeamlessPanning, setSeamlessPanning)
%attribute(carto::Options, bool, TiltGestureReversed, isTiltGestureReversed, setTiltGestureReversed)
%attributeval(carto::Options, carto::MapRange, ZoomRange, getZoomRange, setZoomRange)
%attributeval(carto::Options, carto::MapRange, TiltRange, getTiltRange, setTiltRange)
%attributeval(carto::Options, carto::MapBounds, PanBounds, getPanBounds, setPanBounds)
%attributeval(carto::Options, carto::ScreenPos, FocusPointOffset, getFocusPointOffset, setFocusPointOffset)
%attributeval(carto::Options, carto::Color, AmbientLightColor, getAmbientLightColor, setAmbientLightColor)
%attributeval(carto::Options, carto::Color, MainLightColor, getMainLightColor, setMainLightColor)
%attributeval(carto::Options, carto::MapVec, MainLightDirection, getMainLightDirection, setMainLightDirection)
!attributestring_polymorphic(carto::Options, projections.Projection, BaseProjection, getBaseProjection, setBaseProjection)
%attribute(carto::Options, carto::ProjectionMode::ProjectionMode, ProjectionMode, getProjectionMode, setProjectionMode)
%attribute(carto::Options, carto::PanningMode::PanningMode, PanningMode, getPanningMode, setPanningMode)
%attribute(carto::Options, carto::PivotMode::PivotMode, PivotMode, getPivotMode, setPivotMode)
%attributestring(carto::Options, std::shared_ptr<carto::Bitmap>, BackgroundBitmap, getBackgroundBitmap, setBackgroundBitmap)
%attributestring(carto::Options, std::shared_ptr<carto::Bitmap>, SkyBitmap, getSkyBitmap, setSkyBitmap)
%attribute(carto::Options, int, EnvelopeThreadPoolSize, getEnvelopeThreadPoolSize, setEnvelopeThreadPoolSize)
%attribute(carto::Options, int, TileThreadPoolSize, getTileThreadPoolSize, setTileThreadPoolSize)
%attribute(carto::Options, int, TileDrawSize, getTileDrawSize, setTileDrawSize)
%attribute(carto::Options, float, DPI, getDPI, setDPI)
%attribute(carto::Options, float, DrawDistance, getDrawDistance, setDrawDistance)
%attributestring(carto::Options, std::shared_ptr<carto::Bitmap>, WatermarkBitmap, getWatermarkBitmap, setWatermarkBitmap)
%attribute(carto::Options, float, WatermarkAlignmentX, getWatermarkAlignmentX, setWatermarkAlignmentX)
%attribute(carto::Options, float, WatermarkAlignmentY, getWatermarkAlignmentY, setWatermarkAlignmentY)
%attribute(carto::Options, float, WatermarkScale, getWatermarkScale, setWatermarkScale)
%attributeval(carto::Options, carto::ScreenPos, WatermarkPadding, getWatermarkPadding, setWatermarkPadding)
%std_exceptions(carto::Options::setBaseProjection)
!objc_rename(setWatermarkAnchorX) carto::Options::setWatermarkAnchor;
!objc_rename(setWatermarkPaddingX) carto::Options::setWatermarkPadding;
%ignore carto::Options::Options;
%ignore carto::Options::getInternalPanBounds;
%ignore carto::Options::OnChangeListener;
%ignore carto::Options::registerOnChangeListener;
%ignore carto::Options::unregisterOnChangeListener;
!standard_equals(carto::Options);

%include "components/Options.h"

#endif
