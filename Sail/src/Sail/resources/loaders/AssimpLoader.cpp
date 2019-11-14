#include "pch.h"
#include "AssimpLoader.h"

AssimpLoader::AssimpLoader() :
	m_importer() {

}

AssimpLoader::~AssimpLoader() {

}

Model* AssimpLoader::importModel(const std::string& path, Shader* shader) {

	const aiScene* scene = m_importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs  | aiProcess_CalcTangentSpace);
	if ( errorCheck(scene) ) {
		return nullptr;
	}
	std::string name = scene->GetShortFilename(path.c_str());

	makeOffsets(scene);
	Mesh::Data meshData;

	for ( unsigned int i = 0; i < scene->mNumMeshes; i++ ) {
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
	Model* model = SAIL_NEW Model();
	model->addMesh(std::move(mesh));

	clearData();

	return model;
}

AnimationStack* AssimpLoader::importAnimationStack(const std::string& path) {
	const aiScene* scene = m_importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs );
	if ( errorCheck(scene) ) {
		return nullptr;
	}
	std::string name = scene->GetShortFilename(path.c_str());
	//processNode(scene, scene->mRootNode);

	unsigned int vertCount = 0;

	//	__________________________________MATRIX______________________________
	m_globalTransform = glm::inverse(mat4_cast(scene->mRootNode->mTransformation));
	//	________________glm::inverse_______________________________________________________


	makeOffsets(scene);
	for (unsigned int i = 0; i < scene->mNumMeshes; i++) {
		vertCount += scene->mMeshes[i]->mNumVertices;
	}
	AnimationStack* stack = SAIL_NEW AnimationStack(vertCount);
	
	if (!importBonesFromNode(scene, scene->mRootNode, stack)) {
		Memory::SafeDelete(stack);
		clearData();
		return nullptr;
	}
	stack->checkWeights();

	if ( !importAnimations(scene, stack) ) {
		Memory::SafeDelete(stack);
		clearData();
		return nullptr;
	}
	clearData();
	return stack;
}

std::vector<Model*> AssimpLoader::importScene(const std::string& path, Shader* shader) {
	return std::vector<Model*>();
}




void AssimpLoader::processNode(const aiScene* scene, aiNode* node, Mesh::Data& meshData) {
	for (unsigned int i = 0; i < node->mNumMeshes; i++ ) {
		getGeometry(scene->mMeshes[node->mMeshes[i]], meshData, m_meshOffsets[node->mMeshes[i]]);
		//getMaterial(pNode, mesh->getMaterial());
	}

	for (unsigned int i = 0; i < node->mNumChildren; i++ ) {
		processNode(scene, node->mChildren[i], meshData);
	}
}

