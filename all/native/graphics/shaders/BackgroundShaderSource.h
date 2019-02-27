#ifndef _CARTO_BACKGROUNDSHADERSOURCE_H_
#define _CARTO_BACKGROUNDSHADERSOURCE_H_

#include "ShaderSource.h"

static std::string background_vert_glsl =
    "#version 100\n"

    "attribute vec4 a_coord;"
    "attribute vec3 a_normal;"
    "attribute vec2 a_texCoord;"
    "uniform vec3 u_lightDir;"
    "uniform mat4 u_mvpMat;"
    "varying vec4 v_color;"
    "varying vec2 v_texCoord;"
    "void main() {"
    "    float lighting = max(0.0, dot(a_normal, u_lightDir)) * 0.5 + 0.5;"
    "    v_color = vec4(lighting, lighting, lighting, 1.0);"
    "    v_texCoord = a_texCoord;"
    "    gl_Position = u_mvpMat * a_coord;"
    "}";

static std::string background_frag_glsl =
    "#version 100\n"

    "precision mediump float;"

    "uniform sampler2D u_tex;"
    "varying lowp vec4 v_color;"
    "varying highp vec2 v_texCoord;"
    "void main() {"
    "    vec4 color = texture2D(u_tex, v_texCoord) * v_color;"
    "    if (color.a == 0.0) {"
    "        discard;"
    "    }"
    "    gl_FragColor = color;"
    "}";

static carto::ShaderSource background_shader_source("background", &background_vert_glsl, &background_frag_glsl);

#endif
