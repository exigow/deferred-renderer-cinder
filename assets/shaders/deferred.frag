#version 110

varying vec3 position;
varying vec3 normal;
varying vec3 reflectDirection;

uniform sampler2D albedoTexture;
uniform sampler2D enviroTexture;
uniform samplerCube	cubeMap;
uniform vec3 cameraDirection;

void main() {
	vec2 uv = gl_TexCoord[0].st;
	vec3 albedo = texture2D(albedoTexture, uv).rgb;
	vec3 enviro = texture2D(enviroTexture, vec2(.5 + normal.r, .5 + normal.g)).rgb;
	
	float depth = min(gl_FragCoord.z / gl_FragCoord.w * 0.0125, 1.0);
	
	vec3 envColor = pow(1 - dot(normal, vec3(0, 0, 1)), 2) + textureCube(cubeMap, reflectDirection);

	gl_FragData[0] = vec4(albedo.rgb, depth); // Color + depth
	gl_FragData[1] = vec4(normal, 1.0); // Normal
	gl_FragData[2] = vec4(position.rgb, 1.0); // Position
	gl_FragData[3] = vec4(envColor, 1); // Enviro
}