void AssimpLoader::getGeometry(aiMesh* mesh, Mesh::Data& buildData, AssimpLoader::MeshOffset& meshOffset) {
	if ( mesh->HasPositions() && mesh->HasNormals() ) {
		/*
			Vertices
		*/
		for ( unsigned int vertexIndex = 0; vertexIndex < mesh->mNumVertices; vertexIndex++ ) {
			if ( meshOffset.vertexOffset + vertexIndex > buildData.numVertices ) {
				SAIL_LOG_ERROR("TOOO BIIIIIG VERTEX");
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
		for (unsigned int faceIndex = 0; faceIndex < mesh->mNumFaces; faceIndex++ ) {
			for (unsigned int iIndex = 0; iIndex < mesh->mFaces[faceIndex].mNumIndices; iIndex++ ) {
				if ( meshOffset.indexOffset + vIndex > buildData.numIndices ) {
					SAIL_LOG_ERROR("TOOO BIIIIIG INDEX");
				}
				buildData.indices[meshOffset.indexOffset + vIndex++] = mesh->mFaces[faceIndex].mIndices[iIndex];
			}
		}

	} else {
		SAIL_LOG_ERROR("Oh- oh!");
	}

}

Mesh* AssimpLoader::importMesh(const aiScene* scene, aiNode* node) {
	return nullptr;
}

bool AssimpLoader::importBonesFromNode(const aiScene* scene, aiNode* node, AnimationStack* stack) {
	#ifdef _DEBUG 
	SAIL_LOG("1"+std::string(node->mName.C_Str())); 
		if (m_nodes.find(node->mName.C_Str()) == m_nodes.end()) {
			m_nodes[node->mName.C_Str()] = node;
		}
	#endif
	for (unsigned int nodeID = 0; nodeID < node->mNumMeshes; nodeID++) {
		const aiMesh* mesh = scene->mMeshes[node->mMeshes[nodeID]];
		
			#ifdef _DEBUG 
		SAIL_LOG("2 "+std::string(mesh->mName.C_Str()));
			#endif
		if (mesh->HasBones()) {
			for (unsigned int boneID = 0; boneID < mesh->mNumBones; boneID++) {
				const aiBone* bone = mesh->mBones[boneID];

				std::string boneName = bone->mName.C_Str();
				unsigned int index = (unsigned int)m_boneMap.size();
				if (m_boneMap.find(boneName) == m_boneMap.end()) {


					m_boneMap[boneName] = { index, node->mName.C_Str(), (mat4_cast(bone->mOffsetMatrix))};
					
				}
				else {
					index = m_boneMap[boneName].index;
				}



#ifdef _DEBUG 
				SAIL_LOG("3  " + std::string(bone->mName.C_Str()));
#endif



				for (unsigned int weightID = 0; weightID < bone->mNumWeights; weightID++) {
					const aiVertexWeight weight = bone->mWeights[weightID];
					stack->setConnectionData(m_meshOffsets[node->mMeshes[nodeID]].vertexOffset + weight.mVertexId, index, weight.mWeight);

				}


			}

		} else {
			SAIL_LOG("3  No Bones");
		}

	}




	unsigned int size = node->mNumChildren;
	for (unsigned int i = 0; i < size; i++) {
		importBonesFromNode(scene, node->mChildren[i], stack);
	}


	return true;
}

bool AssimpLoader::importAnimations(const aiScene* scene, AnimationStack* stack) {
	if ( !scene->HasAnimations() ) {
		return false;
	}

	//debug
#ifdef _DEBUG
	std::vector<const aiAnimation*> animationz;
	std::vector<std::vector<const aiNodeAnim*>> channels;
#endif
	mapChannels(scene);

	for (unsigned int animationIndex = 0; animationIndex < scene->mNumAnimations; animationIndex++) {
		const aiAnimation* animation = scene->mAnimations[animationIndex];
		Animation* anim = SAIL_NEW Animation();
		
#ifdef _DEBUG
		animationz.emplace_back(animation);
		channels.emplace_back(std::vector<const aiNodeAnim*>());
		for (unsigned int i = 0; i < animation->mNumChannels; i++) {
			channels.back().emplace_back(animation->mChannels[i]);
		}
		std::string temp = std::to_string(animation->mDuration) + ":" + std::to_string(animation->mTicksPerSecond) + " = " + std::to_string(animation->mDuration / animation->mTicksPerSecond);
		SAIL_LOG(temp);
#endif

		unsigned int totalFrames = (unsigned int)animation->mDuration;
		SAIL_LOG(std::to_string(totalFrames));
		float totalDivided = 1.0f / (float)totalFrames;
		float tickRate = 1.0f/(animation->mTicksPerSecond == 0 ? 24.0f : (float)animation->mTicksPerSecond);
		float totalTime = totalFrames * (animation->mTicksPerSecond == 0 ? 24.0f : (float)animation->mTicksPerSecond);
		for (unsigned int frame = 0; frame < totalFrames; frame++) {
			float time = (float)frame * tickRate;
			Animation::Frame* currentFrame = SAIL_NEW Animation::Frame((unsigned int)m_boneMap.size());
			//SAIL_LOG("Added Frame with ");
			
			readNodeHierarchy(animationIndex, frame, frame, scene->mRootNode, glm::identity<glm::mat4>(), currentFrame);
			anim->addFrame(frame, time, currentFrame);
		}

		stack->addAnimation(animation->mName.C_Str(), anim);
	}
	


	return true;
}

void AssimpLoader::readNodeHierarchy(const unsigned int animationID, const unsigned int frame, const float animationTime, const aiNode* node, const glm::mat4& parent, Animation::Frame* animationFrame) {

	std::string name = node->mName.C_Str();
	//glm::mat4 transform2 = mat4_castT(node->mTransformation);
	glm::mat4 nodeTransform =(mat4_cast(node->mTransformation));
	const aiNodeAnim* nodeAnim = m_channels[animationID][name];


	if (nodeAnim) {
		// Interpolate scaling and generate scaling transformation matrix
		aiVector3D Scaling;
		calcInterpolatedScale(Scaling, animationTime, nodeAnim);
		//glm::mat4 scalingM = glm::scale(glm::identity<glm::mat4>(), glm::vec3(1,1,1));
		glm::mat4 scalingM = (glm::scale(glm::identity<glm::mat4>(), glm::vec3(Scaling.x, Scaling.y, Scaling.z)));

		// Interpolate rotation and generate rotation transformation matrix
		aiQuaternion RotationQ;
		calcInterpolatedRotation(RotationQ, animationTime, nodeAnim);
		//aiMatrix4x4 rotationMa = aiMatrix4x4(RotationQ.GetMatrix());

		glm::quat rotation = quat_cast(RotationQ);
		glm::mat4 rotationM = (glm::toMat4(rotation));
		
		// Interpolate translation and generate translation transformation matrix
		aiVector3D Translation;
		calcInterpolatedPosition(Translation, animationTime, nodeAnim);
		glm::mat4 translationM = (glm::translate(glm::identity<glm::mat4>(), glm::vec3(Translation.x, Translation.y, Translation.z)));
		//glm::mat4 translationM = glm::translate(glm::identity<glm::mat4>(), glm::vec3(0,0,0));

		//nodeTransform = scalingM * rotationM * translationM;
		nodeTransform = translationM * rotationM * scalingM;
		// Combine the above transformations
		//NodeTransformation = TranslationM * RotationM * ScalingM;

		
	}


	glm::mat4 global = (parent * nodeTransform);
	//glm::mat4 global = nodeTransform * parent;

	if (m_boneMap.find(name) != m_boneMap.end()) {
		unsigned int index = m_boneMap[name].index;
		animationFrame->setTransform(index, (m_globalTransform * global * m_boneMap[name].offset));
		//animationFrame->setTransform(index, m_boneMap[name].offset * global * m_globalTransform);
	}

	for (unsigned int childID = 0; childID < node->mNumChildren; childID++) {
		readNodeHierarchy(animationID, frame, animationTime, node->mChildren[childID], global, animationFrame);

	}
}

const unsigned int AssimpLoader::getPositionFrame(const float animationTime, const aiNodeAnim* node) {
	for (unsigned int i = 0; i < node->mNumPositionKeys - 1; i++) {
		if (animationTime < (float)node->mPositionKeys[i + 1].mTime) {
			return i;
		}
	}
	assert(0);
	return 0;
}
const unsigned int AssimpLoader::getRotationFrame(const float animationTime, const aiNodeAnim* node) {
	assert(node->mNumRotationKeys > 0);

	for (unsigned int i = 0; i < node->mNumRotationKeys - 1; i++) {
		if (animationTime < (float)node->mRotationKeys[i + 1].mTime) {
			return i;
		}
	}
	assert(0);
	return 0;
}
const unsigned int AssimpLoader::getScaleFrame(const float animationTime, const aiNodeAnim* node) {
	assert(node->mNumScalingKeys > 0);

	for (unsigned int i = 0; i < node->mNumScalingKeys - 1; i++) {
		if (animationTime < (float)node->mScalingKeys[i + 1].mTime) {
			return i;
		}
	}
	assert(0);
	return 0;
}

void AssimpLoader::calcInterpolatedPosition(aiVector3D& out, const float animationTime, const aiNodeAnim* node) {
	if (node->mNumPositionKeys == 1) {
		out = node->mPositionKeys[0].mValue;
		return;
	}

	unsigned int PositionIndex = getPositionFrame(animationTime, node);
	unsigned int NextPositionIndex = (PositionIndex + 1);
	assert(NextPositionIndex < node->mNumPositionKeys);
	float DeltaTime = (float)(node->mPositionKeys[NextPositionIndex].mTime - node->mPositionKeys[PositionIndex].mTime);
	float Factor = (animationTime - (float)node->mPositionKeys[PositionIndex].mTime) / DeltaTime;
	assert(Factor >= 0.0f && Factor <= 1.0f);
	const aiVector3D& Start = node->mPositionKeys[PositionIndex].mValue;
	const aiVector3D& End = node->mPositionKeys[NextPositionIndex].mValue;
	aiVector3D Delta = End - Start;
	out = Start + Factor * Delta;

}
void AssimpLoader::calcInterpolatedRotation(aiQuaternion& out, const float animationTime, const aiNodeAnim* node) {
	// we need at least two values to interpolate...
	if (node->mNumRotationKeys == 1) {
		out = node->mRotationKeys[0].mValue;
		return;
	}

	unsigned int RotationIndex = getRotationFrame(animationTime, node);
	unsigned int NextRotationIndex = (RotationIndex + 1);
	assert(NextRotationIndex < node->mNumRotationKeys);
	float DeltaTime = (float)(node->mRotationKeys[NextRotationIndex].mTime - node->mRotationKeys[RotationIndex].mTime);
	float Factor = (animationTime - (float)node->mRotationKeys[RotationIndex].mTime) / DeltaTime;
	assert(Factor >= 0.0f && Factor <= 1.0f);
	const aiQuaternion& StartRotationQ = node->mRotationKeys[RotationIndex].mValue;
	const aiQuaternion& EndRotationQ = node->mRotationKeys[NextRotationIndex].mValue;
	aiQuaternion::Interpolate(out, StartRotationQ, EndRotationQ, Factor);
	out = out.Normalize();

}
void AssimpLoader::calcInterpolatedScale(aiVector3D& out, const float animationTime, const aiNodeAnim* node) {
	if (node->mNumScalingKeys == 1) {
		out = node->mScalingKeys[0].mValue;
		return;
	}

	unsigned int ScalingIndex = getScaleFrame(animationTime, node);
	unsigned int NextScalingIndex = (ScalingIndex + 1);
	assert(NextScalingIndex < node->mNumScalingKeys);
	float DeltaTime = (float)(node->mScalingKeys[NextScalingIndex].mTime - node->mScalingKeys[ScalingIndex].mTime);
	float Factor = (animationTime - (float)node->mScalingKeys[ScalingIndex].mTime) / DeltaTime;
	assert(Factor >= 0.0f && Factor <= 1.0f);
	const aiVector3D& Start = node->mScalingKeys[ScalingIndex].mValue;
	const aiVector3D& End = node->mScalingKeys[NextScalingIndex].mValue;
	aiVector3D Delta = End - Start;
	out = Start + Factor * Delta;

}

const bool AssimpLoader::errorCheck(const aiScene* scene) {
	if ( !scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode ) {
		SAIL_LOG_ERROR("ERROR::ASSIMP::" + std::string(m_importer.GetErrorString()));
		return true;
	}
	return false;
}

void AssimpLoader::clearData() {
	m_meshOffsets.clear();
	m_boneMap.clear();
	m_nodes.clear();
	m_channels.clear();
}

