#pragma once

#include <memory>
#include "../Model.h"

namespace ModelFactory {

	class CubeModel {
	public:
		static std::unique_ptr<Model> Create(const glm::vec3& halfSizes, ShaderSet* shaderSet) {

			const int numVerts = 36;

			glm::vec3* positions = new glm::vec3[numVerts]{
				glm::vec3(-halfSizes.x, -halfSizes.y, -halfSizes.z),
				glm::vec3(-halfSizes.x, halfSizes.y, -halfSizes.z),
				glm::vec3(halfSizes.x, -halfSizes.y, -halfSizes.z),
				glm::vec3(halfSizes.x, -halfSizes.y, -halfSizes.z),
				glm::vec3(-halfSizes.x, halfSizes.y, -halfSizes.z),
				glm::vec3(halfSizes.x, halfSizes.y, -halfSizes.z),

				glm::vec3(halfSizes.x, -halfSizes.y, -halfSizes.z),
				glm::vec3(halfSizes.x, halfSizes.y, -halfSizes.z),
				glm::vec3(halfSizes.x, -halfSizes.y, halfSizes.z),
				glm::vec3(halfSizes.x, -halfSizes.y, halfSizes.z),
				glm::vec3(halfSizes.x, halfSizes.y, -halfSizes.z),
				glm::vec3(halfSizes.x, halfSizes.y, halfSizes.z),

				glm::vec3(halfSizes.x, -halfSizes.y, halfSizes.z),
				glm::vec3(halfSizes.x, halfSizes.y, halfSizes.z),
				glm::vec3(-halfSizes.x, -halfSizes.y, halfSizes.z),
				glm::vec3(-halfSizes.x, -halfSizes.y, halfSizes.z),
				glm::vec3(halfSizes.x, halfSizes.y, halfSizes.z),
				glm::vec3(-halfSizes.x, halfSizes.y, halfSizes.z),

				glm::vec3(-halfSizes.x, -halfSizes.y, halfSizes.z),
				glm::vec3(-halfSizes.x, halfSizes.y, halfSizes.z),
				glm::vec3(-halfSizes.x, -halfSizes.y, -halfSizes.z),
				glm::vec3(-halfSizes.x, -halfSizes.y, -halfSizes.z),
				glm::vec3(-halfSizes.x, halfSizes.y, halfSizes.z),
				glm::vec3(-halfSizes.x, halfSizes.y, -halfSizes.z),

				glm::vec3(-halfSizes.x, halfSizes.y, -halfSizes.z),
				glm::vec3(-halfSizes.x, halfSizes.y, halfSizes.z),
				glm::vec3(halfSizes.x, halfSizes.y, -halfSizes.z),
				glm::vec3(halfSizes.x, halfSizes.y, -halfSizes.z),
				glm::vec3(-halfSizes.x, halfSizes.y, halfSizes.z),
				glm::vec3(halfSizes.x, halfSizes.y, halfSizes.z),

				glm::vec3(-halfSizes.x, -halfSizes.y, -halfSizes.z),
				glm::vec3(halfSizes.x, -halfSizes.y, -halfSizes.z),
				glm::vec3(-halfSizes.x, -halfSizes.y, halfSizes.z),
				glm::vec3(-halfSizes.x, -halfSizes.y, halfSizes.z),
				glm::vec3(halfSizes.x, -halfSizes.y, -halfSizes.z),
				glm::vec3(halfSizes.x, -halfSizes.y, halfSizes.z),
			};

			glm::vec2* texCoords = new glm::vec2[numVerts]{
				// Neg z (front)
				glm::vec2(0.f, 1.0f),
				glm::vec2(0.f, 0.f),
				glm::vec2(1.f, 1.f),
				glm::vec2(1.f, 1.f),
				glm::vec2(0.f, 0.f),
				glm::vec2(1.f, 0.f),

				// Pos x (right)
				glm::vec2(0.99f, 0.99f),
				glm::vec2(0.99f, 0.99f),
				glm::vec2(0.99f, 0.99f),
				glm::vec2(0.99f, 0.99f),
				glm::vec2(0.99f, 0.99f),
				glm::vec2(0.99f, 0.99f),
				/*glm::vec2(0.f, 1.0f),
				glm::vec2(0.f, 0.f),
				glm::vec2(1.f, 1.f),
				glm::vec2(1.f, 1.f),
				glm::vec2(0.f, 0.f),
				glm::vec2(1.f, 0.f),*/

				// Pos z (back)
				glm::vec2(1.f, 1.0f),
				glm::vec2(1.f, 0.f),
				glm::vec2(0.f, 1.f),
				glm::vec2(0.f, 1.f),
				glm::vec2(1.f, 0.f),
				glm::vec2(0.f, 0.f),

				// Neg x (left)
				glm::vec2(0.f, 1.0f),
				glm::vec2(0.f, 0.f),
				glm::vec2(1.f, 1.f),
				glm::vec2(1.f, 1.f),
				glm::vec2(0.f, 0.f),
				glm::vec2(1.f, 0.f),

				// Pos y (up)
				glm::vec2(0.f, 1.0f),
				glm::vec2(0.f, 0.f),
				glm::vec2(1.f, 1.f),
				glm::vec2(1.f, 1.f),
				glm::vec2(0.f, 0.f),
				glm::vec2(1.f, 0.f),

				// Neg y (down)
				glm::vec2(0.f, 0.f),
				glm::vec2(1.f, 0.f),
				glm::vec2(0.f, 1.f),
				glm::vec2(0.f, 1.f),
				glm::vec2(1.f, 0.f),
				glm::vec2(1.f, 1.f),
			};

			glm::vec3* normals = new glm::vec3[numVerts]{
				glm::vec3(0.f, 0.f, -1.f),
				glm::vec3(0.f, 0.f, -1.f),
				glm::vec3(0.f, 0.f, -1.f),
				glm::vec3(0.f, 0.f, -1.f),
				glm::vec3(0.f, 0.f, -1.f),
				glm::vec3(0.f, 0.f, -1.f),

				glm::vec3(1.f, 0.f, 0.f),
				glm::vec3(1.f, 0.f, 0.f),
				glm::vec3(1.f, 0.f, 0.f),
				glm::vec3(1.f, 0.f, 0.f),
				glm::vec3(1.f, 0.f, 0.f),
				glm::vec3(1.f, 0.f, 0.f),

				glm::vec3(0.f, 0.f, 1.0f),
				glm::vec3(0.f, 0.f, 1.0f),
				glm::vec3(0.f, 0.f, 1.0f),
				glm::vec3(0.f, 0.f, 1.0f),
				glm::vec3(0.f, 0.f, 1.0f),
				glm::vec3(0.f, 0.f, 1.0f),

				glm::vec3(-1.f, 0.f, 0.f),
				glm::vec3(-1.f, 0.f, 0.f),
				glm::vec3(-1.f, 0.f, 0.f),
				glm::vec3(-1.f, 0.f, 0.f),
				glm::vec3(-1.f, 0.f, 0.f),
				glm::vec3(-1.f, 0.f, 0.f),

				glm::vec3(0.f, 1.f, 0.f),
				glm::vec3(0.f, 1.f, 0.f),
				glm::vec3(0.f, 1.f, 0.f),
				glm::vec3(0.f, 1.f, 0.f),
				glm::vec3(0.f, 1.f, 0.f),
				glm::vec3(0.f, 1.f, 0.f),

				glm::vec3(0.f, -1.f, 0.f),
				glm::vec3(0.f, -1.f, 0.f),
				glm::vec3(0.f, -1.f, 0.f),
				glm::vec3(0.f, -1.f, 0.f),
				glm::vec3(0.f, -1.f, 0.f),
				glm::vec3(0.f, -1.f, 0.f)
			};

			Mesh::Data buildData;
			buildData.numVertices = numVerts;
			buildData.positions = positions;
			buildData.texCoords = texCoords;
			buildData.normals = normals;

			std::unique_ptr<Model> model = std::make_unique<Model>(buildData, shaderSet);

			return model;

		}
	};
}

