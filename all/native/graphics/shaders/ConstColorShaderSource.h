#ifndef _CARTO_CONSTCOLORSHADERSOURCE_H_
#define _CARTO_CONSTCOLORSHADERSOURCE_H_

#include "ShaderSource.h"

static std::string constcolor_vert_glsl =
    "#version 100\n"

    "attribute vec4 a_coord;"
    "attribute vec4 a_color;"
    "varying vec4 v_color;"
    "uniform mat4 u_mvpMat;"
    "void main() {"
    "	v_color = a_color;"
    "	gl_Position = u_mvpMat * a_coord;"
    "}";

static std::string constcolor_frag_glsl =
    "#version 100\n"

    "precision mediump float;"

    "varying lowp vec4 v_color;"
    "void main() {"
    "	vec4 color = v_color;"
    "	if (color.a == 0.0) {"
    "		discard;"
    "	}"
    "	gl_FragColor = color;"
    "}";

static carto::ShaderSource constcolor_shader_source("constcolor", &constcolor_vert_glsl, &constcolor_frag_glsl);

#endif
