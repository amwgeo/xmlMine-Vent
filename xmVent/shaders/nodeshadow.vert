#version 120

uniform mat4 u_matMVP;
uniform float u_size;
uniform vec3 u_offset;

// in
attribute vec3 a_vertex;

void main(void)
{
    vec4 vertex = vec4( u_offset + u_size * a_vertex, 1. );
    gl_Position = u_matMVP * vertex;
}
