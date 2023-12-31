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
    sampler2D texture_opacity1;
    sampler2D texture_opacity2;
    sampler2D texture_opacity3;
};

uniform materials material;
uniform bool no_textures;

void main()
{    
    vec4 color;
    if (no_textures == true) {
        color = vec4(1.0f, 1.0f, 1.0, 1.0f);
    }
    else {
        color = texture(material.texture_diffuse1, TexCoords);
    }
    if (color.a < 0.1) //Kind of Works
        discard;
    FragColor = color;
}