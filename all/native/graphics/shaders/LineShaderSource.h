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
    uniform float u_normalScale;
    uniform mat4 u_mvpMat;
    varying lowp vec4 v_color;
    varying vec2 v_texCoord;
    varying vec2 v_dist;
    varying float v_width;

    void main() {
        float width = length(a_normal.xy);
        float roundedWidth = width + 1.0;
        vec3 pos = a_coord + u_normalScale * roundedWidth / width * vec3(a_normal.xy * a_normal.z, 0.0);
        v_color = a_color;
        v_texCoord = a_texCoord;
        v_dist = vec2(0.0, a_normal.z) * roundedWidth * u_gamma;
        v_width = (width - 1.0) * u_gamma;
        gl_Position = u_mvpMat * vec4(pos, 1.0);
    }
)GLSL";

static std::string line_frag_glsl = R"GLSL(
    #version 100

    precision mediump float;
    uniform sampler2D u_tex;
    varying lowp vec4 v_color;
    varying vec2 v_texCoord;
    varying vec2 v_dist;
    varying float v_width;

    void main() {
        float dist = length(v_dist) - v_width;
        lowp float a = clamp(1.0 - dist, 0.0, 1.0);
        gl_FragColor = texture2D(u_tex, v_texCoord) * v_color * a;
    }
)GLSL";

static carto::ShaderSource line_shader_source("line", &line_vert_glsl, &line_frag_glsl);

#endif
