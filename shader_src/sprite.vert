#version 100

uniform mat4 transform;

attribute vec3 pos;

void main() {
	vec4 r = transform * vec4(pos.x, pos.y, pos.z, 1.0);
	gl_Position = r;
}