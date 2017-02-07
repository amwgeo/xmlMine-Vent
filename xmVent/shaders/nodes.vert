#version 120

// refernce:
//      https://www.opengl.org/sdk/docs/tutorials/ClockworkCoders/lighting.php
//      https://en.wikibooks.org/wiki/GLSL_Programming/Blender/Smooth_Specular_Highlights

uniform mat4 u_matMV;
uniform mat4 u_matMVP;
uniform mat3 u_matNorm;
uniform float u_size;

attribute vec3 a_vertex;
attribute vec3 a_normal;

// out
varying vec3 v_normal;      // N
varying vec3 v_eyePos;      // v

void main(void)
{
    vec4 vertex = vec4( u_size * a_vertex, 1. );
    v_eyePos = vec3(u_matMV * vertex);
    v_normal = normalize(u_matNorm * a_normal);
    gl_Position = u_matMVP * vertex;
}
