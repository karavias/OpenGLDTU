#version 150
// 02561-01-03
uniform vec4 AmbientProduct, DiffuseProduct, SpecularProduct;
uniform float mirror;
uniform float seaLevel;

in vec4 colorV;
in vec3 N;
in vec3 L;
in vec3 E;
in vec3 vPosition;
out vec4 fragColor;

void main(void)
{
	if (vPosition.y <= seaLevel) {
		discard;
	}

	float Shininess = 100;
	vec3 NN = normalize(N);
	vec3 EE = normalize(E);
	vec3 LL = normalize(L);
	vec4 ambient, diffuse, specular;
	vec3 H = normalize(LL+EE);
	float Kd = max(dot(LL, NN), 0.0);
	Kd = dot(LL, NN);
	float Ks = pow(max(dot(NN, H), 0.0), Shininess);
	ambient = AmbientProduct;
	diffuse = Kd*colorV;
	if (dot(LL, NN) < 0.0) specular = vec4(0.0, 0.0, 0.0, 1.0);
	else specular = Ks*SpecularProduct;
	fragColor = colorV*vec4((ambient*colorV + diffuse).xyz, 1.0);
}