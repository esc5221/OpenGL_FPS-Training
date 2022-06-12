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
	if (is_billboard == 1)
	{	
		mat4 bbTransformation = model * inverse(view)
		bbTransformation[0][0] = 1.0;
		bbTransformation[0][1] = 0.0;
		bbTransformation[0][2] = 0.0;

		bbTransformation[1][0] = 0.0;
		bbTransformation[1][1] = 1.0;
		bbTransformation[1][2] = 0.0;

		bbTransformation[2][0] = 0.0;
		bbTransformation[2][1] = 0.0;
		bbTransformation[2][2] = 1.0;
		gl_Position = projection * (vec4(aPos, 1.0) * bbTransformation)
	}
	
}