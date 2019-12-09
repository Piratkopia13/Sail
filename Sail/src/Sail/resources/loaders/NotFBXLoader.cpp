#include "pch.h"
#include "NotFBXLoader.h"
#include <fstream>

void NotFBXLoader::Load(std::string filename, Model*& model, Shader* shader,  AnimationStack*& animationStack) {
	std::ifstream out(filename, std::fstream::binary);
	char m = 'm';
	char a = 'a';

	char type;

	while (!out.eof()) {
		out.read((char*)&type, sizeof(type));
		if (type == m) {

			Mesh::Data data;

			//Mesh / Model
			out.read((char*)&data.numIndices, sizeof(data.numIndices));
			data.indices = new unsigned long[data.numIndices];
			out.read((char*)data.indices, sizeof(*data.indices) * data.numIndices);
			out.read((char*)& data.numVertices, sizeof(data.numVertices));
			out.read((char*)& data.numInstances, sizeof(data.numInstances));

			data.positions		= new Mesh::vec3[data.numVertices];
			data.normals		= new Mesh::vec3[data.numVertices];
			//data.colors		= new Mesh::vec4[data.numVertices];
			data.texCoords		= new Mesh::vec2[data.numVertices];
			data.tangents		= new Mesh::vec3[data.numVertices];
			data.bitangents	= new Mesh::vec3[data.numVertices];

			out.read((char*)data.positions,	sizeof(*data.positions) *	data.numVertices);
			out.read((char*)data.normals,		sizeof(*data.normals) *	data.numVertices);
			//out.read((char*)data.colors,		sizeof(*data.colors) *		data.numVertices);
			out.read((char*)data.texCoords,	sizeof(*data.texCoords) *	data.numVertices);
			out.read((char*)data.tangents,		sizeof(*data.tangents) *	data.numVertices);
			out.read((char*)data.bitangents,	sizeof(*data.bitangents) * data.numVertices);
						
			model = new Model(data, shader);
		} else if (type == a) {

		} else {
			break;
		}
	}

}

void NotFBXLoader::Save(std::string filename, Model* model, AnimationStack* animationStack) {
	std::ofstream out(filename, std::fstream::binary);

	char m = 'm';
	char a = 'a';

	if (model) {
		out.write((char*)&m, sizeof(m));

		Mesh* mesh = model->getMesh(0);
		const Mesh::Data& data = mesh->getData();

		//Mesh / Model
		out.write((char*)&data.numIndices, sizeof(data.numIndices));
		out.write((char*)data.indices, sizeof(*data.indices) * data.numIndices);
		out.write((char*)&data.numVertices, sizeof(data.numVertices));
		out.write((char*)&data.numInstances, sizeof(data.numInstances));
		out.write((char*)data.positions, sizeof(*data.positions) * data.numVertices);
		out.write((char*)data.normals, sizeof(*data.normals) * data.numVertices);
		out.write((char*)data.texCoords, sizeof(*data.texCoords) * data.numVertices);
		out.write((char*)data.tangents, sizeof(*data.tangents) * data.numVertices);
		out.write((char*)data.bitangents, sizeof(*data.bitangents) * data.numVertices);
	}

	if (animationStack) {
		out.write((char*)&a, sizeof(a));

		//Animation
		const unsigned int connS = animationStack->getConnectionSize();
		AnimationStack::VertConnection* con = animationStack->getConnections();

		out.write((char*)&connS, sizeof(connS));
		out.write((char*)con, sizeof(*con) * connS);
	}

}
