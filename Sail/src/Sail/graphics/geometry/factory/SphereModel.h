#pragma once

#include <memory>
#include "../Model.h"

namespace ModelFactory {

	using namespace DirectX::SimpleMath;

	class SphereModel {
	public:
		static std::unique_ptr<Model> Create(float radius, float numFaces, ShaderSet* shaderSet) {

			//const int numVerts = 4;
			//Vector3* positions = new Vector3[numVerts]{
			//	Vector3(-halfSizes.x, 0.f, -halfSizes.y),
			//	Vector3(-halfSizes.x, 0.f, halfSizes.y),
			//	Vector3(halfSizes.x, 0.f, -halfSizes.y),
			//	//Vector3(halfSizes.x, 0.f, -halfSizes.y),
			//	//Vector3(-halfSizes.x, 0.f, halfSizes.y),
			//	Vector3(halfSizes.x, 0.f, halfSizes.y),
			//};

			//const int numIndices = 6;
			//ULONG* indices = new ULONG[numIndices]{
			//	0, 1, 2, 2, 1, 3
			//};

			//Vector2* texCoords = new Vector2[numVerts]{
			//	Vector2(0.f, 1.f),
			//	Vector2(0.f, 0.f),
			//	Vector2(1.f, 1.f),
			//	Vector2(1.f, 0.f)
			//};

			//Vector3* normals = new Vector3[numVerts]{
			//	Vector3(0.f, 1.f, 0.f),
			//	Vector3(0.f, 1.f, 0.f),
			//	Vector3(0.f, 1.f, 0.f),
			//	Vector3(0.f, 1.f, 0.f)
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

