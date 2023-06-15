#ifndef ANIMATOR_H
#define ANIMATOR_H
#include "Animation.h"
class Animator {
public:
	Animator(Animation* animation);
	void updateAnimation(float deltaTime);
	void playAnimation(Animation* animation);
	void calculateBoneTransformation(const AssimpNodeData* node, const glm::mat4& parentTransform);
	std::vector<glm::mat4> getFinalBoneMatrices();
private:
	std::vector<glm::mat4> m_FinalBoneMatrices;
	Animation* m_CurrentAnimation;
	float m_CurrentTime;
};
#endif
