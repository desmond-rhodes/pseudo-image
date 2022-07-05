#version 450 core

in vec4 vPosition;
in vec2 vTexCoord;

out vec2 fTexCoord;

void main() {
	gl_Position = vPosition;
	fTexCoord = vTexCoord;
}
