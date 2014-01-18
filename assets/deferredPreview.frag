uniform sampler2D frag0;
uniform sampler2D frag1;
uniform sampler2D frag2;

void main() {
	vec2 uvQuad = mod(gl_TexCoord[0].st, .5) * vec2(2.0);
	vec2 uv = gl_TexCoord[0].st;

	vec3 albedo = vec3(texture2D(frag0, uvQuad).rgb);
	vec3 depth = vec3(texture2D(frag0, uvQuad).a);
	vec3 normal = vec3(texture2D(frag1, uvQuad).rgb);
	vec3 position = vec3(texture2D(frag2, uvQuad).rgb);

	vec3 asd;

	if (uv.x < .5) {
		if (uv.y > .5) {
			asd = albedo;
		} else {
			asd = depth;
		}
	} else {
		if (uv.y > .5) {
			asd = normal;
		} else {
			asd = position;
		}
	}

	gl_FragColor = vec4(asd, 1);
}