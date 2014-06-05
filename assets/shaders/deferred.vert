#version 120

varying vec3 position;
varying vec3 viewSpaceNormal;
varying vec3 worldSpaceNormal;

void main(void) {
    gl_TexCoord[0] = gl_MultiTexCoord0;
    
	vec4 p = gl_ModelViewMatrix * gl_Vertex;
    position = p.xyz / p.w;
	
	worldSpaceNormal = gl_Normal.xyz;
	viewSpaceNormal = normalize(gl_NormalMatrix * worldSpaceNormal);
	
	gl_Position = ftransform();
}