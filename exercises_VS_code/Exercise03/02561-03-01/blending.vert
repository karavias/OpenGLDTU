#version 150
// 02561-03-01

uniform mat4 projection;
uniform mat4 modelView;
uniform float blendValue;
uniform float normalExtrusion;

in vec3 position1;
in vec3 color1;
in vec3 normal1;
in vec3 position2;
in vec3 color2;
in vec3 normal2;

out vec3 colorV;
out vec3 position;

void main (void) {
    colorV = (1-blendValue) * color1 + blendValue*color2;
    position = (1-blendValue) * (position1 + normal1 * normalExtrusion)+ blendValue * (position2 + normal2 * normalExtrusion);
	gl_Position = projection * modelView * vec4((1-blendValue) * (position1 + normal1 * normalExtrusion)+
				blendValue * (position2 + normal2 * normalExtrusion), 1.0);

}