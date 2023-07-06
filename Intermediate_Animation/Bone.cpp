#include "Bone.h"
#include <iostream>

Bone::Bone(const std::string& name, int ID, const aiNodeAnim* channel) : m_Name(name), m_ID(ID), m_LocalTransform(1.0f) {
    //Transfer Assimp data to our Bone class data srucutre and its member variables.
    //Specifically, the position, rotation, and scale values and their associated timestamps are extracted.
    m_NumPositions = channel->mNumPositionKeys;
    for (int positionIndex = 0; positionIndex < m_NumPositions; positionIndex++) {
        aiVector3D aiPosition = channel->mPositionKeys[positionIndex].mValue;
        float timeStamp = channel->mPositionKeys[positionIndex].mTime;
        KeyPosition data;
        data.position.x = aiPosition.x;
        data.position.y = aiPosition.y;
        data.position.z = aiPosition.z;
        data.timeStamp = timeStamp;
        m_Positions.push_back(data);
    }

    m_NumRotations = channel->mNumRotationKeys;
    for (int rotationIndex = 0; rotationIndex < m_NumRotations; rotationIndex++) {
        aiQuaternion aiOrientation = channel->mRotationKeys[rotationIndex].mValue;
        float timeStamp = channel->mRotationKeys[rotationIndex].mTime;
        KeyRotation data;
        data.orientation.w = aiOrientation.w;
        data.orientation.x = aiOrientation.x;
        data.orientation.y = aiOrientation.y;
        data.orientation.z = aiOrientation.z;
        data.timeStamp = timeStamp;
        m_Rotations.push_back(data);
    }

    m_NumScalings = channel->mNumScalingKeys;
    for (int scaleIndex = 0; scaleIndex < m_NumScalings; scaleIndex++) {
        aiVector3D scale = channel->mScalingKeys[scaleIndex].mValue;
        float timeStamp = channel->mScalingKeys[scaleIndex].mTime;
        KeyScale data;
        data.scale.x = scale.x;
        data.scale.y = scale.y;
        data.scale.z = scale.z;
        data.timeStamp = timeStamp;
        m_Scales.push_back(data);
    }
}

void Bone::Update(float animationTime) { //Updatas the final SRT transformation matrix of the bone (in bone space) at the specified animation time at a keyframe time or inbetween two keyframes.
    glm::mat4 translation = interpolatePosition(animationTime);
    glm::mat4 rotation = interpolateRotation(animationTime);
    glm::mat4 scale = interpolateScaling(animationTime);
    m_LocalTransform = translation * rotation * scale;
}

glm::mat4 Bone::getLocalTransform() const {
    return m_LocalTransform;
}

std::string Bone::getBoneName() const {
    return m_Name;
}

int Bone::getBoneID() const {
    return m_ID;
}

int Bone::getPositionIndex(float animationTime) const { //get_Index functions get the index of the keyframe's position, rotation, or scale where the animation time has passed. Aka return last keyframe index based on time.
    for (int index = 0; index < m_NumPositions - 1; index++) { //Iteraes through list until finding the keyframe where animation time hasn't pass
        if (animationTime < m_Positions[index + 1].timeStamp)
            return index;
        if (index == m_NumPositions - 2) { //If time is actually pass the last keyframe, return last index instead.
            return m_NumPositions - 1;
        }
    }
    return -1; //Failed to find an index. -1 for Failure.
}

int Bone::getRotationIndex(float animationTime) const {
    for (int index = 0; index < m_NumRotations - 1; index++) {
        if (animationTime < m_Rotations[index + 1].timeStamp)
            return index;
        if (index == m_NumRotations - 2) {
            return m_NumRotations - 1;
        }
    }
    return -1;
}

int Bone::getScaleIndex(float animationTime) const {
    for (int index = 0; index < m_NumScalings - 1; index++) {
        if (animationTime < m_Scales[index + 1].timeStamp)
            return index;
        if (index == m_NumScalings - 2) {
            return m_NumScalings - 1;
        }
    }
    return -1;
}

float Bone::getScaleFactor(float lastTimeStamp, float nextTimeStamp, float animationTime) { //Finds the scale factor used in interpolation between the last keyframe and next keyframe's transformation by judging based on the time between the keyframes |----|--I--|----|
    float scaleFactor = 0.0f;
    float midWayLength = animationTime - lastTimeStamp; //The span of time between the last keyframe's time to the current animation time.
    float framesDiff = nextTimeStamp - lastTimeStamp; //The span of time between the last keyframe's time and the next keyframe's time
    scaleFactor = midWayLength / framesDiff; //Ratio of the current span of time between the two over the total span of time between the keyframes. 
    return scaleFactor;
}

glm::mat4 Bone::interpolatePosition(float animationTime) { //interpolate_ functions use the last keyframe and next keyframe index to get the keyframe's values and the calculated scale factor to interpolate betweem the two values. Then returns result as a transformation matrix.
    if (m_NumPositions == 1)
        return glm::translate(glm::mat4(1.0f), m_Positions[0].position);
    glm::vec3 finalPosition;
    int p0Index = getPositionIndex(animationTime);
    assert(p0Index != -1);
    if (p0Index != m_NumPositions - 1) { //Normal Case when time is between Keyframes
        int p1Index = p0Index + 1;
        float scaleFactor = getScaleFactor(m_Positions[p0Index].timeStamp, m_Positions[p1Index].timeStamp, animationTime);
        finalPosition = glm::mix(m_Positions[p0Index].position, m_Positions[p1Index].position, scaleFactor);
    }
    else { //When Time is pass last keyframe, and time is under total time duration, clamp to the last keyframe animation pose.
        finalPosition = m_Positions[p0Index].position;
    }
    return glm::translate(glm::mat4(1.0f), finalPosition);
}

glm::mat4 Bone::interpolateRotation(float animationTime) {
    if (m_NumRotations == 1) {
       glm::quat rotation = glm::normalize(m_Rotations[0].orientation);
       return glm::mat4_cast(rotation);
    }
    glm::quat finalRotation;
    int p0Index = getRotationIndex(animationTime);
    assert(p0Index != -1);
    if (p0Index != m_NumRotations - 1) {
        int p1Index = p0Index + 1;
        float scaleFactor = getScaleFactor(m_Rotations[p0Index].timeStamp, m_Rotations[p1Index].timeStamp, animationTime);
        finalRotation = glm::slerp(m_Rotations[p0Index].orientation, m_Rotations[p1Index].orientation, scaleFactor); //Rotation uses SLERP instead of mix's LERP for more accuracy
        finalRotation = glm::normalize(finalRotation);
    }
    else {
        finalRotation = m_Rotations[p0Index].orientation;
        finalRotation = glm::normalize(finalRotation);
    }
    return glm::mat4_cast(finalRotation);
}

glm::mat4 Bone::interpolateScaling(float animationTime) {
    if (m_NumScalings == 1)
        return glm::scale(glm::mat4(1.0f), m_Scales[0].scale);
    glm::vec3 finalScale;
    int p0Index = getScaleIndex(animationTime);
    assert(p0Index != -1);
    if (p0Index != m_NumScalings - 1) {
        int p1Index = p0Index + 1;
        float scaleFactor = getScaleFactor(m_Scales[p0Index].timeStamp, m_Scales[p1Index].timeStamp, animationTime);
        finalScale = glm::mix(m_Scales[p0Index].scale, m_Scales[p1Index].scale, scaleFactor);
    }
    else {
        finalScale = m_Scales[p0Index].scale;
    }
    return glm::scale(glm::mat4(1.0f), finalScale);
}
