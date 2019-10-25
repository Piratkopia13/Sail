#pragma once

#pragma once

#include <memory>
#include "../Model.h"

namespace ModelFactory {

	class FontModel {
	public:
		struct Constraints {
			Mesh::vec3 origin;
			Mesh::vec2 halfSize;
			char character;
		};
	public:

		static Model* Create(Shader* shader, const FontModel::Constraints& constraints) {

			Mesh::vec2 halfSizes(constraints.halfSize);
			Mesh::vec3 origin = constraints.origin;

			const int numVerts = 4;
			Mesh::vec3* positions = SAIL_NEW Mesh::vec3[numVerts]{
				Mesh::vec3(origin.vec.x - halfSizes.vec.x, origin.vec.y - halfSizes.vec.y, -0.1f),
				Mesh::vec3(origin.vec.x - halfSizes.vec.x, origin.vec.y + halfSizes.vec.y, -0.1f),
				Mesh::vec3(origin.vec.x + halfSizes.vec.x, origin.vec.y - halfSizes.vec.y, -0.1f),
				Mesh::vec3(origin.vec.x + halfSizes.vec.x, origin.vec.y + halfSizes.vec.y, -0.1f),
			};

			const int numIndices = 6;
			ULONG* indices = SAIL_NEW ULONG[numIndices]{
				0, 1, 2, 2, 1, 3
			};

			int indexX = 1;
			int indexY = 1;
			if (constraints.character == 'a') {
				indexX = 0;
				indexY = 0;
			}
			if (constraints.character == 'b') {
				indexX = 1;
				indexY = 0;
			}

			Mesh::vec2* texCoords = SAIL_NEW Mesh::vec2[numVerts]{
				Mesh::vec2((indexX * 0.1111f), (indexY * 0.1111f) + 0.1111f),
				Mesh::vec2((indexX * 0.1111f), (indexY * 0.1111f)),
				Mesh::vec2((indexX * 0.1111f) + 0.1111f, (indexY * 0.1111f) + 0.1111f),
				Mesh::vec2((indexX * 0.1111f) + 0.1111f, (indexY * 0.1111f))
			};

			Mesh::Data data;
			data.numVertices = numVerts;
			data.numIndices = numIndices;
			data.positions = positions;
			data.indices = indices;
			data.texCoords = texCoords;

			return SAIL_NEW Model(data, shader);

		}

	};
}
