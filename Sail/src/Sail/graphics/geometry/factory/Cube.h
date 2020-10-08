#pragma once

#include <memory>
#include "Sail/api/Mesh.h"

namespace MeshFactory {

	class Cube {
	public:
		static std::shared_ptr<Mesh> Create(const glm::vec3& halfSizes) {

			const int numVerts = 36;

			Mesh::vec3* positions = SAIL_NEW Mesh::vec3[numVerts]{
				Mesh::vec3(-halfSizes.x, -halfSizes.y, -halfSizes.z),
				Mesh::vec3(-halfSizes.x, halfSizes.y, -halfSizes.z),
				Mesh::vec3(halfSizes.x, -halfSizes.y, -halfSizes.z),
				Mesh::vec3(halfSizes.x, -halfSizes.y, -halfSizes.z),
				Mesh::vec3(-halfSizes.x, halfSizes.y, -halfSizes.z),
				Mesh::vec3(halfSizes.x, halfSizes.y, -halfSizes.z),

				Mesh::vec3(halfSizes.x, -halfSizes.y, -halfSizes.z),
				Mesh::vec3(halfSizes.x, halfSizes.y, -halfSizes.z),
				Mesh::vec3(halfSizes.x, -halfSizes.y, halfSizes.z),
				Mesh::vec3(halfSizes.x, -halfSizes.y, halfSizes.z),
				Mesh::vec3(halfSizes.x, halfSizes.y, -halfSizes.z),
				Mesh::vec3(halfSizes.x, halfSizes.y, halfSizes.z),

				Mesh::vec3(halfSizes.x, -halfSizes.y, halfSizes.z),
				Mesh::vec3(halfSizes.x, halfSizes.y, halfSizes.z),
				Mesh::vec3(-halfSizes.x, -halfSizes.y, halfSizes.z),
				Mesh::vec3(-halfSizes.x, -halfSizes.y, halfSizes.z),
				Mesh::vec3(halfSizes.x, halfSizes.y, halfSizes.z),
				Mesh::vec3(-halfSizes.x, halfSizes.y, halfSizes.z),

				Mesh::vec3(-halfSizes.x, -halfSizes.y, halfSizes.z),
				Mesh::vec3(-halfSizes.x, halfSizes.y, halfSizes.z),
				Mesh::vec3(-halfSizes.x, -halfSizes.y, -halfSizes.z),
				Mesh::vec3(-halfSizes.x, -halfSizes.y, -halfSizes.z),
				Mesh::vec3(-halfSizes.x, halfSizes.y, halfSizes.z),
				Mesh::vec3(-halfSizes.x, halfSizes.y, -halfSizes.z),

				Mesh::vec3(-halfSizes.x, halfSizes.y, -halfSizes.z),
				Mesh::vec3(-halfSizes.x, halfSizes.y, halfSizes.z),
				Mesh::vec3(halfSizes.x, halfSizes.y, -halfSizes.z),
				Mesh::vec3(halfSizes.x, halfSizes.y, -halfSizes.z),
				Mesh::vec3(-halfSizes.x, halfSizes.y, halfSizes.z),
				Mesh::vec3(halfSizes.x, halfSizes.y, halfSizes.z),

				Mesh::vec3(-halfSizes.x, -halfSizes.y, -halfSizes.z),
				Mesh::vec3(halfSizes.x, -halfSizes.y, -halfSizes.z),
				Mesh::vec3(-halfSizes.x, -halfSizes.y, halfSizes.z),
				Mesh::vec3(-halfSizes.x, -halfSizes.y, halfSizes.z),
				Mesh::vec3(halfSizes.x, -halfSizes.y, -halfSizes.z),
				Mesh::vec3(halfSizes.x, -halfSizes.y, halfSizes.z),
			};

			Mesh::vec2* texCoords = SAIL_NEW Mesh::vec2[numVerts]{
				// Neg z (front)
				Mesh::vec2(0.f, 1.0f),
				Mesh::vec2(0.f, 0.f),
				Mesh::vec2(1.f, 1.f),
				Mesh::vec2(1.f, 1.f),
				Mesh::vec2(0.f, 0.f),
				Mesh::vec2(1.f, 0.f),

				// Pos x (right)
				Mesh::vec2(0.99f, 0.99f),
				Mesh::vec2(0.99f, 0.99f),
				Mesh::vec2(0.99f, 0.99f),
				Mesh::vec2(0.99f, 0.99f),
				Mesh::vec2(0.99f, 0.99f),
				Mesh::vec2(0.99f, 0.99f),
				/*Mesh::vec2(0.f, 1.0f),
				Mesh::vec2(0.f, 0.f),
				Mesh::vec2(1.f, 1.f),
				Mesh::vec2(1.f, 1.f),
				Mesh::vec2(0.f, 0.f),
				Mesh::vec2(1.f, 0.f),*/

				// Pos z (back)
				Mesh::vec2(1.f, 1.0f),
				Mesh::vec2(1.f, 0.f),
				Mesh::vec2(0.f, 1.f),
				Mesh::vec2(0.f, 1.f),
				Mesh::vec2(1.f, 0.f),
				Mesh::vec2(0.f, 0.f),

				// Neg x (left)
				Mesh::vec2(0.f, 1.0f),
				Mesh::vec2(0.f, 0.f),
				Mesh::vec2(1.f, 1.f),
				Mesh::vec2(1.f, 1.f),
				Mesh::vec2(0.f, 0.f),
				Mesh::vec2(1.f, 0.f),

				// Pos y (up)
				Mesh::vec2(0.f, 1.0f),
				Mesh::vec2(0.f, 0.f),
				Mesh::vec2(1.f, 1.f),
				Mesh::vec2(1.f, 1.f),
				Mesh::vec2(0.f, 0.f),
				Mesh::vec2(1.f, 0.f),

				// Neg y (down)
				Mesh::vec2(0.f, 0.f),
				Mesh::vec2(1.f, 0.f),
				Mesh::vec2(0.f, 1.f),
				Mesh::vec2(0.f, 1.f),
				Mesh::vec2(1.f, 0.f),
				Mesh::vec2(1.f, 1.f),
			};

			Mesh::vec3* normals = SAIL_NEW Mesh::vec3[numVerts]{
				Mesh::vec3(0.f, 0.f, -1.f),
				Mesh::vec3(0.f, 0.f, -1.f),
				Mesh::vec3(0.f, 0.f, -1.f),
				Mesh::vec3(0.f, 0.f, -1.f),
				Mesh::vec3(0.f, 0.f, -1.f),
				Mesh::vec3(0.f, 0.f, -1.f),

				Mesh::vec3(1.f, 0.f, 0.f),
				Mesh::vec3(1.f, 0.f, 0.f),
				Mesh::vec3(1.f, 0.f, 0.f),
				Mesh::vec3(1.f, 0.f, 0.f),
				Mesh::vec3(1.f, 0.f, 0.f),
				Mesh::vec3(1.f, 0.f, 0.f),

				Mesh::vec3(0.f, 0.f, 1.0f),
				Mesh::vec3(0.f, 0.f, 1.0f),
				Mesh::vec3(0.f, 0.f, 1.0f),
				Mesh::vec3(0.f, 0.f, 1.0f),
				Mesh::vec3(0.f, 0.f, 1.0f),
				Mesh::vec3(0.f, 0.f, 1.0f),

				Mesh::vec3(-1.f, 0.f, 0.f),
				Mesh::vec3(-1.f, 0.f, 0.f),
				Mesh::vec3(-1.f, 0.f, 0.f),
				Mesh::vec3(-1.f, 0.f, 0.f),
				Mesh::vec3(-1.f, 0.f, 0.f),
				Mesh::vec3(-1.f, 0.f, 0.f),

				Mesh::vec3(0.f, 1.f, 0.f),
				Mesh::vec3(0.f, 1.f, 0.f),
				Mesh::vec3(0.f, 1.f, 0.f),
				Mesh::vec3(0.f, 1.f, 0.f),
				Mesh::vec3(0.f, 1.f, 0.f),
				Mesh::vec3(0.f, 1.f, 0.f),

				Mesh::vec3(0.f, -1.f, 0.f),
				Mesh::vec3(0.f, -1.f, 0.f),
				Mesh::vec3(0.f, -1.f, 0.f),
				Mesh::vec3(0.f, -1.f, 0.f),
				Mesh::vec3(0.f, -1.f, 0.f),
				Mesh::vec3(0.f, -1.f, 0.f)
			};

			Mesh::Data buildData;
			buildData.numVertices = numVerts;
			buildData.positions = positions;
			buildData.texCoords = texCoords;
			buildData.normals = normals;

			return std::shared_ptr<Mesh>(Mesh::Create(buildData));

		}
	};
}

