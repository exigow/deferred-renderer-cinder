#version 110

varying vec3 position;
varying vec3 viewSpaceNormal;
varying vec3 worldSpaceNormal;

uniform sampler2D textureAlbedo;
uniform sampler2D textureNormal;
uniform samplerCube	cubeMap;
uniform vec3 cameraDirection;

void main() {
	vec2 uv = gl_TexCoord[0].st * vec2(1, -1) + vec2(0, 1);
	vec3 albedoSource = texture2D(textureAlbedo, uv).rgb;
	vec3 normalSource = texture2D(textureNormal, uv).rgb;
	
	vec3 normal = normalize(normalSource * 2.0 - 1.0); 
	float diffuse = max(dot(normal, viewSpaceNormal), 0.0);  
	
	float depth = min(gl_FragCoord.z / gl_FragCoord.w * 0.0125, 1.0);
	
	vec3 reflectDirection = reflect(cameraDirection, worldSpaceNormal);
	vec3 envColor = pow(1 - dot(viewSpaceNormal, vec3(0, 0, 1)), 2) + textureCube(cubeMap, reflectDirection);

	gl_FragData[0] = vec4(albedoSource.rgb, depth); // Color + depth
	gl_FragData[1] = vec4(viewSpaceNormal, 1.0); // Normal
	gl_FragData[2] = vec4(position.rgb, 1.0); // Position
	gl_FragData[3] = vec4(envColor, 1); // Enviro
}