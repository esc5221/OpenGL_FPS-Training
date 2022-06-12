#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out vec2 TexCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform int is_billboard; 

void main()
{
	TexCoords = aTexCoords;
	
	mat4 viewModel = view * model;

	gl_Position = projection * viewModel * vec4(aPos, 1.0);
	//if (is_billboard == 1)
	//{
	//	gl_Position = vec4(aPos, 1.0);
	//}
	
}