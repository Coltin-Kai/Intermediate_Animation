#ifndef MORPHANIM_H
#define MORPHANIM_H
#include <assimp/scene.h>
#include "Model.h"
#include "Mesh.h"

struct KeyMorph {
	std::vector<unsigned int> values;
	std::vector<float> weights;
	float timeStamp;
};

class MorphAnim {
public:
	MorphAnim(const aiMeshMorphAnim* assimp_moprhAnim, Model* model);
	void Update(float animationTime);
private:
	std::vector<Mesh*> meshes; //Meshes this MorphAnim affects.
	std::vector<KeyMorph> morph_keys; //The Keyframes of the Morphs
	std::vector<float> interpolateWeights(float animationTime);
	int getMorphKeyIndex(float animationTime);
	float getScaleFactor(float lastTimeStamp, float nextTimeStamp, float animationTime);
};

#endif