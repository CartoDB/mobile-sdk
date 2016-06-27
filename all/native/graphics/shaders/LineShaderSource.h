#ifndef _CARTO_LINESHADERSOURCE_H_
#define _CARTO_LINESHADERSOURCE_H_

#include "ShaderSource.h"

#include <string>

static const std::string line_vert_glsl = R"GLSL(
    #version 100

    attribute vec3 a_coord;
    attribute vec3 a_normal;
    attribute vec2 a_texCoord;
    attribute vec4 a_color;
    uniform float u_gamma;
    uniform float u_dpToPX;
    uniform float u_unitToDP;
    uniform mat4 u_mvpMat;
    varying lowp vec4 v_color;
    varying vec2 v_texCoord;
    varying float v_dist;
    varying float v_width;

    void main() {
        float width = length(a_normal.xy) * u_dpToPX;
        float roundedWidth = width + 1.0;
        vec3 pos = a_coord + u_unitToDP * roundedWidth / width * vec3(a_normal.xy * a_normal.z, 0.0);
        v_color = a_color;
        v_texCoord = a_texCoord;
        v_dist = a_normal.z * roundedWidth * u_gamma;
        v_width = 1.0 + (width - 1.0) * u_gamma;
        gl_Position = u_mvpMat * vec4(pos, 1.0);
    }
)GLSL";

static std::string line_frag_glsl = R"GLSL(
    #version 100

    precision mediump float;
    uniform sampler2D u_tex;
    varying lowp vec4 v_color;
    varying vec2 v_texCoord;
    varying float v_dist;
    varying float v_width;

    void main() {
        lowp float a = clamp(v_width - abs(v_dist), 0.0, 1.0);
        gl_FragColor = texture2D(u_tex, v_texCoord) * v_color * a;
    }
)GLSL";

static carto::ShaderSource line_shader_source("line", &line_vert_glsl, &line_frag_glsl);

#endif
