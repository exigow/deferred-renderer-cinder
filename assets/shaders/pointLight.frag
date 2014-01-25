#version 120

uniform sampler2D albedoAndDepthMap;
uniform sampler2D normalMap;
uniform sampler2D positionMap;

uniform vec3 cameraPosition;
uniform vec3 lightPosition;
uniform vec3 lightColor;
uniform float lightRadius;

varying vec4 pos;

void main() {
	vec2 uv = pos.xy / pos.w * 0.5 + 0.5;

	vec3 position = texture2D(positionMap, uv).rgb;

	vec3 localPosition = lightPosition - position;
	float distance = length(localPosition);
	if (distance > lightRadius) {
		discard;
	}

	vec3 strength = vec3(1.0 - (distance / lightRadius));
	strength.x = pow(strength.x, 2.0);
	strength.y = pow(strength.y, 2.0);
	strength.z = pow(strength.z, 2.0);

	vec3 albedo = texture2D(albedoAndDepthMap, uv).rgb;
	vec3 normal = texture2D(normalMap, uv).rgb;

	//vec3 h = normalize(localPosition + normalize(cameraPosition - position));
	//float phong = .25f * pow(max(dot(h, normal), 0.0), 64.0);
	

	gl_FragColor = vec4(strength * lightColor, 1.0);  // + phong
}
