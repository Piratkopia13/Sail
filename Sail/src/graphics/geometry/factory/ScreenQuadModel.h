#pragma once

#include <memory>
#include "PlaneModel.h"

namespace ModelFactory {

	using namespace DirectX::SimpleMath;

	class ScreenQuadModel {
	public:
		static std::unique_ptr<Model> Create(ShaderSet* shaderSet) {

			//ShaderSet* shaderSet = Application::getResourceManager().getShaderSet<>();

			Vector2 halfSizes(1.f, 1.f);

			const int numVerts = 4;
			Vector3* positions = new Vector3[numVerts]{
				Vector3(-halfSizes.x, -halfSizes.y, 0.f),
				Vector3(-halfSizes.x, halfSizes.y, 0.f),
				Vector3(halfSizes.x, -halfSizes.y, 0.f),
				Vector3(halfSizes.x, halfSizes.y, 0.f),
			};

			const int numIndices = 6;
			ULONG* indices = new ULONG[numIndices]{
				0, 1, 2, 2, 1, 3
			};

			Vector2* texCoords = new Vector2[numVerts]{
				Vector2(0.f, 1.f),
				Vector2(0.f, 0.f),
				Vector2(1.f, 1.f),
				Vector2(1.f, 0.f)
			};

			Mesh::Data data;
			data.numVertices = numVerts;
			data.numIndices = numIndices;
			data.positions = positions;
			data.indices = indices;
			data.texCoords = texCoords;

			return std::make_unique<Model>(data, shaderSet);

		}
	};
}

