#version 150

uniform sampler2D texture1;
uniform sampler2D shadowTexture;

uniform mat4 lightViewProjection;
out vec4 fragColor;

in vec2 vTextureCoordinate;
in vec4 worldCoordinate;
void main(void) {
	if (!gl_FrontFacing){
		fragColor = vec4(1,1,1,1);
		return;
	}
	vec4 Smap = (lightViewProjection * worldCoordinate);
	vec2 normalizedSmap = Smap.xy/Smap.w;
	vec2 shadowUV = (normalizedSmap + vec2(1.0, 1.0))/2.0; // todo find the shadow map UV coordinates

	vec4 tex = texture(texture1, vTextureCoordinate);
	vec4 shadow = texture(shadowTexture, shadowUV);
	shadow = (shadow + vec4(1.0, 1.0, 1.0, 1.0))/2;
	fragColor = shadow* tex;
}
