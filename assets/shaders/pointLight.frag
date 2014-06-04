#version 120

// Buffers.
uniform sampler2D positionMap;
uniform sampler2D normalMap;
uniform sampler2D albedoAndDepthMap;

// Cam position.
uniform vec3 cameraPosition;

// Light attributes.
uniform vec2 lightScreenPosition;
uniform float lightRadius;
uniform vec3 lightColor;
uniform vec3 lightPosition;
uniform vec3 lightTransformedPosition;

varying vec4 pos;

void main() {
	vec2 uv = pos.xy / pos.w * .5 + .5;
	
	vec3 positionSource = texture2D(positionMap, uv).rgb;
	vec3 normalSource = texture2D(normalMap, uv).rgb;

    gl_FragColor = vec4(vec3(normalSource), 1);
}




















