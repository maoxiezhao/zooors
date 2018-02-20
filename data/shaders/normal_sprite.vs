#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec4 color;
layout (location = 2) in vec2 texCoord;
layout (location = 3) in vec3 normal;

uniform mat4 projection;

out vec4 ourColor;
out vec2 TexCoords;

void main()
{
	gl_Position = vec4(position, 1.0) * projection;
    TexCoords = texCoord;
	ourColor  = color;
}