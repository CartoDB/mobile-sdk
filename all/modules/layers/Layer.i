#ifndef _LAYER_I
#define _LAYER_I

#pragma SWIG nowarn=401

%module Layer

!proxy_imports(carto::Layer, core.MapRange, core.ScreenPos, core.Variant, core.StringVariantMap, graphics.ViewState, renderers.components.CullState, ui.ClickInfo)
!java_imports(carto::Layer, com.carto.ui.ClickType)

%{
#include "layers/Layer.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <cartoswig.i>

%import "core/MapRange.i"
%import "core/ScreenPos.i"
%import "core/Variant.i"
%import "graphics/ViewState.i"
%import "renderers/components/CullState.i"
%import "ui/ClickInfo.i"

!polymorphic_shared_ptr(carto::Layer, layers.Layer)
!value_type(std::vector<std::shared_ptr<carto::Layer> >, layers.LayerVector)

%attributeval(carto::Layer, %arg(std::map<std::string, carto::Variant>), MetaData, getMetaData, setMetaData)
%attribute(carto::Layer, int, UpdatePriority, getUpdatePriority, setUpdatePriority)
%attribute(carto::Layer, int, CullDelay, getCullDelay, setCullDelay)
%attribute(carto::Layer, bool, Visible, isVisible, setVisible)
%attributeval(carto::Layer, carto::MapRange, VisibleZoomRange, getVisibleZoomRange, setVisibleZoomRange)
%attribute(carto::Layer, float, Opacity, getOpacity, setOpacity)
%ignore carto::Layer::onDrawFrame;
%ignore carto::Layer::onDrawFrame3D;
%ignore carto::Layer::getBackgroundBitmap;
%ignore carto::Layer::getSkyBitmap;
%ignore carto::Layer::calculateRayIntersectedElements;
%ignore carto::Layer::registerDataSourceListener;
%ignore carto::Layer::unregisterDataSourceListener;
%ignore carto::Layer::getCullDelay;
%ignore carto::Layer::getLastCullState;
!standard_equals(carto::Layer);

%include "layers/Layer.h"

!value_template(std::vector<std::shared_ptr<carto::Layer> >, layers.LayerVector)

#endif
