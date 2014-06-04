#version 120

varying vec3 position;
varying vec3 normal;
varying vec3 reflectDirection;

uniform vec3 cameraDirection;
uniform vec3 cameraEyePoint;

void main(void) {
    gl_TexCoord[0] = gl_MultiTexCoord0;
    
	vec4 p = gl_ModelViewMatrix * gl_Vertex;
    position = p.xyz / p.w;
	normal = normalize(gl_NormalMatrix * gl_Normal);
	
	reflectDirection = reflect(cameraDirection, gl_Normal.xyz);
	
	gl_Position = ftransform();
}