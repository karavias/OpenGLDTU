#version 150 

in  vec3 vPosition;
in  vec3 vNormal;
out vec4 color;

uniform vec4 AmbientProduct, DiffuseProduct, SpecularProduct;
uniform mat4 ModelView;
uniform mat4 Projection;
uniform vec4 LightPosition;
uniform float Shininess;

void main()
{
	gl_Position = Projection * ModelView * vec4(vPosition, 1.0);
	
	vec3 N = normalize(vec4(vNormal, 1.0).xyz);
	vec3 L = normalize(LightPosition.xyz - (ModelView* vec4(vPosition, 1.0)).xyz);
	vec3 E = -normalize((ModelView * vec4(vPosition, 1.0)).xyz);
	vec3 H = normalize(L + E);
	float Kd = max(dot(L, N), 0.0);
	vec4 ambient = AmbientProduct;
	vec4 diffuse = Kd * DiffuseProduct;
	vec4 specular = max(pow(max(dot(N, H), 0.0), Shininess)*
					SpecularProduct, 0.0);
    color = vec4((ambient + diffuse + specular).xyz, 1.0);

}