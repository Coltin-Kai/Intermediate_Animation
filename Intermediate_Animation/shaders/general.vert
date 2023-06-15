#version 450 core

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 norm;
layout(location = 2) in vec2 tex;
layout(location = 3) in ivec4 boneIds;
layout(location = 4) in vec4 weights;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

const int MAX_BONES = 100; //Max number of bones allowed total
const int MAX_BONE_INFLUENCE = 4; //Max number of influences the vertice can experience
uniform mat4 finalBonesMatrices[MAX_BONES];
	
out vec2 TexCoords;
	
void main()
{
    vec4 totalPosition = vec4(0.0f);
    for(int i = 0 ; i < MAX_BONE_INFLUENCE ; i++)
    {
        if(boneIds[i] == -1) //If bone somehow doesn't exist, just continue anyway to next one
            continue;
        if(boneIds[i] >= MAX_BONES) //If value exceed max, just use the normal vertice posiion and break out loop
        {
            totalPosition = vec4(pos,1.0f);
            break;
        }
        vec4 localPosition = finalBonesMatrices[boneIds[i]] * vec4(pos,1.0f);
        totalPosition += localPosition * weights[i];
    }
	//totalPosition at this point should have the final result of all transformations caused by the bones incluencing the vertice
    mat4 viewModel = view * model;
    gl_Position = projection * viewModel * totalPosition;
    TexCoords = tex;
}