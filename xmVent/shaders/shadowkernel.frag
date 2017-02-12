#version 120

const float offset = 1. / 100.;

uniform sampler2D screenTexture;

varying vec2 TexCoords;

const float kernel[25] = float[](
        0.f/80.f, 1.f/80.f,  2.f/80.f, 1.f/80.f, 0.f/80.f,
        1.f/80.f, 4.f/80.f,  8.f/80.f, 4.f/80.f, 1.f/80.f,
        2.f/80.f, 8.f/80.f, 16.f/80.f, 8.f/80.f, 2.f/80.f,
        1.f/80.f, 4.f/80.f,  8.f/80.f, 4.f/80.f, 1.f/80.f,
        0.f/80.f, 1.f/80.f,  2.f/80.f, 1.f/80.f, 0.f/80.f );

const vec2 offsets[25] = vec2[](
    vec2(-2.f*offset, 2.f*offset),  vec2(-2.f*offset, offset),  vec2(-2.f*offset, 0.f),
    vec2(-2.f*offset, -offset),  vec2(-2.f*offset, -2.f*offset),

    vec2(-offset, 2.f*offset),  vec2(-offset, offset),  vec2(-offset, 0.f),
    vec2(-offset, -offset),  vec2(-offset, -2.f*offset),

    vec2(0.f, 2.f*offset),  vec2(0.f, offset),  vec2(0.f, 0.f),
    vec2(0.f, -offset),  vec2(0.f, -2.f*offset),

    vec2(offset, 2.f*offset),  vec2(offset, offset),  vec2(offset, 0.f),
    vec2(offset, -offset),  vec2(offset, -2.f*offset),

    vec2(2.f*offset, 2.f*offset),  vec2(2.f*offset, offset),  vec2(2.f*offset, 0.f),
    vec2(2.f*offset, -offset),  vec2(2.f*offset, -2.f*offset)
);


void main()
{
    //gl_FragColor = texture2D(screenTexture, TexCoords);
    vec4 color = vec4(0.);
    for( int i=0; i<25; i++){
        color += kernel[i] * texture2D(screenTexture, TexCoords + offsets[i]);
    }
    gl_FragColor = color;
}
