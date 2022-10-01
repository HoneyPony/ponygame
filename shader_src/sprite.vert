#version 100
precision highp float;

uniform mat4 transform;

attribute vec3 a_pos;
attribute vec2 a_uv;

attribute vec4 a_color;

varying vec2 v_uv;
varying vec4 v_color;

void main() {
	vec4 pos = vec4(a_pos, 1.0);
	gl_Position = transform * pos;

	v_uv = a_uv;
	v_color = a_color;
}