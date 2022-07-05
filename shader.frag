#version 450

uniform sampler2D tex;

in vec4 fColor;
in vec2 fTexCoord;

out vec4 color;

void main() {
	color = fColor;
	color += texture(tex, fTexCoord); /* crash when texture() don't exist */
}
