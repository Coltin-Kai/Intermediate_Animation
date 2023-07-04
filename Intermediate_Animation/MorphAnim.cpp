#include "MorphAnim.h"

MorphAnim::MorphAnim(const aiMeshMorphAnim* assimp_moprhAnim, Model* model) {
	morph_keys.reserve(assimp_moprhAnim->mNumKeys);
	for (int i = 0; i < assimp_moprhAnim->mNumKeys; i++) {
		KeyMorph key;
		key.timeStamp = assimp_moprhAnim->mKeys[i].mTime;
		key.values.reserve(assimp_moprhAnim->mKeys[i].mNumValuesAndWeights);
		key.weights.reserve(assimp_moprhAnim->mKeys[i].mNumValuesAndWeights);
		for (int j = 0; j < assimp_moprhAnim->mKeys[i].mNumValuesAndWeights; j++) {
			key.values.push_back(assimp_moprhAnim->mKeys[i].mValues[j]);
			key.weights.push_back(assimp_moprhAnim->mKeys[i].mWeights[j]);
		}
		morph_keys.push_back(key);
	}
	std::vector<Mesh>& model_meshes = model->getMeshes();
	std::string morphAnimName(assimp_moprhAnim->mName.C_Str());
	for (int i = 0; i < model_meshes.size(); i++) {
		if (model_meshes[i].name.find(morphAnimName) != std::string::npos) {
			meshes.push_back(&model_meshes[i]);
		}
	}
}

void MorphAnim::Update(float animationTime) {
	std::vector<float> newWeights = interpolateWeights(animationTime);
	for (int i = 0; i < meshes.size(); i++) {
		meshes[i]->updateMorphWeights(newWeights);
	}
}

std::vector<float> MorphAnim::interpolateWeights(float animationTime) {
	if (morph_keys.size() == 1) {
		return morph_keys[0].weights;
	}
	std::vector<float> weights;
	weights.reserve(morph_keys[0].weights.size());
	int p0Index = getMorphKeyIndex(animationTime);
	int p1Index = p0Index + 1;
	float factor = getScaleFactor(morph_keys[p0Index].timeStamp, morph_keys[p1Index].timeStamp, animationTime);
	for (int i = 0; i < morph_keys[0].weights.size(); i++) {
		weights.push_back(glm::mix(morph_keys[p0Index].weights[i], morph_keys[p1Index].weights[i], factor));
	}
	return weights;
}

int MorphAnim::getMorphKeyIndex(float animationTime) {
	for (int index = 0; index < morph_keys.size() - 1; index++) {
		if (animationTime < morph_keys[index + 1].timeStamp)
			return index;
	}
	return -1;
}

float MorphAnim::getScaleFactor(float lastTimeStamp, float nextTimeStamp, float animationTime) {
	float scaleFactor = 0.0f;
	float midWayLength = animationTime - lastTimeStamp;
	float framesDiff = nextTimeStamp - lastTimeStamp;
	scaleFactor = midWayLength / framesDiff;
	return scaleFactor;
}
