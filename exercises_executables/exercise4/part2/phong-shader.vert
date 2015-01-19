#version 150 

in  vec3 vPosition;
in  vec3 vNormal;


uniform vec4 AmbientProduct, DiffuseProduct, SpecularProduct;
uniform mat4 ModelView;
uniform mat4 Projection;
uniform vec4 LightPosition;
uniform float Shininess;
out vec3 N;
out vec3 L;
out vec3 E;
out vec4 color;
void main()
{
    gl_Position = Projection * ModelView * vec4(vPosition, 1.0);
	N = vNormal.xyz;
	L = LightPosition.xyz - vPosition.xyz;
	if (LightPosition.w == 0.0) {
		L = LightPosition.xyz;
	}
	E = vPosition.xyz;
}
