#version 150

uniform samplerCube cubemap;

out vec4 fragColor;
in vec3 texcoords;
void main(void)
{
	fragColor =  texture(cubemap, texcoords);
}
