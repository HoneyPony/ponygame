#version 100
precision mediump float;

uniform mat4 transform;

attribute vec3 a_pos;
attribute vec2 a_uv;

varying vec2 v_uv;

void main() {
	vec4 pos = vec4(a_pos, 1.0);
	gl_Position = transform * pos;

	v_uv = a_uv;
}