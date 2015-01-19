#version 150 

uniform vec4 AmbientProduct, DiffuseProduct, SpecularProduct;
uniform mat4 ModelView;
uniform mat4 Projection;
uniform vec4 LightPosition;
uniform vec4 LightPosition2;
uniform vec4 LightPosition3;
uniform float Shininess;

in  vec3 vPosition;
in  vec3 vNormal;
out vec3 N;
out vec3 L1;
out vec3 L2;
out vec3 L3;
out vec3 E;
out vec4 color;
void main()
{
    gl_Position = Projection * ModelView * vec4(vPosition, 1.0);
	N = vNormal.xyz;
	L1 = LightPosition.xyz - vPosition.xyz;
	L2 = LightPosition2.xyz - vPosition.xyz;
	L3 = LightPosition3.xyz - vPosition.xyz;
	if (LightPosition.w == 0.0) {
		L1 = LightPosition.xyz;
		L2 = LightPosition2.xyz;
		L3 = LightPosition3.xyz;
	}
	E = vPosition.xyz;
}
