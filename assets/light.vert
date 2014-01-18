#version 120

uniform float dist;
varying vec4 pos;

void main(void) {
	gl_Position = ftransform();
	pos = gl_Position;
}