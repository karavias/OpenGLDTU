#version 150

in vec2 vTextureCoord;

uniform sampler2D textureMap;

out vec4 fragColor;

void main(void)
{
	fragColor = vec4(vTextureCoord.x*0.1, vTextureCoord.y *0.1, 1, 1);
}