#include "pch.h"
#include "AssimpLoader.h"

AssimpLoader::AssimpLoader() :
	m_importer() {

}

AssimpLoader::~AssimpLoader() {
	//delete m_importer;
}

Model* AssimpLoader::importModel(const std::string& path, Shader* shader) {
	Model* model = new Model();

	const aiScene* scene = m_importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_JoinIdenticalVertices);
	if ( errorCheck(scene) ) {
		return nullptr;
	}
	std::string name = scene->GetShortFilename(path.c_str());

	makeOffsets(scene);
	Mesh::Data meshData;
	for ( int i = 0; i < scene->mNumMeshes; i++ ) {
		meshData.numVertices += scene->mMeshes[i]->mNumVertices;
		meshData.numIndices += scene->mMeshes[i]->mNumFaces * 3; // assumes 3 indices per face
	}

	meshData.indices = SAIL_NEW unsigned long[meshData.numIndices];

	meshData.positions = SAIL_NEW Mesh::vec3[meshData.numVertices];
	meshData.normals = SAIL_NEW Mesh::vec3[meshData.numVertices];
	meshData.texCoords = SAIL_NEW Mesh::vec2[meshData.numVertices];
	meshData.tangents = SAIL_NEW Mesh::vec3[meshData.numVertices];
	meshData.bitangents = SAIL_NEW Mesh::vec3[meshData.numVertices];

	processNode(scene, scene->mRootNode, meshData);
	std::unique_ptr<Mesh> mesh = std::unique_ptr<Mesh>(Mesh::Create(meshData, shader));
	model->addMesh(std::move(mesh));

	clearData();

	return model;
}

AnimationStack* AssimpLoader::importAnimationStack(const std::string& path) {
	const aiScene* scene = m_importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_JoinIdenticalVertices);
	if ( errorCheck(scene) ) {
		return nullptr;
	}
	std::string name = scene->GetShortFilename(path.c_str());
	//processNode(scene, scene->mRootNode);

	size_t vertCount = 0;

	makeOffsets(scene);
	for ( size_t i = 0; i < scene->mNumMeshes; i++ ) {
		vertCount += scene->mMeshes[i]->mNumVertices;
	}
	AnimationStack* stack = new AnimationStack(vertCount);

	if ( !importBonesFromNode(scene, scene->mRootNode, stack) ) {
		Memory::SafeDelete(stack);
		return nullptr;
	}
	stack->checkWeights();

	if ( !importAnimations(scene, stack) ) {
		Memory::SafeDelete(stack);
		return nullptr;
	}
	clearData();
	return stack;
}

std::vector<Model*> AssimpLoader::importScene(const std::string& path, Shader* shader) {
	return std::vector<Model*>();
}




void AssimpLoader::processNode(const aiScene* scene, aiNode* node, Mesh::Data& meshData) {
	for ( int i = 0; i < node->mNumMeshes; i++ ) {
		getGeometry(scene->mMeshes[node->mMeshes[i]], meshData, m_meshOffsets[node->mMeshes[i]]);
		//getMaterial(pNode, mesh->getMaterial());
	}

	for ( int i = 0; i < node->mNumChildren; i++ ) {
		processNode(scene, node->mChildren[i], meshData);
	}
}

void AssimpLoader::getGeometry(aiMesh* mesh, Mesh::Data& buildData, AssimpLoader::MeshOffset& meshOffset) {
	if ( mesh->HasPositions() && mesh->HasNormals() ) {
		/*
			Vertices
		*/
		for ( int vertexIndex = 0; vertexIndex < mesh->mNumVertices; vertexIndex++ ) {
			if ( meshOffset.vertexOffset + vertexIndex > buildData.numVertices ) {
				Logger::Error("TOOO BIIIIIG VERTEX");
			}
			/*
				Positions
			*/
			buildData.positions[meshOffset.vertexOffset + vertexIndex].vec.x = mesh->mVertices[vertexIndex].x;
			buildData.positions[meshOffset.vertexOffset + vertexIndex].vec.y = mesh->mVertices[vertexIndex].y;
			buildData.positions[meshOffset.vertexOffset + vertexIndex].vec.z = mesh->mVertices[vertexIndex].z;

			/*
				Normals
			*/
			buildData.normals[meshOffset.vertexOffset + vertexIndex].vec.x = mesh->mNormals[vertexIndex].x;
			buildData.normals[meshOffset.vertexOffset + vertexIndex].vec.y = mesh->mNormals[vertexIndex].y;
			buildData.normals[meshOffset.vertexOffset + vertexIndex].vec.z = mesh->mNormals[vertexIndex].z;

			/*
				UVs
			*/
			buildData.texCoords[meshOffset.vertexOffset + vertexIndex].vec.x = mesh->mTextureCoords[0][vertexIndex].x;
			buildData.texCoords[meshOffset.vertexOffset + vertexIndex].vec.y = mesh->mTextureCoords[0][vertexIndex].y;


			/*
				Tangents and bitangents
			*/
			if ( mesh->HasTangentsAndBitangents() ) {
				/*
					Tangents
				*/
				buildData.tangents[meshOffset.vertexOffset + vertexIndex].vec.x = mesh->mTangents[vertexIndex].x;
				buildData.tangents[meshOffset.vertexOffset + vertexIndex].vec.y = mesh->mTangents[vertexIndex].y;
				buildData.tangents[meshOffset.vertexOffset + vertexIndex].vec.z = mesh->mTangents[vertexIndex].z;

				/*
					Bitangents
				*/
				buildData.bitangents[meshOffset.vertexOffset + vertexIndex].vec.x = mesh->mBitangents[vertexIndex].x;
				buildData.bitangents[meshOffset.vertexOffset + vertexIndex].vec.y = mesh->mBitangents[vertexIndex].y;
				buildData.bitangents[meshOffset.vertexOffset + vertexIndex].vec.z = mesh->mBitangents[vertexIndex].z;			
			}

		}

		/*
			Indices
		*/
		int vIndex = 0;
		for ( int faceIndex = 0; faceIndex < mesh->mNumFaces; faceIndex++ ) {
			for ( int iIndex = 0; iIndex < mesh->mFaces[faceIndex].mNumIndices; iIndex++ ) {
				if ( meshOffset.indexOffset + vIndex > buildData.numIndices ) {
					Logger::Error("TOOO BIIIIIG INDEX");
				}
				buildData.indices[meshOffset.indexOffset + vIndex++] = mesh->mFaces[faceIndex].mIndices[iIndex];
			}
		}

	} else {
		Logger::Error("Oh- oh!");
	}

}

