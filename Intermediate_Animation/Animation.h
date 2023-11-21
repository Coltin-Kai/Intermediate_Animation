#ifndef  ANIMATION_H
#define ANIMATION_H
#include <glm/gtc/type_ptr.hpp>
#include <string>
#include <vector>
#include <unordered_map>
#include "Model.h"
#include "Bone.h"
#include "MorphAnim.h"

struct AssimpNodeData { //Animation data sructure isolated from Assimp that mimics its aiNode class.
	glm::mat4 transformation;
	std::string name;
	int childrenCount;
	std::vector<AssimpNodeData> children;
};

class Animation {
public:
	std::vector<std::string> animationNames; //Name of every animation contained in the file. Not exactly related to just the animation represented by the object.
	Animation() = default;
	Animation(std::string animationPath, Model* model, int index = 0);
	~Animation();
	Bone* findBone(const std::string& name);
	std::vector<MorphAnim>& getMorphAnims();
	inline float getTicksPerSecond() {
		return m_TicksPerSecond;
	}

	inline float getDuration() {
		return m_Duraction;
	}

	inline const AssimpNodeData& getRootNode() {
		return m_RootNode;
	}

	inline const std::unordered_map<std::string, BoneInfo>& getBoneIDMap() {
		return m_BoneInfoMap;
	}
private:
	float m_Duraction; //How long the animation is
	int m_TicksPerSecond; //The animation speed
	std::vector<Bone> m_Bones; //List of every bone
	std::vector<MorphAnim> m_MorphAnims; //List of every MorphAnim
	AssimpNodeData m_RootNode; //The root node of the heirarchical structure of bones
	std::unordered_map<std::string, BoneInfo> m_BoneInfoMap;
	void readMissingBones(const aiAnimation* animation, Model& model);
	void readHeirarchyData(AssimpNodeData& dest, const aiNode* src);
};
#endif
