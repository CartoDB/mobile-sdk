#ifndef _CARTO_BLENDSHADERSOURCE_H_
#define _CARTO_BLENDSHADERSOURCE_H_

#include "ShaderSource.h"

static std::string blend_vert_glsl =
    "#version 100\n"

    "attribute vec2 a_coord;"
    "uniform mat4 u_mvpMat;"
    "void main() {"
    "    gl_Position = u_mvpMat * vec4(a_coord, 0.0, 1.0);"
    "}";

static std::string blend_frag_glsl =
    "#version 100\n"

    "precision mediump float;"

    "uniform sampler2D u_tex;"
    "uniform lowp vec4 u_color;"
    "uniform mediump vec2 u_invScreenSize;"
    "void main() {"
    "    vec4 texColor = texture2D(u_tex, gl_FragCoord.xy * u_invScreenSize);"
    "    gl_FragColor = texColor * u_color;"
    "}";

static carto::ShaderSource blend_shader_source("blend", &blend_vert_glsl, &blend_frag_glsl);

#endif

