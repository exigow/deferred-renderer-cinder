#version 120

uniform sampler2D frag0;
uniform sampler2D frag1;
uniform sampler2D frag2;

uniform vec3 lightPosition;
uniform vec3 lightColor;
uniform float lightRadius;

varying vec4 pos;

void main() {
	vec2 uv = pos.xy / pos.w * 0.5f + 0.5f;

	vec3 position = texture2D(frag2, uv).rgb;

	vec3 localPosition = lightPosition - position;
	float distance = length(localPosition);
	if (distance > lightRadius) {
		discard;
	}

	vec3 strength = 1.0 - (vec3(distance) / lightRadius);

	vec3 albedo = texture2D(frag0, uv).rgb;
	vec3 normal = texture2D(frag1, uv).rgb;

	//float cost = dot(normal, localPosition);

	gl_FragColor = vec4(strength * lightColor, 1);
}