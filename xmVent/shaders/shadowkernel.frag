#version 120

const float offset = 1. / 100.;

uniform sampler2D screenTexture;

varying vec2 TexCoords;

const float kernel[9] = float[](
    1./16., 2./16., 1./16.,
    2./16., 4./16., 2./16.,
    1./16., 2./16., 1./16. );

void main()
{
    vec2 offsets[9] = vec2[](
        vec2(-offset, offset),  // top-left
        vec2(0.0f,    offset),  // top-center
        vec2(offset,  offset),  // top-right
        vec2(-offset, 0.0f),    // center-left
        vec2(0.0f,    0.0f),    // center-center
        vec2(offset,  0.0f),    // center-right
        vec2(-offset, -offset), // bottom-left
        vec2(0.0f,    -offset), // bottom-center
        vec2(offset,  -offset)  // bottom-right
    );

    //gl_FragColor = texture2D(screenTexture, TexCoords);
    vec4 color = vec4(0.);
    for( int i=0; i<9; i++){
        color += kernel[i] * texture2D(screenTexture, TexCoords + offsets[i]);
    }
    gl_FragColor = color;
}
