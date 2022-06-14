#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D texture_diffuse1;
uniform float alpha;

void main()
{    
    FragColor = texture(texture_diffuse1, TexCoords);
    FragColor = vec4(FragColor.xyz, alpha);
}
