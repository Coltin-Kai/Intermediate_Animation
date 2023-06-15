#ifndef BONE_H
#define BONE_H
#include <glm/gtc/type_ptr.hpp>
#include <assimp/scene.h>
#include <vector>

struct KeyPosition {
	glm::vec3 position;
	float timeStamp;
};

struct KeyRotation {
	glm::quat orientation;
	float timeStamp;
};

struct KeyScale {
	glm::vec3 scale;
	float timeStamp;
};

class Bone {
public:
	Bone(const std::string& name, int ID, const aiNodeAnim* channel);
	void Update(float animationTime);
	glm::mat4 getLocalTransform() const;
	std::string getBoneName() const;
	int getBoneID() const;
	int getPositionIndex(float animationTime) const;
	int getRotationIndex(float animationTime) const;
	int getScaleIndex(float animationTime) const;
private:
	std::vector<KeyPosition> m_Positions; //Holds every keyframe position, rotation, and scale of the bone
	std::vector<KeyRotation> m_Rotations;
	std::vector<KeyScale> m_Scales;
	int m_NumPositions; //Total number of keyframe positions, rotations, and scales.
	int m_NumRotations;
	int m_NumScalings;
	glm::mat4 m_LocalTransform;
	std::string m_Name; 
	int m_ID;

	float getScaleFactor(float lastTimeStamp, float nextTimeStamp, float animationTime);
	glm::mat4 interpolatePosition(float animationTime);
	glm::mat4 interpolateRotation(float animationTime);
	glm::mat4 interpolateScaling(float animationTime);
};
#endif
