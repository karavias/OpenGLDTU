#version 150
// 02561-03-01

in vec3 colorV;
in vec3 position;
out vec4 fragColor;

void main(void)
{
    fragColor = vec4(colorV, 1.0);
	if (mod(round(position.y), 2) == 0) {
		discard;
	}
}