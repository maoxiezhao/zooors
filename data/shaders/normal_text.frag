#version 330 core
in vec4 ourColor;
in vec2 TexCoords;

out vec4 color;

uniform sampler2D Texture0;

void main()
{   
	vec4 sampled = vec4(1.0,1.0,1.0,texture(Texture0,TexCoords).g);
	color = ourColor * sampled;
}