#include "Animator.h"

Animator::Animator(Animation* animation) { //Initlize the Animator with a loaded animation
    m_CurrentTime = 0.0f; 
    m_CurrentAnimation = animation;
    m_FinalBoneMatrices.reserve(100);
    for (int i = 0; i < 100; i++)
        m_FinalBoneMatrices.push_back(glm::mat4(1.0f));
}

void Animator::updateAnimation(float deltaTime) { //Updates the animation time based on deltaTime and recalculates the bone transformations.
    if (m_CurrentAnimation) {
        m_CurrentTime += m_CurrentAnimation->getTicksPerSecond() * deltaTime; //Calculates the new current time by adding span of time of the deltaTime adjusted by ticksPerSecond
        m_CurrentTime = fmod(m_CurrentTime, m_CurrentAnimation->getDuration()); //Modulo it with the total durection of the animation as currentTime will eventually go above the duration time, so we want the currentTime to reset to cause an animation repeat.
        calculateBoneTransformation(&m_CurrentAnimation->getRootNode(), glm::mat4(1.0f));
    }
}

void Animator::playAnimation(Animation* animation) { //Change the animation
    m_CurrentAnimation = animation;
    m_CurrentTime = 0.0f;
}

void Animator::calculateBoneTransformation(const AssimpNodeData* node, const glm::mat4& parentTransform) { //Calculates and outputs the final array of matrices used to send to shader and transform matrices
    std::string nodeName = node->name;
    glm::mat4 nodeTransform = node->transformation;

    Bone* bone = m_CurrentAnimation->findBone(nodeName);

    if (bone) { //If node actually is bone, we used the associated bone's transformation. Else, just use the transformation already within.
        bone->Update(m_CurrentTime);
        nodeTransform = bone->getLocalTransform();
    }

    glm::mat4 globalTransformation = parentTransform * nodeTransform;

    auto boneInfoMap = m_CurrentAnimation->getBoneIDMap();
    if (boneInfoMap.find(nodeName) != boneInfoMap.end()) {
        int index = boneInfoMap[nodeName].id;
        glm::mat4 offset = boneInfoMap[nodeName].offset;
        m_FinalBoneMatrices[index] = globalTransformation * offset; //Applies offset transformation of the bone then the transformations from it's parents in node hierarchy.
    }
   
    for (int i = 0; i < node->childrenCount; i++)
        calculateBoneTransformation(&node->children[i], globalTransformation);
}

std::vector<glm::mat4> Animator::getFinalBoneMatrices() {
    return m_FinalBoneMatrices;
}
