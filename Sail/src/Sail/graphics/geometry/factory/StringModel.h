#pragma once

#include <memory>
#include "FontMesh.h"
#include "../Model.h"

namespace ModelFactory {

	class StringModel {
	public:
		struct Constraints {
			Mesh::vec3 origin;
			Mesh::vec2 size;
			std::string text;
		};

	public:
		static Model* Create(Shader* shader, const StringModel::Constraints& constraints) {
			if (constraints.text.size() == 0) {
				SAIL_LOG("StringModel::Create : Tried to create a string model without any characters");
				return nullptr;
			}

			Mesh::vec2 charSize = Mesh::vec2(constraints.size.vec.x / static_cast<float>(constraints.text.size()), constraints.size.vec.y);
			Mesh::vec2 charHalfSize = Mesh::vec2(charSize.vec.x / 2.f, charSize.vec.y / 2.f);

			Model* stringModel = SAIL_NEW Model;
			MeshFactory::FontMesh::Constraints charConst;
			charConst.halfSize = charHalfSize;
			float offsetX = -constraints.size.vec.x / 2.f + charHalfSize.vec.x / 2.f;
			for (int i = 0; i < constraints.text.size(); i++) {
				charConst.origin = Mesh::vec3(constraints.origin.vec.x + offsetX, constraints.origin.vec.y, 0.f);
				charConst.character = constraints.text[i];
				stringModel->addMesh(MeshFactory::FontMesh::Create(shader, charConst));
				offsetX += charSize.vec.x;
			}

			return stringModel;
		}

	};
}
