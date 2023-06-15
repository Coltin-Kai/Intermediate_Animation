#version 450 core
out vec4 FragColor;

in vec2 TexCoords;

struct materials {
    sampler2D texture_diffuse1;
    sampler2D texture_diffuse2;
    sampler2D texture_diffuse3;
    sampler2D texture_specular1;
    sampler2D texture_specular2;
    sampler2D texture_specular3;
};

uniform materials material;

void main()
{    
    FragColor = texture(material.texture_diffuse1, TexCoords);
}