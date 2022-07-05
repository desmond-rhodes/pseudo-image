#version 450 core

in vec4 vPosition;
in vec4 vColor;
in vec2 vTexCoord;

out vec4 fColor;
out vec2 fTexCoord;

void main() {
	gl_Position = vPosition;
	fColor = vColor;
	fTexCoord = vTexCoord;
}
