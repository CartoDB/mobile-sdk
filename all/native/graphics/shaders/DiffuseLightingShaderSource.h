#ifndef _CARTO_DIFFUSELIGHTINGSHADERSOURCE_H_
#define _CARTO_DIFFUSELIGHTINGSHADERSOURCE_H_

#include "ShaderSource.h"

#include <string>

static std::string diffuse_lighting_vert_glsl =
    "#version 100\n"

    "attribute vec4 a_color;"
    "attribute vec4 a_coord;"
    "attribute vec3 a_normal;"
    "attribute vec2 a_texCoord;"
    "uniform vec4 u_ambientColor;"
    "uniform vec4 u_lightColor;"
    "uniform vec3 u_lightDir;"
    "uniform mat4 u_mvpMat;"
    "varying vec4 v_color;"
    "varying vec2 v_texCoord;"
    "void main() {"
    "   if (a_normal.z != 1.0) {"
    "      float dotProduct = max(0.0, dot(a_normal, u_lightDir));"
    "	   v_color = vec4(u_ambientColor.rgb + u_lightColor.rgb * dotProduct, 1.0) * a_color;"
    "   } else {"
    "      v_color = a_color;"
    "   }"
    "   v_texCoord = a_texCoord;"
    "	gl_Position = u_mvpMat * a_coord;"
    "}";

static std::string diffuse_lighting_frag_glsl =
    "#version 100\n"

    "precision mediump float;"

    "uniform sampler2D u_tex;"
    "varying vec4 v_color;"
    "varying vec2 v_texCoord;"
    "void main() {"
    "	vec4 color = texture2D(u_tex, v_texCoord) * v_color;"
    "	if (color.a == 0.0) {"
    "		discard;"
    "	}"
    "	gl_FragColor = color;"
    "}";

static carto::ShaderSource diffuse_lighting_shader_source("diffuse-lighting", &diffuse_lighting_vert_glsl, &diffuse_lighting_frag_glsl);

#endif
