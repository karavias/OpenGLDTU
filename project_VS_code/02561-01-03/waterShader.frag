#version 150
// 02561-01-03
uniform vec4 AmbientProduct, DiffuseProduct, SpecularProduct;
uniform float seaLevel;
uniform float height;

in vec4 colorV;
in vec3 N;
in vec3 L;
in vec3 E;
in vec3 vPosition;
out vec4 fragColor;

void main(void)
{

	
	if (vPosition.y > seaLevel + 2) {
		discard;
		return;
	}
	fragColor = colorV;
	
}