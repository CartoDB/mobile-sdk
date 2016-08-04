#ifndef _CARTO_SOLIDSHADERSOURCE_H_
#define _CARTO_SOLIDSHADERSOURCE_H_

#include "ShaderSource.h"

#include <string>

static std::string solid_vert_glsl =
    "#version 100\n"
    "attribute vec4 a_coord;"
    "attribute vec2 a_texCoord;"
    "varying vec2 v_texCoord;"
    "uniform mat4 u_mvpMat;"
    "void main() {"
    "	v_texCoord = a_texCoord;"
    "	gl_Position = u_mvpMat * a_coord;"
    "}";

static std::string solid_frag_glsl =
    "#version 100\n"
    "precision mediump float;"
    "varying vec2 v_texCoord;"
    "uniform sampler2D u_tex;"
    "uniform vec4 u_color;"
    "void main() {"
    "	gl_FragColor = texture2D(u_tex, v_texCoord) * u_color;"
    "}";

static carto::ShaderSource solid_shader_source("solid", &solid_vert_glsl, &solid_frag_glsl);

#endif

