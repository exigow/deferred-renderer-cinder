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
	
	vec3 localPosition = lightTransformedPosition - positionSource;
	float distance = length(localPosition);
	if (distance > lightRadius) {
		discard;
	}
	localPosition /= distance;
	
	vec4 normal = texture2D(normalMap, uv);
	
	vec3 halfVector = normalize(localPosition + normalize(-positionSource));
	
	float NdotL = dot(normal.rgb, localPosition);

	float attenuation = 1 - (distance / lightRadius);

	float specular = pow((max(dot(halfVector, normal.rgb), 0.0) + .025), 64); //  * normal.a

    gl_FragColor = vec4(lightColor * vec3((NdotL + specular) * attenuation) * normal.a, 1);
	
	//gl_FragColor = vec4(lightColor * vec3(specular * attenuation), 1);
}




















