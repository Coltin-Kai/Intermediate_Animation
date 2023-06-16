#include "Model.h"
#include "stb_image.h"

static unsigned int TextureFromFile(const char* path, const std::string& directory, bool gamma = false);

Model::Model(const char* path) {
    loadModel(path);
}

void Model::Draw(Shader& shader) {
    for (unsigned int i = 0; i < meshes.size(); i++) {
        meshes[i].Draw(shader);
    }
}

void Model::loadModel(std::string path) {
    Assimp::Importer import;
    const aiScene* scene = import.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs); //Get Scene from object file
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        std::cout << "ERROR::ASSIMP::Model::" << import.GetErrorString() << std::endl;
        return;
    }

    std::cout << "Num of Materials: " << +scene->mNumMaterials << std::endl; //*

    directory = path.substr(0, path.find_last_of('\\'));
    processNode(scene->mRootNode, scene);
}

void Model::processNode(aiNode* node, const aiScene* scene) {
    for (unsigned int i = 0; i < node->mNumMeshes; i++) { //Get the index for each mesh from nodes and use the index to access actual mesh data from scene's meshes array
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        meshes.push_back(processMesh(mesh, scene));
    }
    for (unsigned int i = 0; i < node->mNumChildren; i++) { //Recurse to children nodes
        processNode(node->mChildren[i], scene);
    }
}

Mesh Model::processMesh(aiMesh* mesh, const aiScene* scene) { //Transfers Assimps Mesh structure to our Mesh Class Structure
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture> textures;
    for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
        Vertex vertex;
        setVertexBoneDataToDefault(vertex); //FIll m_BoneIDs and m_Weights of the Vertex with default values
        glm::vec3 vector;
        vector.x = mesh->mVertices[i].x;
        vector.y = mesh->mVertices[i].y;
        vector.z = mesh->mVertices[i].z;
        vertex.position = vector;
        if (mesh->HasNormals()) {
            vector.x = mesh->mNormals[i].x;
            vector.y = mesh->mNormals[i].y;
            vector.z = mesh->mNormals[i].z;
            vertex.normal = vector;
        }
        if (mesh->mTextureCoords[0]) { //True if Mesh contain Texture Coords
            glm::vec3 vec;
            vec.x = mesh->mTextureCoords[0][i].x;
            vec.y = mesh->mTextureCoords[0][i].y;
            vertex.texCoords = vec;
        }
        else {
            vertex.texCoords = glm::vec2(0.0f, 0.0f);
        }
        vertices.push_back(vertex);
    }
    //Indices
    for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
        aiFace face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++)
            indices.push_back(face.mIndices[j]);
    }
    //Material
    if (mesh->mMaterialIndex >= 0) {
        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

        std::cout << "Material " << material->GetName().C_Str() << std::endl; //*
        std::cout << "  TextureCount" << std::endl;
        std::cout << "  DiffuseTextures: " << material->GetTextureCount(aiTextureType_DIFFUSE) << std::endl;
        std::cout << "  SpecularTextures: " << material->GetTextureCount(aiTextureType_SPECULAR) << std::endl;
        std::cout << "  AmbientTextures: " << material->GetTextureCount(aiTextureType_AMBIENT) << std::endl;
        std::cout << "  EmissiveTextures: " << material->GetTextureCount(aiTextureType_EMISSIVE) << std::endl;
        std::cout << "  HeightTextures: " << material->GetTextureCount(aiTextureType_HEIGHT) << std::endl;
        std::cout << "  NormalTextures: " << material->GetTextureCount(aiTextureType_NORMALS) << std::endl;
        std::cout << "  ShininessTextures: " << material->GetTextureCount(aiTextureType_SHININESS) << std::endl;
        std::cout << "  OpacityTextures:  " << material->GetTextureCount(aiTextureType_OPACITY) << std::endl;
        std::cout << "  DisplacementTextures: " << material->GetTextureCount(aiTextureType_DISPLACEMENT) << std::endl;
        std::cout << "  LightMapTextures: " << material->GetTextureCount(aiTextureType_LIGHTMAP) << std::endl;
        std::cout << "  ReflectionTextures: " << material->GetTextureCount(aiTextureType_REFLECTION) << std::endl;
        std::cout << "  PBR Materials\n  BaseColorTextures: " << material->GetTextureCount(aiTextureType_BASE_COLOR) << std::endl;
        std::cout << "  NormalCameraTextures: " << material->GetTextureCount(aiTextureType_NORMAL_CAMERA) << std::endl;
        std::cout << "  EmissionColorTextures: " << material->GetTextureCount(aiTextureType_EMISSION_COLOR) << std::endl;
        std::cout << "  MetalnessTextures: " << material->GetTextureCount(aiTextureType_METALNESS) << std::endl;
        std::cout << "  DiffuseRoughnessTextures: " << material->GetTextureCount(aiTextureType_DIFFUSE_ROUGHNESS) << std::endl;
        std::cout << "  AmbientOcclusionTextures: " << material->GetTextureCount(aiTextureType_AMBIENT_OCCLUSION) << std::endl;
        std::cout << "  UnknownTexutes: " << material->GetTextureCount(aiTextureType_UNKNOWN) << std::endl;


        std::vector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
        textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
        std::vector<Texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
        textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
        std::vector<Texture> opacityMaps = loadMaterialTextures(material, aiTextureType_OPACITY, "texture_opacity");
        textures.insert(textures.end(), opacityMaps.begin(), opacityMaps.end());
    }
    extractBoneWeightForVertices(vertices, mesh, scene); //Extract Bone associated information from Assimp's Scene and Mesh to vertices
    return Mesh(vertices, indices, textures);
}

