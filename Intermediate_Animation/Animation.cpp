#include "Animation.h"

Animation::Animation(std::string animationPath, Model* model, int& num_Animations, int index) { //Takes animation file path and the Model used for the animation.
    Assimp::Importer importer; //Uses Importer to read the animation file and contain its contents in a Scene object
    const aiScene* scene = importer.ReadFile(animationPath, aiProcess_Triangulate);
    if (scene == NULL) {
        std::cout << "ANIMATION::Scene points to nothing" << std::endl;
    }
    assert(scene && scene->mRootNode);
    aiAnimation* animation = scene->mAnimations[index]; //Note that it only takes in one animation from the file (the first one present in the list of animations).
    num_Animations = scene->mNumAnimations;
    m_Duraction = animation->mDuration;
    m_TicksPerSecond = animation->mTicksPerSecond;
    readHeirarchyData(m_RootNode, scene->mRootNode);
    readMissingBones(animation, *model);
    m_MorphAnims.reserve(animation->mNumMorphMeshChannels);
    for (int i = 0; i < animation->mNumMorphMeshChannels; i++) {
        m_MorphAnims.push_back(MorphAnim(animation->mMorphMeshChannels[i], model));
    }
}

Animation::~Animation() {
}

Bone* Animation::findBone(const std::string& name) { //Finds bone with specified name in the container of bones (via lamda functions).
   auto iter = std::find_if(m_Bones.begin(), m_Bones.end(), [&](const Bone& bone) {
        return bone.getBoneName() == name;
        });
   if (iter == m_Bones.end())
       return nullptr;
   else
       return &(*iter);
}

std::vector<MorphAnim>& Animation::getMorphAnims() {
    return m_MorphAnims;
}

void Animation::readMissingBones(const aiAnimation* animation, Model& model) { //Used just in case there are missing bones in the model file that are found in the animation file. Adds these missing bones to our model BoneInfoMap container and adjust other variables
    int size = animation->mNumChannels;
    auto& boneInfoMap = model.getBoneInfoMap(); //Reference as we are adding back the missing bones to the Model's contaienr and changing count.
    int& boneCount = model.getBoneCount();
    m_Bones.reserve(size);
    for (int i = 0; i < size; i++) {
        auto channel = animation->mChannels[i];
        std::string boneName = channel->mNodeName.data;

        if (boneInfoMap.find(boneName) == boneInfoMap.end()) {
            boneInfoMap[boneName].id = boneCount;
            boneCount++;
        }
        m_Bones.push_back(Bone(channel->mNodeName.data, boneInfoMap[channel->mNodeName.data].id, channel));
    }

    m_BoneInfoMap = boneInfoMap;
}

void Animation::readHeirarchyData(AssimpNodeData& dest, const aiNode* src) { //Used to recreate the heirarchy of node data of Assimp into our Animation's member variable's data structure.
    assert(src);

    dest.name = src->mName.data;
    aiMatrix4x4 temp = src->mTransformation;
    dest.transformation[0][0] = temp.a1; dest.transformation[1][0] = temp.a2; dest.transformation[2][0] = temp.a3; dest.transformation[3][0] = temp.a4;
    dest.transformation[0][1] = temp.b1; dest.transformation[1][1] = temp.b2; dest.transformation[2][1] = temp.b3; dest.transformation[3][1] = temp.b4;
    dest.transformation[0][2] = temp.c1; dest.transformation[1][2] = temp.c2; dest.transformation[2][2] = temp.c3; dest.transformation[3][2] = temp.c4;
    dest.transformation[0][3] = temp.d1; dest.transformation[1][3] = temp.d2; dest.transformation[2][3] = temp.d3; dest.transformation[3][3] = temp.d4;
    dest.childrenCount = src->mNumChildren;
    dest.children.reserve(src->mNumChildren);
    for (int i = 0; i < src->mNumChildren; i++) {
        AssimpNodeData newData;
        readHeirarchyData(newData, src->mChildren[i]);
        dest.children.push_back(newData);
    }
}
