#pragma once

#include <memory>
#include "../Model.h"

namespace ModelFactory {

	class ConeModel {
	public:
		static std::unique_ptr<Model> Create(const glm::vec3& halfSizes, Shader* shader) {

			const int numVerts = 36;

			Mesh::vec3* positions = SAIL_NEW Mesh::vec3[numVerts]{
				Mesh::vec3(-halfSizes.x, -halfSizes.y, -halfSizes.z),
				Mesh::vec3(-halfSizes.x, halfSizes.y, -halfSizes.z),
				Mesh::vec3(halfSizes.x, 0.f, 0.f),
				Mesh::vec3(halfSizes.x, 0.f, 0.f),
				Mesh::vec3(-halfSizes.x, halfSizes.y, -halfSizes.z),
				Mesh::vec3(halfSizes.x, 0.f, 0.f),

				Mesh::vec3(halfSizes.x, 0.f, 0.f),
				Mesh::vec3(halfSizes.x, 0.f, 0.f),
				Mesh::vec3(halfSizes.x, 0.f, 0.f),
				Mesh::vec3(halfSizes.x, 0.f, 0.f),
				Mesh::vec3(halfSizes.x, 0.f, 0.f),
				Mesh::vec3(halfSizes.x, 0.f, 0.f),

				Mesh::vec3(halfSizes.x, 0.f, 0.f),
				Mesh::vec3(halfSizes.x, 0.f, 0.f),
				Mesh::vec3(-halfSizes.x, -halfSizes.y, halfSizes.z),
				Mesh::vec3(-halfSizes.x, -halfSizes.y, halfSizes.z),
				Mesh::vec3(halfSizes.x, 0.f, 0.f),
				Mesh::vec3(-halfSizes.x, halfSizes.y, halfSizes.z),

				Mesh::vec3(-halfSizes.x, -halfSizes.y, halfSizes.z),
				Mesh::vec3(-halfSizes.x, halfSizes.y, halfSizes.z),
				Mesh::vec3(-halfSizes.x, -halfSizes.y, -halfSizes.z),
				Mesh::vec3(-halfSizes.x, -halfSizes.y, -halfSizes.z),
				Mesh::vec3(-halfSizes.x, halfSizes.y, halfSizes.z),
				Mesh::vec3(-halfSizes.x, halfSizes.y, -halfSizes.z),

				Mesh::vec3(-halfSizes.x, halfSizes.y, -halfSizes.z),
				Mesh::vec3(-halfSizes.x, halfSizes.y, halfSizes.z),
				Mesh::vec3(halfSizes.x, 0.f, 0.f),
				Mesh::vec3(halfSizes.x, 0.f, 0.f),
				Mesh::vec3(-halfSizes.x, halfSizes.y, halfSizes.z),
				Mesh::vec3(halfSizes.x, 0.f, 0.f),

				Mesh::vec3(-halfSizes.x, -halfSizes.y, -halfSizes.z),
				Mesh::vec3(halfSizes.x, 0.f, 0.f),
				Mesh::vec3(-halfSizes.x, -halfSizes.y, halfSizes.z),
				Mesh::vec3(-halfSizes.x, -halfSizes.y, halfSizes.z),
				Mesh::vec3(halfSizes.x, 0.f, 0.f),
				Mesh::vec3(halfSizes.x, 0.f, 0.f),
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
				Mesh::vec2(1.f, 1.f),
				Mesh::vec2(1.f, 1.f),
				Mesh::vec2(1.f, 1.f),
				Mesh::vec2(1.f, 1.f),
				Mesh::vec2(1.f, 1.f),
				Mesh::vec2(1.f, 1.f),
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

			std::unique_ptr<Model> model = std::make_unique<Model>(buildData, shader);

			return model;

		}
	};
}

