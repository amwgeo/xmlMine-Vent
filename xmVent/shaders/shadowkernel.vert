#version 120

// in
attribute vec2 position;
attribute vec2 texCoords;

varying vec2 TexCoords;

void main()
{
    gl_Position = vec4(position, 0.f, 1.f);
    TexCoords = texCoords;
}
