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
	
	float fresnel = 1 - dot(normal.rgb, vec3(0, 0, 1));
	if (fresnel == 1) {
		fresnel = 0;
	}
	enviro.rgb += pow(fresnel, 4);
	
	gl_FragColor = vec4(enviro.rgb + light.rgb, 1);
}
