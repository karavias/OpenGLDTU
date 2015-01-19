#version 150

in vec3 position;
in vec3 color;
in vec3 normal;



uniform mat4 projection;
uniform mat4 modelView;
uniform float height;
uniform vec4 LightPosition;
uniform float point;
uniform float seaLevel;
uniform vec4 centerOfTheWorld;
out vec4 colorV;
out vec3 normalV;
out vec3 N;
out vec3 L;
out vec3 E;
out vec3 vPosition;

//vec2 co
float rand(float val){
	float PI = 3.14159265f;
	return sin(val * 2 * PI /20);
//	return sin(dot(co.xy ,vec2(2.9898,78.233))) * 2;
//    return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

void main (void)
{
	vPosition = position;
	colorV = vec4(color, 1.0);
	normalV = (LightPosition * vec4(normal,0.0)).xyz;

	
	N = normal.xyz;
	L = LightPosition.xyz - centerOfTheWorld.xyz;// - position.xyz;
	if (LightPosition.w == 0.0) {
		L = LightPosition.xyz;
	}
	E = position.xyz;


	vec3 offset = vec3(0, 0, 0);
	if (point == 1) {
		colorV = vec4(1.0, 1.0, 1.0,1.0);
		offset = normal*2;
	}

	if (position.y <= seaLevel && normal.x > -1000) {
		if (height > seaLevel) {
			gl_Position = projection * modelView * vec4(position.x, height + rand(position.z + height), position.z, 1.0);
		} else {
			gl_Position = projection * modelView * (vec4(position.x, seaLevel, position.z, 1.0));
		}
	} else {
		if (height > position.y) {
			gl_Position = projection * modelView * vec4(position.x, height + rand(position.z + height), position.z, 1.0);
		} else {
			gl_Position = projection * modelView * vec4(position + offset, 1.0);
		}
	}


}