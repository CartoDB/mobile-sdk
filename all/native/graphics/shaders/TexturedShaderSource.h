#ifndef _CARTO_TEXTUREDSHADERSOURCE_H_
#define _CARTO_TEXTUREDSHADERSOURCE_H_

#include "ShaderSource.h"

#include <string>

static std::string textured_vert_glsl =
    "#version 100\n"
    "attribute vec4 a_coord;"
    "attribute vec2 a_texCoord;"
    "varying vec2 v_texCoord;"
    "uniform mat4 u_mvpMat;"
    "void main() {"
    "	v_texCoord = a_texCoord;"
    "	gl_Position = u_mvpMat * a_coord;"
    "}";

static std::string textured_frag_glsl =
    "#version 100\n"
    "precision mediump float;"
    "varying vec2 v_texCoord;"
    "uniform sampler2D u_tex;"
    "void main() {"
    "	gl_FragColor = texture2D(u_tex, v_texCoord);"
    "}";

static carto::ShaderSource textured_shader_source("textured", &textured_vert_glsl, &textured_frag_glsl);

#endif

