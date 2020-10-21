#pragma once

#include <memory>
#include "Sail/api/Mesh.h"

namespace MeshFactory {

	class ScreenQuad {
	public:
		static std::unique_ptr<Mesh> Create() {
			glm::vec2 halfSizes(1.f, 1.f);

			const int numVerts = 4;
			Mesh::vec3* positions = SAIL_NEW Mesh::vec3[numVerts]{
				Mesh::vec3(-halfSizes.x, -halfSizes.y, 0.f),
				Mesh::vec3(-halfSizes.x, halfSizes.y, 0.f),
				Mesh::vec3(halfSizes.x, -halfSizes.y, 0.f),
				Mesh::vec3(halfSizes.x, halfSizes.y, 0.f),
			};

			const int numIndices = 6;
			auto* indices = SAIL_NEW uint32_t[numIndices]{
				0, 1, 2, 2, 1, 3
			};

			Mesh::vec2* texCoords = SAIL_NEW Mesh::vec2[numVerts]{
				Mesh::vec2(0.f, 1.f),
				Mesh::vec2(0.f, 0.f),
				Mesh::vec2(1.f, 1.f),
				Mesh::vec2(1.f, 0.f)
			};

			Mesh::Data data;
			data.numVertices = numVerts;
			data.numIndices = numIndices;
			data.positions = positions;
			data.indices = indices;
			data.texCoords = texCoords;

			return std::unique_ptr<Mesh>(Mesh::Create(data));
		}
	};
	
}

