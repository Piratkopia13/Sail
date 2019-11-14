#pragma once

#include <memory>
#include "Sail/api/Mesh.h"
#include "Sail/utils/GUISettings.h"

namespace MeshFactory {

	class FontMesh {
	public:
		struct Constraints {
			Mesh::vec3 origin;
			Mesh::vec2 halfSize;
			char character;
		};

	public:
		static std::unique_ptr<Mesh> Create(Shader* shader, const FontMesh::Constraints& constraints) {

			int indexX = 0;
			int indexY = 0;
			if (constraints.character > 64 && constraints.character < 91) {
				int index = static_cast<int>(constraints.character) - 65;
				indexX = index % GUIText::numX;
				indexY = index / GUIText::numX;
			} else {
				// Always print a space if the character isn't found
				indexX = GUIText::spacePosX;
				indexY = GUIText::spacePosY;
			}

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

			Mesh::vec2* texCoords = SAIL_NEW Mesh::vec2[numVerts]{
				Mesh::vec2((indexX * GUIText::charSize.x), (indexY * GUIText::charSize.y) + GUIText::charSize.y),
				Mesh::vec2((indexX * GUIText::charSize.x), (indexY * GUIText::charSize.y)),
				Mesh::vec2((indexX * GUIText::charSize.x) + GUIText::charSize.x, (indexY * GUIText::charSize.y) + GUIText::charSize.y),
				Mesh::vec2((indexX * GUIText::charSize.x) + GUIText::charSize.x, (indexY * GUIText::charSize.y))
			};

			Mesh::Data data;
			data.numVertices = numVerts;
			data.numIndices = numIndices;
			data.positions = positions;
			data.indices = indices;
			data.texCoords = texCoords;

			return std::unique_ptr<Mesh>(Mesh::Create(data, shader));

		}

	};
}
