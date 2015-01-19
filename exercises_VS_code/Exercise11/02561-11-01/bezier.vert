#version 150

uniform mat4 projection;
uniform mat4 modelView;
uniform mat4 curveMatrix;

in float u;

void main(void)
{
	vec4 position = vec4(u*u*u, u*u, u, 1)*curveMatrix;
	gl_Position = projection * modelView * position;
}