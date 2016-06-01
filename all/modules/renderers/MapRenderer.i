#ifndef _MAPRENDERER_I
#define _MAPRENDERER_I

%module MapRenderer

!proxy_imports(carto::MapRenderer, graphics.ViewState, renderers.MapRendererListener, renderers.RendererCaptureListener, renderers.RedrawRequestListener)

%{
#include "renderers/MapRenderer.h"
%}

%include <std_shared_ptr.i>
%include <cartoswig.i>

%import "graphics/ViewState.i"
%import "renderers/MapRendererListener.i"
%import "renderers/RendererCaptureListener.i"
%import "renderers/RedrawRequestListener.i"

!shared_ptr(carto::MapRenderer, renderers.MapRenderer)

%attributestring(carto::MapRenderer, std::shared_ptr<carto::MapRendererListener>, MapRendererListener, getMapRendererListener, setMapRendererListener)
%ignore carto::MapRenderer::MapRenderer;
%ignore carto::MapRenderer::init;
%ignore carto::MapRenderer::deinit;
%ignore carto::MapRenderer::getBillboardDrawDatas;
%ignore carto::MapRenderer::getCameraPos;
%ignore carto::MapRenderer::getFocusPos;
%ignore carto::MapRenderer::getUpVec;
%ignore carto::MapRenderer::getRotation;
%ignore carto::MapRenderer::getTilt;
%ignore carto::MapRenderer::getZoom;
%ignore carto::MapRenderer::getAnimationHandler;
%ignore carto::MapRenderer::getKineticEventHandler;
%ignore carto::MapRenderer::getRedrawRequestListener;
%ignore carto::MapRenderer::setRedrawRequestListener;
%ignore carto::MapRenderer::calculateCameraEvent;
%ignore carto::MapRenderer::moveToFitPoints;
%ignore carto::MapRenderer::screenToWorld;
%ignore carto::MapRenderer::worldToScreen;
%ignore carto::MapRenderer::onSurfaceCreated;
%ignore carto::MapRenderer::onSurfaceChanged;
%ignore carto::MapRenderer::onDrawFrame;
%ignore carto::MapRenderer::onSurfaceDestroyed;
%ignore carto::MapRenderer::calculateRayIntersectedElements;
%ignore carto::MapRenderer::billboardsChanged;
%ignore carto::MapRenderer::layerChanged;
%ignore carto::MapRenderer::viewChanged;
%ignore carto::MapRenderer::registerOnChangeListener;
%ignore carto::MapRenderer::unregisterOnChangeListener;
%ignore carto::MapRenderer::addRenderThreadCallback;

!standard_equals(carto::MapRenderer);

%include "renderers/MapRenderer.h"

#endif
