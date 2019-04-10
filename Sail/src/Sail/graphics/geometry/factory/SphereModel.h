#pragma once

#include <memory>
#include "../Model.h"

namespace ModelFactory {

	using namespace DirectX::SimpleMath;

	class SphereModel {
	public:
		static std::unique_ptr<Model> Create(float radius, float numFaces, ShaderSet* shaderSet) {

			//const int numVerts = 4;
			//glm::vec3* positions = new glm::vec3[numVerts]{
			//	glm::vec3(-halfSizes.x, 0.f, -halfSizes.y),
			//	glm::vec3(-halfSizes.x, 0.f, halfSizes.y),
			//	glm::vec3(halfSizes.x, 0.f, -halfSizes.y),
			//	//glm::vec3(halfSizes.x, 0.f, -halfSizes.y),
			//	//glm::vec3(-halfSizes.x, 0.f, halfSizes.y),
			//	glm::vec3(halfSizes.x, 0.f, halfSizes.y),
			//};

			//const int numIndices = 6;
			//ULONG* indices = new ULONG[numIndices]{
			//	0, 1, 2, 2, 1, 3
			//};

			//glm::vec2* texCoords = new glm::vec2[numVerts]{
			//	glm::vec2(0.f, 1.f),
			//	glm::vec2(0.f, 0.f),
			//	glm::vec2(1.f, 1.f),
			//	glm::vec2(1.f, 0.f)
			//};

			//glm::vec3* normals = new glm::vec3[numVerts]{
			//	glm::vec3(0.f, 1.f, 0.f),
			//	glm::vec3(0.f, 1.f, 0.f),
			//	glm::vec3(0.f, 1.f, 0.f),
			//	glm::vec3(0.f, 1.f, 0.f)
			//};

			Model::Data buildData;
			//buildData.numVertices = numVerts;
			//buildData.positions = positions;
			//buildData.texCoords = texCoords;
			//buildData.normals = normals;
			//buildData.numIndices = numIndices;
			//buildData.indices = indices;

			std::unique_ptr<Model> model = std::make_unique<Model>(buildData, shaderSet);

			return model;

		}
	};
}

