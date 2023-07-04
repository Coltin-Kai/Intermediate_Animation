#ifndef MESH_H
#define MESH_H
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <string>
#include <vector>
#include "Shader.h"

constexpr int MAX_BONE_INFLUENCE = 4;

struct Vertex {
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec2 texCoords;
	int m_BoneIDs[MAX_BONE_INFLUENCE];
	float m_Weights[MAX_BONE_INFLUENCE];
};

struct Texture {
	unsigned int id;
	std::string type;
	std::string path;
};

struct Shape {
	std::vector<glm::vec4> positions;
	float weight;
};

class Mesh {
public:
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
	std::vector<Texture> textures;
	std::string name;
	std::vector<Shape> shapes;

	Mesh(std::string name, std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures, std::vector<Shape> shapes);
	void Draw(Shader& shader);
	void updateMorphWeights(std::vector<float> newWeights);
private:
	unsigned int VAO, VBO, EBO, SSBO_shapes;
	void setupMesh();
};

#endif
