#version 100
precision highp float;

varying vec2 v_uv;
varying vec4 v_color;

uniform sampler2D tex;

void main() {
	gl_FragColor = v_color * texture2D(tex, v_uv, 0.0);
	if(gl_FragColor.a < 0.01) discard;
}