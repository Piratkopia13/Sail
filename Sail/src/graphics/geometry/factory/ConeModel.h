#pragma once

#include <memory>
#include "../Model.h"

namespace ModelFactory {

	using namespace DirectX::SimpleMath;

	class ConeModel {
	public:
		static std::unique_ptr<Model> Create(const DirectX::SimpleMath::Vector3& halfSizes, ShaderSet* shaderSet) {

			const int numVerts = 36;

			Vector3* positions = new Vector3[numVerts]{
				Vector3(-halfSizes.x, -halfSizes.y, -halfSizes.z),
				Vector3(-halfSizes.x, halfSizes.y, -halfSizes.z),
				Vector3(halfSizes.x, 0.f, 0.f),
				Vector3(halfSizes.x, 0.f, 0.f),
				Vector3(-halfSizes.x, halfSizes.y, -halfSizes.z),
				Vector3(halfSizes.x, 0.f, 0.f),

				Vector3(halfSizes.x, 0.f, 0.f),
				Vector3(halfSizes.x, 0.f, 0.f),
				Vector3(halfSizes.x, 0.f, 0.f),
				Vector3(halfSizes.x, 0.f, 0.f),
				Vector3(halfSizes.x, 0.f, 0.f),
				Vector3(halfSizes.x, 0.f, 0.f),

				Vector3(halfSizes.x, 0.f, 0.f),
				Vector3(halfSizes.x, 0.f, 0.f),
				Vector3(-halfSizes.x, -halfSizes.y, halfSizes.z),
				Vector3(-halfSizes.x, -halfSizes.y, halfSizes.z),
				Vector3(halfSizes.x, 0.f, 0.f),
				Vector3(-halfSizes.x, halfSizes.y, halfSizes.z),

				Vector3(-halfSizes.x, -halfSizes.y, halfSizes.z),
				Vector3(-halfSizes.x, halfSizes.y, halfSizes.z),
				Vector3(-halfSizes.x, -halfSizes.y, -halfSizes.z),
				Vector3(-halfSizes.x, -halfSizes.y, -halfSizes.z),
				Vector3(-halfSizes.x, halfSizes.y, halfSizes.z),
				Vector3(-halfSizes.x, halfSizes.y, -halfSizes.z),

				Vector3(-halfSizes.x, halfSizes.y, -halfSizes.z),
				Vector3(-halfSizes.x, halfSizes.y, halfSizes.z),
				Vector3(halfSizes.x, 0.f, 0.f),
				Vector3(halfSizes.x, 0.f, 0.f),
				Vector3(-halfSizes.x, halfSizes.y, halfSizes.z),
				Vector3(halfSizes.x, 0.f, 0.f),

				Vector3(-halfSizes.x, -halfSizes.y, -halfSizes.z),
				Vector3(halfSizes.x, 0.f, 0.f),
				Vector3(-halfSizes.x, -halfSizes.y, halfSizes.z),
				Vector3(-halfSizes.x, -halfSizes.y, halfSizes.z),
				Vector3(halfSizes.x, 0.f, 0.f),
				Vector3(halfSizes.x, 0.f, 0.f),
			};

			Vector2* texCoords = new Vector2[numVerts]{
				// Neg z (front)
				Vector2(0.f, 1.0f),
				Vector2(0.f, 0.f),
				Vector2(1.f, 1.f),
				Vector2(1.f, 1.f),
				Vector2(0.f, 0.f),
				Vector2(1.f, 0.f),

				// Pos x (right)
				Vector2(1.f, 1.f),
				Vector2(1.f, 1.f),
				Vector2(1.f, 1.f),
				Vector2(1.f, 1.f),
				Vector2(1.f, 1.f),
				Vector2(1.f, 1.f),
				/*Vector2(0.f, 1.0f),
				Vector2(0.f, 0.f),
				Vector2(1.f, 1.f),
				Vector2(1.f, 1.f),
				Vector2(0.f, 0.f),
				Vector2(1.f, 0.f),*/

				// Pos z (back)
				Vector2(1.f, 1.0f),
				Vector2(1.f, 0.f),
				Vector2(0.f, 1.f),
				Vector2(0.f, 1.f),
				Vector2(1.f, 0.f),
				Vector2(0.f, 0.f),

				// Neg x (left)
				Vector2(0.f, 1.0f),
				Vector2(0.f, 0.f),
				Vector2(1.f, 1.f),
				Vector2(1.f, 1.f),
				Vector2(0.f, 0.f),
				Vector2(1.f, 0.f),

				// Pos y (up)
				Vector2(0.f, 1.0f),
				Vector2(0.f, 0.f),
				Vector2(1.f, 1.f),
				Vector2(1.f, 1.f),
				Vector2(0.f, 0.f),
				Vector2(1.f, 0.f),

				// Neg y (down)
				Vector2(0.f, 0.f),
				Vector2(1.f, 0.f),
				Vector2(0.f, 1.f),
				Vector2(0.f, 1.f),
				Vector2(1.f, 0.f),
				Vector2(1.f, 1.f),
			};

			Vector3* normals = new Vector3[numVerts]{
				Vector3(0.f, 0.f, -1.f),
				Vector3(0.f, 0.f, -1.f),
				Vector3(0.f, 0.f, -1.f),
				Vector3(0.f, 0.f, -1.f),
				Vector3(0.f, 0.f, -1.f),
				Vector3(0.f, 0.f, -1.f),

				Vector3(1.f, 0.f, 0.f),
				Vector3(1.f, 0.f, 0.f),
				Vector3(1.f, 0.f, 0.f),
				Vector3(1.f, 0.f, 0.f),
				Vector3(1.f, 0.f, 0.f),
				Vector3(1.f, 0.f, 0.f),

				Vector3(0.f, 0.f, 1.0f),
				Vector3(0.f, 0.f, 1.0f),
				Vector3(0.f, 0.f, 1.0f),
				Vector3(0.f, 0.f, 1.0f),
				Vector3(0.f, 0.f, 1.0f),
				Vector3(0.f, 0.f, 1.0f),

				Vector3(-1.f, 0.f, 0.f),
				Vector3(-1.f, 0.f, 0.f),
				Vector3(-1.f, 0.f, 0.f),
				Vector3(-1.f, 0.f, 0.f),
				Vector3(-1.f, 0.f, 0.f),
				Vector3(-1.f, 0.f, 0.f),

				Vector3(0.f, 1.f, 0.f),
				Vector3(0.f, 1.f, 0.f),
				Vector3(0.f, 1.f, 0.f),
				Vector3(0.f, 1.f, 0.f),
				Vector3(0.f, 1.f, 0.f),
				Vector3(0.f, 1.f, 0.f),

				Vector3(0.f, -1.f, 0.f),
				Vector3(0.f, -1.f, 0.f),
				Vector3(0.f, -1.f, 0.f),
				Vector3(0.f, -1.f, 0.f),
				Vector3(0.f, -1.f, 0.f),
				Vector3(0.f, -1.f, 0.f)
			};

			Model::Data buildData;
			buildData.numVertices = numVerts;
			buildData.positions = positions;
			buildData.texCoords = texCoords;
			buildData.normals = normals;

			std::unique_ptr<Model> model = std::make_unique<Model>(buildData, shaderSet);

			return model;

		}
	};
}

