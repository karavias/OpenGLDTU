#version 150 
uniform vec4 AmbientProduct, DiffuseProduct, SpecularProduct;
uniform mat4 ModelView;
uniform vec4 LightPosition;
uniform vec4 LightPosition2;
uniform vec4 LightPosition3;
uniform float Shininess;

in vec3 N;
in vec3 L1;
in vec3 L2;
in vec3 L3;
in vec3 E;
in  vec4 color;
out vec4 fragColor;

void main() 
{ 
	vec3 NN = normalize(N);
	vec3 EE = normalize(E);
	vec4 ambient, diffuse, specular;
	
	vec3 LL = normalize(L1);
	vec3 H = normalize(LL+EE);
	float Kd = max(dot(LL, NN), 0.0);
	Kd = dot(LL, NN);
	float Ks = pow(max(dot(NN, H), 0.0), Shininess);
	ambient = AmbientProduct;
	diffuse = Kd*DiffuseProduct;
	if (dot(LL, NN) < 0.0) specular = vec4(0.0, 0.0, 0.0, 1.0);
	else specular = Ks*SpecularProduct;

	LL = normalize(L2);
	H = normalize(LL+EE);
	Kd = max(dot(LL, NN), 0.0);
	Kd = dot(LL, NN);
	Ks = pow(max(dot(NN, H), 0.0), Shininess);
	ambient += AmbientProduct;
	diffuse += Kd*DiffuseProduct;
	if (dot(LL, NN) < 0.0) specular += vec4(0.0, 0.0, 0.0, 1.0);
	else specular += Ks*SpecularProduct;

	LL = normalize(L3);
	H = normalize(LL+EE);
	Kd = max(dot(LL, NN), 0.0);
	Kd = dot(LL, NN);
	Ks = pow(max(dot(NN, H), 0.0), Shininess);
	ambient += AmbientProduct;
	diffuse += Kd*DiffuseProduct;
	if (dot(LL, NN) < 0.0) specular += vec4(0.0, 0.0, 0.0, 1.0);
	else specular += Ks*SpecularProduct;
	fragColor = vec4((ambient + diffuse + specular).xyz, 1.0);
     
} 

