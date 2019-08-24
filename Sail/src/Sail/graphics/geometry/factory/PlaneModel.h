#pragma once

#include <memory>
#include "../Model.h"

namespace ModelFactory {

	class PlaneModel {
	public:
		static std::unique_ptr<Model> Create(const glm::vec2& halfSizes, ShaderPipeline* shaderSet, const glm::vec2& texCoordScale = glm::vec2(1.f)) {

			const int numVerts = 4;
			Mesh::vec3* positions = SAIL_NEW Mesh::vec3[numVerts]{
				Mesh::vec3(-halfSizes.x, 0.f, -halfSizes.y),
				Mesh::vec3(-halfSizes.x, 0.f, halfSizes.y),
				Mesh::vec3(halfSizes.x, 0.f, -halfSizes.y),
				//Mesh::vec3(halfSizes.x, 0.f, -halfSizes.y),
				//Mesh::vec3(-halfSizes.x, 0.f, halfSizes.y),
				Mesh::vec3(halfSizes.x, 0.f, halfSizes.y),
			};

			const int numIndices = 6;
			ULONG* indices = SAIL_NEW ULONG[numIndices]{
				0, 1, 2, 2, 1, 3
			};

			Mesh::vec2* texCoords = SAIL_NEW Mesh::vec2[numVerts]{
				Mesh::vec2(0.f, texCoordScale.y),
				Mesh::vec2(0.f, 0.f),
				Mesh::vec2(texCoordScale.x, texCoordScale.y),
				Mesh::vec2(texCoordScale.x, 0.f)
			};

			Mesh::vec3* normals = SAIL_NEW Mesh::vec3[numVerts]{
				Mesh::vec3(0.f, 1.f, 0.f),
				Mesh::vec3(0.f, 1.f, 0.f),
				Mesh::vec3(0.f, 1.f, 0.f),
				Mesh::vec3(0.f, 1.f, 0.f)
			};

			Mesh::vec3* tangents = SAIL_NEW Mesh::vec3[numVerts]{
				Mesh::vec3(0.f, 0.f, 1.f),
				Mesh::vec3(0.f, 0.f, 1.f),
				Mesh::vec3(0.f, 0.f, 1.f),
				Mesh::vec3(0.f, 0.f, 1.f)

			};

			Mesh::vec3* bitangents = SAIL_NEW Mesh::vec3[numVerts]{
				Mesh::vec3(1.f, 0.f, 0.f),
				Mesh::vec3(1.f, 0.f, 0.f),
				Mesh::vec3(1.f, 0.f, 0.f),
				Mesh::vec3(1.f, 0.f, 0.f)
			};

			Mesh::Data buildData;
			buildData.numVertices = numVerts;
			buildData.positions = positions;
			buildData.texCoords = texCoords;
			buildData.normals = normals;
			buildData.tangents = tangents;
			buildData.bitangents = bitangents;
			buildData.numIndices = numIndices;
			buildData.indices = indices;

			std::unique_ptr<Model> model = std::make_unique<Model>(buildData, shaderSet);

			return model;

		}
	};
}

