#version 100
precision highp float;

uniform sampler2D texture;
varying vec2 v_uv;

void main() {
	gl_FragColor = texture2D(texture, v_uv, 0.0);
}