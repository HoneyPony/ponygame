#version 100
precision mediump float;

varying vec2 v_uv;

uniform sampler2D tex;

void main() {
	gl_FragColor = texture2D(tex, v_uv, 0.0);
	if(gl_FragColor.a < 0.01) discard;
}