#version 110

uniform sampler2D albedoAndDepthMap;
uniform sampler2D normalMap;
uniform sampler2D positionMap;
uniform sampler2D enviroMap;
uniform sampler2D lightMap;


void main() {
	vec2 uv = gl_TexCoord[0].st;
	vec4 albedoAndDepth = texture2D(albedoAndDepthMap, uv);
	vec4 normal = texture2D(normalMap, uv);
	vec4 position = texture2D(positionMap, uv);
	vec4 enviro = texture2D(enviroMap, uv);
	vec4 light = texture2D(lightMap, uv);
	
	gl_FragColor = vec4(light.rgb * albedoAndDepth.rgb, 1);
}
