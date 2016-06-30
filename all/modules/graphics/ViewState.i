#ifndef _VIEWSTATE_I
#define _VIEWSTATE_I

#pragma SWIG nowarn=325

%module ViewState

!proxy_imports(carto::ViewState, core.MapPos, core.ScreenPos, core.MapVec, components.Options, graphics.Frustum, projections.Projection)
!java_imports(carto::ViewState, com.carto.components.ProjectionMode)

%{
#include "graphics/ViewState.h"	
#include <cglib/mat.h>
%}

%include <cartoswig.i>

%import "core/MapPos.i"
%import "core/MapVec.i"
%import "core/ScreenPos.i"
%import "components/Options.i"
%import "graphics/Frustum.i"

!value_type(carto::ViewState, graphics.ViewState)

%attributeval(carto::ViewState, carto::Frustum, Frustum, getFrustum)
%attribute(carto::ViewState, float, Rotation, getRotation)
%attribute(carto::ViewState, float, Zoom, getZoom)
%attribute(carto::ViewState, float, Tilt, getTilt)
%attribute(carto::ViewState, carto::ProjectionMode::ProjectionMode, ProjectionMode, getProjectionMode)
%attribute(carto::ViewState, float, Zoom0Distance, getZoom0Distance)
%attribute(carto::ViewState, int, FOVY, getFOVY)
%attribute(carto::ViewState, float, Near, getNear)
%attribute(carto::ViewState, float, Far, getFar)
%attribute(carto::ViewState, bool, CameraChanged, isCameraChanged)
%attribute(carto::ViewState, int, Width, getWidth)
%attribute(carto::ViewState, int, Height, getHeight)
%attribute(carto::ViewState, int, ScreenWidth, getScreenWidth)
%attribute(carto::ViewState, int, ScreenHeight, getScreenHeight)
%attribute(carto::ViewState, float, DPI, getDPI)
%attribute(carto::ViewState, float, DPToPX, getDPToPX)
%attribute(carto::ViewState, float, UnitToDPCoef, getUnitToDPCoef)
%attribute(carto::ViewState, float, UnitToPXCoef, getUnitToPXCoef)
%attribute(carto::ViewState, float, AspectRatio, getAspectRatio)
%ignore carto::ViewState::ViewState;
%ignore carto::ViewState::RotationState;
%ignore carto::ViewState::getCameraPos();
%ignore carto::ViewState::getFocusPos();
%ignore carto::ViewState::getUpVec();
%ignore carto::ViewState::getHalfWidth();
%ignore carto::ViewState::getHalfHeight();
%ignore carto::ViewState::getTanHalfFOVY();
%ignore carto::ViewState::getHalfFOVY();
%ignore carto::ViewState::getNormalizedResolution();
%ignore carto::ViewState::getTanHalfFOVX();
%ignore carto::ViewState::getCosHalfFOVY();
%ignore carto::ViewState::getCosHalfFOVXY();
%ignore carto::ViewState::cameraChanged;
%ignore carto::ViewState::setRotation;
%ignore carto::ViewState::setTilt;
%ignore carto::ViewState::setZoom;
%ignore carto::ViewState::get2PowZoom;
%ignore carto::ViewState::getRotationState;
%ignore carto::ViewState::getProjectionMat;
%ignore carto::ViewState::getModelviewMat;
%ignore carto::ViewState::getModelviewProjectionMat;
%ignore carto::ViewState::GetLocalMat;
%ignore carto::ViewState::getRTELocalMat;
%ignore carto::ViewState::getRTEModelviewMat;
%ignore carto::ViewState::getRTEModelviewProjectionMat;
%ignore carto::ViewState::setScreenSize;
%ignore carto::ViewState::calculateViewState;
%ignore carto::ViewState::worldToScreen;
%ignore carto::ViewState::screenToWorldPlane;
%ignore carto::ViewState::getHorizontalLayerOffsetDir;
%ignore carto::ViewState::setHorizontalLayerOffsetDir;
!standard_equals(carto::ViewState);

%include "graphics/ViewState.h"

#endif
