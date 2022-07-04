#version 450 core

in vec4 vPosition;
in vec4 vColor;

out vec4 fColor;

void main() {
	fColor = vColor;
	gl_Position = vPosition;
}
