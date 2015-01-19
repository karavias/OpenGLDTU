#version 150

uniform sampler3D textureVolume;
uniform vec3 cameraPos;

const float iso = 0.5;

const float stepsize = 0.001;

out vec4 fragColor;
in vec3 vPosition;
void main(void)
{
	vec4 col_acc = vec4(0.0);//Accumulated color along the ray
	float alpha_acc = 0.0;
  
	vec3 dir = vPosition - cameraPos;
	vec3 p0 = vPosition;

	for (float i =0; i < sqrt(3.0); i+= stepsize) {
		vec3 p = p0 + i*dir;
		vec4 color = texture3D(textureVolume, p);
		alpha_acc += color.a;
		if (color.a < iso) {
			col_acc += color;
		}
	}
	
	fragColor = col_acc;
}
