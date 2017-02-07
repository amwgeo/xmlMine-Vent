#version 120

uniform mat4 u_matMV;
uniform mat4 u_matMVP;
uniform vec4 u_coluorMaterial;

// in
varying vec3 v_normal;      // N
varying vec3 v_eyePos;      // v

const vec4 specularColor = vec4(.5,.5,.5,1.);
const float shininess = 3.;
const vec4 ambientColor = vec4(0.75,0.75,0.75,1);
const vec3 normalLight = vec3(0.577350269189626,0.577350269189626,0.577350269189626);


void main(void)
{
    // ambient colour
    vec4 ambient = u_coluorMaterial * ambientColor;

    // diffuse colour
    float cosTheta = max( 0., dot(v_normal, normalLight));
    vec4 diffuse = vec4(cosTheta * u_coluorMaterial.rgb, u_coluorMaterial.a);
    diffuse = clamp(diffuse, 0., 1.);

    // specular colour
    vec3 eye = normalize( -v_eyePos ); // vector from frag to eye
    vec3 ref = normalize( -reflect(normalLight, v_normal) );  // vector reflectance off frag
    vec4 specular = specularColor * pow(max(dot(ref, eye), 0.), 0.3*shininess);
    specular = clamp(specular, 0., 1.);

    gl_FragColor = ambient + diffuse + specular;
}