Mesh* AssimpLoader::importMesh(const aiScene* scene, aiNode* node) {
	return nullptr;
}

bool AssimpLoader::importBonesFromNode(const aiScene* scene, aiNode* node, AnimationStack* stack) {
	Animation* animation = new Animation();


#ifdef _DEBUG 
	Logger::Log("1" + std::string(node->mName.C_Str()));
#endif
	for ( size_t nodeID = 0; nodeID < node->mNumMeshes; nodeID++ ) {
		const aiMesh* mesh = scene->mMeshes[node->mMeshes[nodeID]];

#ifdef _DEBUG 
		Logger::Log("2 " + std::string(mesh->mName.C_Str()));
#endif
		if ( mesh->HasBones() ) {
			for ( size_t boneID = 0; boneID < mesh->mNumBones; boneID++ ) {
				const aiBone* bone = mesh->mBones[boneID];

				std::string boneName = bone->mName.C_Str();
				size_t index = stack->m_boneMap.size();
				if ( stack->m_boneMap.find(boneName) == stack->m_boneMap.end() ) {
					stack->m_boneMap[boneName] = { index, mat4_cast(bone->mOffsetMatrix) };

				} else {
					index = stack->m_boneMap[boneName].index;
				}



#ifdef _DEBUG 
				Logger::Log("3  " + std::string(bone->mName.C_Str()));
#endif



				for ( size_t weightID = 0; weightID < bone->mNumWeights; weightID++ ) {
					const aiVertexWeight weight = bone->mWeights[weightID];
					stack->setConnectionData(m_meshOffsets[node->mMeshes[nodeID]].vertexOffset + weight.mVertexId, index, weight.mWeight);

				}


			}

		} else {
			Logger::Log("3  No Bones");
		}

	}




	int size = node->mNumChildren;
	for ( size_t i = 0; i < size; i++ ) {
		importBonesFromNode(scene, node->mChildren[i], stack);
	}


	return true;
}

bool AssimpLoader::importAnimations(const aiScene* scene, AnimationStack* stack) {
	if ( !scene->HasAnimations() ) {
		return false;
	}


	for ( size_t animationIndex = 0; animationIndex < scene->mNumAnimations; animationIndex++ ) {
		const aiAnimation* animation = scene->mAnimations[animationIndex];

		std::string temp = std::to_string(animation->mDuration) + ":" + std::to_string(animation->mTicksPerSecond) + " = " + std::to_string(animation->mDuration / animation->mTicksPerSecond);
		Logger::Log(temp);





	}






	return true;
}

//Animation* AssimpLoader::importAnimation(const aiScene* scene, aiNode* node) {
//
//	if (scene->HasAnimations()) {
//		for (int i = 0; i < scene->mNumAnimations; i++) {
//			std::string name = scene->mAnimations[i]->mName.C_Str();
//			Logger::Log(name);
//
//		}
//
//
//
//
//	}
//	return ;
//}

const bool AssimpLoader::errorCheck(const aiScene* scene) {
	if ( !scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode ) {
		Logger::Error("ERROR::ASSIMP::" + std::string(m_importer.GetErrorString()));
		return true;
	}
	return false;
}

void AssimpLoader::clearData() {
	m_meshOffsets.clear();

}

