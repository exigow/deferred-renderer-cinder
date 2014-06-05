#version 110

uniform sampler2D albedoMap;
uniform sampler2D lightMap;

void main() {
	vec2 uv = gl_TexCoord[0].st;
	vec3 albedo = texture2D(albedoMap, uv).rgb;
	
	gl_FragColor = vec4(albedo, 1);
}
