#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D texture_diffuse1;
//uniform vec3 base_color;
//uniform vec3 base_color = vec3(1.0, 0.0, 0.0);

void main()
{    
    //FragColor = vec4(base_color, 1.0) * texture(texture_diffuse1, TexCoords);
    FragColor = texture(texture_diffuse1, TexCoords);
}