std::vector<Texture> Model::loadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName) {
    std::vector<Texture> textures;
    for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
    {
        aiString str;
        mat->GetTexture(type, i, &str);
        bool skip = false;
        for (unsigned int j = 0; j < textures_loaded.size(); j++) {
            if (std::strcmp(textures_loaded[j].path.data(), str.C_Str()) == 0 && textures_loaded[j].type == typeName) { //Compare current texture's path to those already loaded. If there is an equal comparison, then the texture was already loaded. So indicate to skip
                textures.push_back(textures_loaded[j]);
                skip = true;
                break;
            }
        }
        if (!skip) {   // if texture hasn't been loaded already, load it
            std::cout << "  Got " << typeName << ": " << str.C_Str() << std::endl; //*
            Texture texture;
            texture.id = TextureFromFile(str.C_Str(), directory); 
            texture.type = typeName;
            texture.path = str.C_Str();
            textures.push_back(texture);
            textures_loaded.push_back(texture); // add to loaded textures
        }
    }
    return textures;
}

std::unordered_map<std::string, BoneInfo>& Model::getBoneInfoMap() {
    return m_BoneInfoMap;
}

int& Model::getBoneCount() {
    return m_BoneCounter;
}

void Model::setVertexBoneDataToDefault(Vertex& vertex) { //Set the two member arrays of vertex with default values
    for (int i = 0; i < MAX_BONE_INFLUENCE; i++) {
        vertex.m_BoneIDs[i] = -1;
        vertex.m_Weights[i] = 0.0f;
    }
}

void Model::setVertexBoneData(Vertex& vertex, int boneID, float weight) { //Assign the vertex with the boneID and the weight.
    for (int i = 0; i < MAX_BONE_INFLUENCE; i++) { //Goes through list until it finds an empty slot (where boneID = -1). Then it puts in the boneid and weight
        if (vertex.m_BoneIDs[i] < 0) { 
            vertex.m_Weights[i] = weight;
            vertex.m_BoneIDs[i] = boneID;
            break;
        }
    }
}

void Model::extractBoneWeightForVertices(std::vector<Vertex>& vertices, aiMesh* mesh, const aiScene* scene) { //Should make it so that a Model contains information about every Bone's name, ID, and their offset matrix. And every Mesh's vertex should know what bones affect it and their weight.
    for (int boneIndex = 0; boneIndex < mesh->mNumBones; boneIndex++) { //Extract every bone's name and offset matrix to be put in Model's BoneInfo Map and assign them an ID. Then for every weight of the bone, find the associated vertex and assign them the weight and boneID.
        int boneID = -1;
        std::string boneName = mesh->mBones[boneIndex]->mName.C_Str();
        if (m_BoneInfoMap.find(boneName) == m_BoneInfoMap.end()) { //If we cannot find bone in our map, it's a new bone. So add it
            BoneInfo newBoneInfo;
            newBoneInfo.id = m_BoneCounter;
            //Transfer Assimp's Matrix to GLM's Matrix. Luckily, both column major order. a,b,c,d are rows. 1,2,3,4 are columns
            aiMatrix4x4 temp = mesh->mBones[boneIndex]->mOffsetMatrix;
            newBoneInfo.offset[0][0] = temp.a1; newBoneInfo.offset[1][0] = temp.a2; newBoneInfo.offset[2][0] = temp.a3; newBoneInfo.offset[3][0] = temp.a4;
            newBoneInfo.offset[0][1] = temp.b1; newBoneInfo.offset[1][1] = temp.b2; newBoneInfo.offset[2][1] = temp.b3; newBoneInfo.offset[3][1] = temp.b4;
            newBoneInfo.offset[0][2] = temp.c1; newBoneInfo.offset[1][2] = temp.c2; newBoneInfo.offset[2][2] = temp.c3; newBoneInfo.offset[3][2] = temp.c4;
            newBoneInfo.offset[0][3] = temp.d1; newBoneInfo.offset[1][3] = temp.d2; newBoneInfo.offset[2][3] = temp.d3; newBoneInfo.offset[3][3] = temp.d4;
            m_BoneInfoMap[boneName] = newBoneInfo;
            boneID = m_BoneCounter;
            m_BoneCounter++;
        }
        else {
            boneID = m_BoneInfoMap[boneName].id;
        }
        assert(boneID != -1);
        aiVertexWeight* weights = mesh->mBones[boneIndex]->mWeights;
        int numWeights = mesh->mBones[boneIndex]->mNumWeights;
        for (int weightIndex = 0; weightIndex < numWeights; weightIndex++) {
            int vertexId = weights[weightIndex].mVertexId; //Associated vertex (via id/indice) with the weight
            float weight = weights[weightIndex].mWeight;
            assert(vertexId <= vertices.size());
            setVertexBoneData(vertices[vertexId], boneID, weight); //Add weight and boneID to vertex's data structure
        }
    }

}

static unsigned int TextureFromFile(const char* path, const std::string& directory, bool gamma) { //Function used to load texture onto gpu and outputs its ID
    std::string filename(path);
    filename = directory + '/' + filename;

    unsigned int textureID;
    glGenTextures(1, &textureID);
    int width, height, nrComponents;
    unsigned char* data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
    if (data) {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }
    return textureID;
}
