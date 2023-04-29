#pragma once

#include "ourGraphics.h"

//Assimp library needed ti include this header

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

struct Vertex {
	glm::vec3 Position;
	glm::vec3 Color;
	glm::vec3 Normal;
	glm::vec2 TexCoords;
};

struct Texture {
	unsigned int id;
	string type;
};

class Mesh {
public:
	// mesh data
	vector<Vertex>       vertices;
	vector<unsigned int> indices;
	vector<Texture>      textures;

	Mesh(vector<Vertex> vertices, vector<unsigned int> indices, vector<Texture> textures);
	void Draw(Shader *shader);
private:
	//  render data
	unsigned int VAO, VBO, EBO;
	void setupMesh();
};

class Model
{
public:
	Model(const char *path)
	{
		loadModel(path);
	}
	void Draw(Shader *shader);
private:
	// model data
	vector<Mesh> meshes;
	string directory;

	void loadModel(string path);
	void processNode(aiNode *node, const aiScene *scene);
	Mesh processMesh(aiMesh *mesh, const aiScene *scene);
	vector<Texture> loadMaterialTextures(aiMaterial *mat, 
		aiTextureType type,	string typeName);
};