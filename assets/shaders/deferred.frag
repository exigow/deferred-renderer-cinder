#version 110

varying vec3 position;
varying vec3 viewSpaceNormal;
varying vec3 worldSpaceNormal;

uniform sampler2D textureAlbedo;
uniform sampler2D textureNormal;
uniform sampler2D textureSpecular;
uniform sampler2D textureGloss;

uniform samplerCube	cubeMap;
uniform vec3 cameraDirection;

void main() {
	vec2 uv = gl_TexCoord[0].st * vec2(1, -1) + vec2(0, 1);
	
	vec3 albedoSource = texture2D(textureAlbedo, uv).rgb;
	vec3 normalSource = texture2D(textureNormal, uv).rgb;
	float specularSource = texture2D(textureSpecular, uv).r;
	float glossSource = texture2D(textureGloss, uv).r;
	
	float depth = min(gl_FragCoord.z / gl_FragCoord.w * 0.0125, 1.0);
	
	vec3 reflectDirection = reflect(cameraDirection, worldSpaceNormal);
	vec3 envColor = textureCube(cubeMap, reflectDirection);

	gl_FragData[0] = vec4(albedoSource, depth); // Albedo R, G, B, Depth A
	gl_FragData[1] = vec4(viewSpaceNormal, specularSource); // Normal R, G, B, Specular A
	gl_FragData[2] = vec4(position.rgb, glossSource); // Position R, G, B, ???gloss??? A
	gl_FragData[3] = vec4(envColor, 1); // Enviro R, G, B, ... A
}




