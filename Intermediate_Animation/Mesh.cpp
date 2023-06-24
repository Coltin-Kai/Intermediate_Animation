#include "Mesh.h"

Mesh::Mesh(std::string name, std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures, std::vector<Shape> shapes) : name(name), vertices(vertices), indices(indices), textures(textures), shapes(shapes) {
	setupMesh();
}

void Mesh::Draw(Shader& shader) {
	unsigned int diffuseNr = 1;
	unsigned int specularNr = 1;
	unsigned int opacityNr = 1;
	for ( unsigned int i = 0; i < textures.size(); i++ ) { //Assigns each texture to the correct texture unit and uniform sampler
		glActiveTexture(GL_TEXTURE0 + i);
		std::string number;
		std::string name = textures[i].type;
		if (name == "texture_diffuse")
			number = std::to_string(diffuseNr++);
		else if (name == "texture_specular")
			number = std::to_string(specularNr++);
		else if (name == "texture_opacity")
			number = std::to_string(opacityNr++);
		shader.setInt(("material." + name + number).c_str(), i); 
		glBindTexture(GL_TEXTURE_2D, textures[i].id);
	}
	glActiveTexture(GL_TEXTURE0);

	//Draw
	glBindVertexArray(VAO);
	glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}

void Mesh::setupMesh() { //Simply set up the associated Vertex Arrays and Buffers of the Mesh
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal)); //Since C++ enforce no standard on structs offsets and padding. Have to query offset of struct field via offsetof function.
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoords));
	glEnableVertexAttribArray(3);
	glVertexAttribIPointer(3, 4, GL_INT, sizeof(Vertex), (void*)offsetof(Vertex, m_BoneIDs));
	glEnableVertexAttribArray(4);
	glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, m_Weights));
	glBindVertexArray(0);

	//Morph Stuff

	/*
	Using std430 layout, similiar to std140 but some optimizations and change in alignment and strides for arrays and structs of scalars and vector elements (except vec3). Basically they are no longer rounded up to a muliple
	of 16 bytes (that matching of a vec4). So array of floats will match with a C++ array of floats. This layout is only usable with SSBOs. Not UBOs.
	Component | Base Alignment | Offset | Alligned Offset
	float weights[] 4 | 0 | 0
	vec4 positions[][] 16 | 4*weghts.length | 4*weights.length and whatever makes it to multiple of 16
	*/
	unsigned int delta_16 = (4 * shapes.size()) % 16;
	unsigned int positions_alligned_offset = (delta_16 == 16) ? 4 * shapes.size() : 4 * shapes.size() + (16 - delta_16);
	unsigned int ssb_size = positions_alligned_offset + (shapes.size() * shapes[0].positions.size());
	glGenBuffers(1, &SSBO_shapes);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, SSBO_shapes);
	glBufferData(GL_SHADER_STORAGE_BUFFER, ssb_size, NULL, GL_DYNAMIC_DRAW);
	
}
