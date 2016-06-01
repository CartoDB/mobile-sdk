#ifndef _CARTO_REGULARSHADERSOURCE_H_
#define _CARTO_REGULARSHADERSOURCE_H_

#include "ShaderSource.h"

#include <string>

static std::string regular_vert_glsl =
    "#version 100\n"

    "attribute vec4 a_coord;"
    "attribute vec2 a_texCoord;"
    "attribute vec4 a_color;"
    "varying vec2 v_texCoord;"
    "varying vec4 v_color;"
    "uniform mat4 u_mvpMat;"
    "void main() {"
    "   v_texCoord = a_texCoord;"
    "	v_color = a_color;"
    "	gl_Position = u_mvpMat * a_coord;"
    "}";

static std::string regular_frag_glsl =
    "#version 100\n"

    "precision mediump float;"

    "varying vec2 v_texCoord;"
    "varying vec4 v_color;"
    "uniform sampler2D u_tex;"
    "void main() {"
    "	vec4 color = texture2D(u_tex, v_texCoord) * v_color;"
    "	if (color.a == 0.0) {"
    "		discard;"
    "	}"
    "	gl_FragColor = color;"
    "}";

static carto::ShaderSource regular_shader_source("regular", &regular_vert_glsl, &regular_frag_glsl);

#endif