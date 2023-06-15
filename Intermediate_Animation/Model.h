#ifndef MODEL_H
#define MODEL_H
#include <vector>
#include <string>
#include <unordered_map>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "Shader.h"
#include "Mesh.h"

struct BoneInfo {
	int id;
	glm::mat4 offset; //Used to transform vertex to the specific bone's bone space
};

class Model {
public:
	Model(const char* path);
	void Draw(Shader& shader);
	std::unordered_map<std::string, BoneInfo>& getBoneInfoMap();
	int& getBoneCount();
private:
	std::vector<Mesh> meshes;
	std::vector<Texture> textures_loaded;
	std::string directory;
	std::unordered_map<std::string, BoneInfo> m_BoneInfoMap; //Contains bone names, which are associated to an id and offset matrix.
	int m_BoneCounter = 0; //Used for assigning id numbers to bones

	void loadModel(std::string path);
	void processNode(aiNode* node, const aiScene* scene);
	Mesh processMesh(aiMesh* mesh, const aiScene* scene);
	std::vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName);
	void setVertexBoneDataToDefault(Vertex& vertex);
	void setVertexBoneData(Vertex& vertex, int boneID, float weight);
	void extractBoneWeightForVertices(std::vector<Vertex>& vertices, aiMesh* mesh, const aiScene* scene);
};
#endif
