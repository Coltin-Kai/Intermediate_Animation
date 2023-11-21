#version 450 core

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 norm;
layout(location = 2) in vec2 tex;
layout(location = 3) in ivec4 boneIds; 
layout(location = 4) in vec4 bone_weights;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

const int MAX_BONES = 200; //Max number of bones allowed total
const int MAX_BONE_INFLUENCE = 4; //Max number of influences the vertice can experience
uniform mat4 finalBonesMatrices[MAX_BONES];

const int MAX_SHAPES = 200; //Note: To Change, Need to also change its variable in Mesh.cpp
layout(std430, binding = 0) buffer MorphBuffer {
    int numShapes;
    float weights[MAX_SHAPES];
    vec4 positions[][MAX_SHAPES];
} morphBuffer;
	
out vec2 TexCoords;
	
void main()
{
    //Morph 
    vec3 morph_displacements = vec3(0.0);
    for (int i = 0; i < morphBuffer.numShapes; i++) {
        morph_displacements += morphBuffer.weights[i] * (morphBuffer.positions[gl_VertexID][i].xyz - pos);
    }
    vec3 morph_position = pos + morph_displacements;

    //Skeleton        
    vec4 totalPosition = vec4(0.0f);
    for(int i = 0 ; i < MAX_BONE_INFLUENCE ; i++)
    {
        if(boneIds[i] == -1) //If bone somehow doesn't exist, just continue anyway to next one
            continue;
        if(boneIds[i] >= MAX_BONES) //If value exceed max, just use the normal vertice posiion and break out loop
        {
            totalPosition = vec4(morph_position,1.0f);
            break;
        }
        vec4 localPosition = finalBonesMatrices[boneIds[i]] * vec4(morph_position, 1.0f);
        totalPosition += localPosition * bone_weights[i];
    }
    if (boneIds[0] == -1 && boneIds[1] == -1 && boneIds[2] == -1 && boneIds[3] == -1) { //If no bones at all
        totalPosition = vec4(morph_position, 1.0f);
    }
	//totalPosition at this point should have the final result of all transformations caused by the bones incluencing the vertice (or just the regular pos + shape morphs if no bones)
    mat4 viewModel = view * model;
    gl_Position = projection * viewModel * totalPosition;
    TexCoords = tex;
}