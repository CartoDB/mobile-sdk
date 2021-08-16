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

!enum(carto::RenderProjectionMode::RenderProjectionMode)
!enum(carto::PanningMode::PanningMode)
!enum(carto::PivotMode::PivotMode)
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
%attribute(carto::Options, bool, DoubleClickDetection, isDoubleClickDetection, setDoubleClickDetection)
%attribute(carto::Options, float, LongClickDuration, getLongClickDuration, setLongClickDuration)
%attribute(carto::Options, float, DoubleClickMaxDuration, getDoubleClickMaxDuration, setDoubleClickMaxDuration)
%attribute(carto::Options, bool, KineticPan, isKineticPan, setKineticPan)
%attribute(carto::Options, bool, KineticRotation, isKineticRotation, setKineticRotation)
%attribute(carto::Options, bool, SeamlessPanning, isSeamlessPanning, setSeamlessPanning)
%attribute(carto::Options, bool, RestrictedPanning, isRestrictedPanning, setRestrictedPanning)
%attribute(carto::Options, bool, TiltGestureReversed, isTiltGestureReversed, setTiltGestureReversed)
%attribute(carto::Options, bool, ZoomGestures, isZoomGestures, setZoomGestures)
%attributeval(carto::Options, carto::MapRange, ZoomRange, getZoomRange, setZoomRange)
%attributeval(carto::Options, carto::MapRange, TiltRange, getTiltRange, setTiltRange)
%attributeval(carto::Options, carto::MapBounds, PanBounds, getPanBounds, setPanBounds)
%attributeval(carto::Options, carto::ScreenPos, FocusPointOffset, getFocusPointOffset, setFocusPointOffset)
%attributeval(carto::Options, carto::Color, AmbientLightColor, getAmbientLightColor, setAmbientLightColor)
%attributeval(carto::Options, carto::Color, MainLightColor, getMainLightColor, setMainLightColor)
%attributeval(carto::Options, carto::MapVec, MainLightDirection, getMainLightDirection, setMainLightDirection)
!attributestring_polymorphic(carto::Options, projections.Projection, BaseProjection, getBaseProjection, setBaseProjection)
%attribute(carto::Options, carto::RenderProjectionMode::RenderProjectionMode, RenderProjectionMode, getRenderProjectionMode, setRenderProjectionMode)
%attribute(carto::Options, carto::PanningMode::PanningMode, PanningMode, getPanningMode, setPanningMode)
%attribute(carto::Options, carto::PivotMode::PivotMode, PivotMode, getPivotMode, setPivotMode)
%attributeval(carto::Options, carto::Color, ClearColor, getClearColor, setClearColor)
%attributeval(carto::Options, carto::Color, SkyColor, getSkyColor, setSkyColor)
%attributestring(carto::Options, std::shared_ptr<carto::Bitmap>, BackgroundBitmap, getBackgroundBitmap, setBackgroundBitmap)
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
%std_exceptions(carto::Options::setTiltRange)
%std_exceptions(carto::Options::setZoomRange)
%std_exceptions(carto::Options::setPanBounds)
!objc_rename(setWatermarkPadding) carto::Options::setWatermarkPadding;
%ignore carto::Options::Options;
%ignore carto::Options::getProjectionSurface;
%ignore carto::Options::getSkyBitmap;
%ignore carto::Options::getAdjustedInternalPanBounds;
%ignore carto::Options::OnChangeListener;
%ignore carto::Options::registerOnChangeListener;
%ignore carto::Options::unregisterOnChangeListener;
%ignore carto::Options::GetDefaultBackgroundBitmap;
%ignore carto::Options::GetCartoWatermarkBitmap;
%ignore carto::Options::GetEvaluationWatermarkBitmap;
%ignore carto::Options::GetExpiredWatermarkBitmap;
!standard_equals(carto::Options);

%include "components/Options.h"

#endif
