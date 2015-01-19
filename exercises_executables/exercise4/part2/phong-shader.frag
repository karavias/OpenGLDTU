#version 150 
uniform vec4 AmbientProduct, DiffuseProduct, SpecularProduct;
uniform mat4 ModelView;
uniform vec4 LightPosition;
uniform float Shininess;

in vec3 N;
in vec3 L;
in vec3 E;
in  vec4 color;
out vec4 fragColor;

void main() 
{ 
	vec3 NN = normalize(N);
	vec3 EE = normalize(E);
	vec3 LL = normalize(L);
	vec4 ambient, diffuse, specular;
	vec3 H = normalize(LL+EE);
	float Kd = max(dot(LL, NN), 0.0);
	Kd = dot(LL, NN);
	float Ks = pow(max(dot(NN, H), 0.0), Shininess);
	ambient = AmbientProduct;
	diffuse = Kd*DiffuseProduct;
	if (dot(LL, NN) < 0.0) specular = vec4(0.0, 0.0, 0.0, 1.0);
	else specular = Ks*SpecularProduct;
	fragColor = vec4((ambient + diffuse + specular).xyz, 1.0);
     
} 

