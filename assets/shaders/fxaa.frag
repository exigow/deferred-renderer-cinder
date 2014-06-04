#version 110

// Based on: http://horde3d.org/wiki/index.php5?title=Shading_Technique_-_FXAA

uniform sampler2D source;
uniform vec2 frameBufSize;

void main() {
	float FXAA_SPAN_MAX = 8.0;
	float FXAA_REDUCE_MUL = 1.0 / 8.0;
	float FXAA_REDUCE_MIN = 1.0 / 128.0;
	
	vec2 uv = gl_TexCoord[0].st;
	
	vec3 rgbNW = texture2D(source, uv + (vec2(-1.0, -1.0) / frameBufSize)).rgb;
	vec3 rgbNE = texture2D(source, uv + (vec2(1.0, -1.0) / frameBufSize)).rgb;
	vec3 rgbSW = texture2D(source, uv + (vec2(-1.0, 1.0) / frameBufSize)).rgb;
	vec3 rgbSE = texture2D(source, uv + (vec2(1.0, 1.0) / frameBufSize)).rgb;
	vec3 rgbM = texture2D(source, uv).rgb;
	
	vec3 luma = vec3(0.299, 0.587, 0.114);
	vec4 scalar = vec4(dot(rgbNW, luma), dot(rgbNE, luma), dot(rgbSW, luma), dot(rgbSE, luma));
	float lumaM  = dot(rgbM,  luma);
	
	float lumaMin = min(lumaM, min(min(scalar.x, scalar.y), min(scalar.z, scalar.a)));
	float lumaMax = max(lumaM, max(max(scalar.x, scalar.y), max(scalar.z, scalar.a)));
	
	vec2 dir;
	dir.x = -((scalar.x + scalar.y) - (scalar.z + scalar.a));
	dir.y = ((scalar.x + scalar.z) - (scalar.y + scalar.a));
	
	float dirReduce = max((scalar.x + scalar.y + scalar.z + scalar.a) * (0.25 * FXAA_REDUCE_MUL), FXAA_REDUCE_MIN);
	float rcpDirMin = 1.0 / (min(abs(dir.x), abs(dir.y)) + dirReduce);
	
	dir = min(vec2(FXAA_SPAN_MAX, FXAA_SPAN_MAX), max(vec2(-FXAA_SPAN_MAX, -FXAA_SPAN_MAX), dir * rcpDirMin)) / frameBufSize;
	
	vec3 rgbA = (1.0 / 2.0) * (texture2D(source, uv.xy + dir * (1.0 / 3.0 - 0.5)).xyz + texture2D(source, uv.xy + dir * (2.0 / 3.0 - 0.5)).xyz);
	vec3 rgbB = rgbA * (1.0 / 2.0) + (1.0 / 4.0) * (texture2D(source, uv.xy + dir * (0.0 / 3.0 - 0.5)).xyz + texture2D(source, uv.xy + dir * (3.0 / 3.0 - 0.5)).xyz);
	
	float lumaB = dot(rgbB, luma);
	if ((lumaB < lumaMin) || (lumaB > lumaMax)) {
		gl_FragColor.xyz = rgbA;
	} else {
		gl_FragColor.xyz = rgbB;
	}
}
