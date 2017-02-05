attribute highp vec4 vertex;
uniform highp mat4 matrix;
uniform float pointSize;

void main(void)
{
  gl_Position = matrix * vertex;
  gl_PointSize = pointSize;
}
