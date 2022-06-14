#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform int is_billboard; 

void main()
{
	mat4 viewModel = view * model;

	gl_Position = projection * viewModel * vec4(aPos, 1.0);
}