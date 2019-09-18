#include "pch.h"
#include "AssimpLoader.h"

AssimpLoader::AssimpLoader() :
m_importer()
{

}

AssimpLoader::~AssimpLoader() {
	//delete m_importer;
}

Model* AssimpLoader::importModel(const std::string& path, Shader* shader) {

	const aiScene* scene = m_importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);
	if (errorCheck(scene)) {
		return nullptr;
	}
	std::string name = scene->GetShortFilename(path.c_str());
	//processNode(scene, scene->mRootNode);

	return nullptr;
}

AnimationStack* AssimpLoader::importAnimationStack(const std::string& path) {
	const aiScene* scene = m_importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_JoinIdenticalVertices);
	if (errorCheck(scene)) {
		return nullptr;
	}
	std::string name = scene->GetShortFilename(path.c_str());
	//processNode(scene, scene->mRootNode);

	size_t vertCount = 0;

	//	__________________________________MATRIX______________________________
	m_globalTransform = glm::inverse(mat4_cast(scene->mRootNode->mTransformation));
	//	_______________________________________________________________________

	makeOffsets(scene);
	for (size_t i = 0; i < scene->mNumMeshes; i++) {
		vertCount += scene->mMeshes[i]->mNumVertices;
	}
	AnimationStack* stack = new AnimationStack(vertCount);
	scene->mMeshes[scene->mRootNode->mMeshes[0]]->mBones[0];
	
	if (!importBonesFromNode(scene, scene->mRootNode, stack)) {
		Memory::SafeDelete(stack);
		clearData();
		return nullptr;
	}
	stack->checkWeights();

	if (!importAnimations(scene, stack)) {
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




Mesh* AssimpLoader::importMesh(const aiScene* scene, aiNode* node) {
	return nullptr;
}

bool AssimpLoader::importBonesFromNode(const aiScene* scene, aiNode* node, AnimationStack* stack) {
	#ifdef _DEBUG 
	Logger::Log("1"+std::string(node->mName.C_Str())); 
	

	if (m_nodes.find(node->mName.C_Str()) == m_nodes.end()) {
		m_nodes[node->mName.C_Str()] = node;
	}

	#endif
	for (size_t nodeID = 0; nodeID < node->mNumMeshes; nodeID++) {
		const aiMesh* mesh = scene->mMeshes[node->mMeshes[nodeID]];
		
			#ifdef _DEBUG 
		Logger::Log("2 "+std::string(mesh->mName.C_Str()));
			#endif
		if (mesh->HasBones()) {
			for (size_t boneID = 0; boneID < mesh->mNumBones; boneID++) {
				const aiBone* bone = mesh->mBones[boneID];
				
				std::string boneName = bone->mName.C_Str();
				size_t index = m_boneMap.size();
				if (m_boneMap.find(boneName) == m_boneMap.end()) {
					m_boneMap[boneName] = { index, node->mName.C_Str(), mat4_cast(bone->mOffsetMatrix)};
					
				}
				else {
					index = m_boneMap[boneName].index;
				}
				


				#ifdef _DEBUG 
				Logger::Log("3  "+std::string(bone->mName.C_Str()));
				#endif



				for (size_t weightID = 0; weightID < bone->mNumWeights; weightID++) {
					const aiVertexWeight weight = bone->mWeights[weightID];
					stack->setConnectionData(m_meshOffsets[node->mMeshes[nodeID]]+weight.mVertexId, index, weight.mWeight);

				}


			}

		}
		else {
			Logger::Log("3  No Bones");
		}

	}




	int size = node->mNumChildren;
	for (size_t i = 0; i < size; i++) {
		importBonesFromNode(scene, node->mChildren[i], stack);
	}


	return true;
}

bool AssimpLoader::importAnimations(const aiScene* scene, AnimationStack* stack) {
	if (!scene->HasAnimations()) {
		return false;
	}

	//debug
#ifdef _DEBUG
	std::vector<const aiAnimation*> animationz;
	std::vector<std::vector<const aiNodeAnim*>> channels;
#endif
	mapChannels(scene);

	for (size_t animationIndex = 0; animationIndex < scene->mNumAnimations; animationIndex++) {
		const aiAnimation* animation = scene->mAnimations[animationIndex];
		Animation* anim = new Animation();
		
#ifdef _DEBUG
		animationz.emplace_back(animation);
		channels.emplace_back(std::vector<const aiNodeAnim*>());
		for (int i = 0; i < animation->mNumChannels; i++) {
			channels.back().emplace_back(animation->mChannels[i]);
		}
		std::string temp = std::to_string(animation->mDuration) + ":" + std::to_string(animation->mTicksPerSecond) + " = " + std::to_string(animation->mDuration / animation->mTicksPerSecond);
		Logger::Log(temp);
#endif

		size_t totalFrames = animation->mDuration;
		Logger::Log(std::to_string(totalFrames));
		float totalDivided = 1.0f / (float)totalFrames;
		float tickRate = 1.0f/(animation->mTicksPerSecond == 0 ? 24.0f : animation->mTicksPerSecond);
		float totalTime = totalFrames * (animation->mTicksPerSecond == 0 ? 24.0f : animation->mTicksPerSecond);
		for (size_t frame = 0; frame < totalFrames; frame++) {
			float time = (float)frame * tickRate;
			Animation::Frame* currentFrame = SAIL_NEW Animation::Frame(m_boneMap.size());
			//Logger::Log("Added Frame with ");
			
			readNodeHierarchy(animationIndex, frame, 0, scene->mRootNode, glm::identity<glm::mat4>(), currentFrame);
			anim->pushBackFrame(time, currentFrame);
		}

		stack->addAnimation(animation->mName.C_Str(),anim);
	}
	


	return true;
}

void AssimpLoader::readNodeHierarchy(const size_t animationID, const size_t frame, const float animationTime, const aiNode* node, const glm::mat4& parent, Animation::Frame* animationFrame) {

	std::string name = node->mName.C_Str();
	glm::mat4 nodeTransform = mat4_cast(node->mTransformation);
	const aiNodeAnim* nodeAnim = m_channels[animationID][name];

	if (nodeAnim) {
		/// DO STUFFFFFFF

	}

	glm::mat4 global = parent * nodeTransform;
	if (m_boneMap.find(name) != m_boneMap.end()) {
		unsigned int index = m_boneMap[name].index;
		animationFrame->setTransform(index, m_globalTransform * global * m_boneMap[name].offset);
	}

	for (int childID = 0; childID < node->mNumChildren; childID++) {
		readNodeHierarchy(animationID, frame, animationTime, node->mChildren[childID], global, animationFrame);

	}
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
	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
		Logger::Error("ERROR::ASSIMP::" + std::string(m_importer.GetErrorString()));
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

