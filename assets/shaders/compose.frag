#version 110

uniform sampler2D albedoMap;
uniform sampler2D lightMap;

void main() {
	vec2 uv = gl_TexCoord[0].st;
	vec3 albedo = texture2D(albedoMap, uv).rgb;
	vec3 light = texture2D(lightMap, uv).rgb;
	gl_FragColor = vec4(albedo * light, 1);
}
