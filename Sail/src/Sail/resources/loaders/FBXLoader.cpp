#include "pch.h"
#include "FBXLoader.h"

#include <d3d11.h>
#include "../../utils/Utils.h"
#include "../../graphics/geometry/factory/CubeModel.h"
#include "Sail/Application.h"
#include "Sail/graphics/geometry/Animation.h"


FbxManager* FBXLoader::s_manager = FbxManager::Create();
FbxIOSettings* FBXLoader::s_ios = FbxIOSettings::Create(s_manager, IOSROOT);


//transform ripped from https ://github.com/Caspila/GUInity/blob/master/Source/Converter.cpp
static const glm::mat4 FBXtoGLM(const FbxAMatrix& mat) {
	glm::mat4 newMat;
	newMat[0][0] = mat[0][0];
	newMat[0][1] = mat[0][1];
	newMat[0][2] = mat[0][2];
	newMat[0][3] = mat[0][3];

	newMat[1][0] = mat[1][0];
	newMat[1][1] = mat[1][1];
	newMat[1][2] = mat[1][2];
	newMat[1][3] = mat[1][3];

	newMat[2][0] = mat[2][0];
	newMat[2][1] = mat[2][1];
	newMat[2][2] = mat[2][2];
	newMat[2][3] = mat[2][3];

	newMat[3][0] = mat[3][0];
	newMat[3][1] = mat[3][1];
	newMat[3][2] = mat[3][2];
	newMat[3][3] = mat[3][3];

	//return glm::transpose(newMat);
	return newMat;
}
static void FBXtoGLM(glm::mat4& newMat, const FbxAMatrix& mat) {
	newMat[0][0] = mat[0][0];
	newMat[0][1] = mat[0][1];
	newMat[0][2] = mat[0][2];
	newMat[0][3] = mat[0][3];

	newMat[1][0] = mat[1][0];
	newMat[1][1] = mat[1][1];
	newMat[1][2] = mat[1][2];
	newMat[1][3] = mat[1][3];

	newMat[2][0] = mat[2][0];
	newMat[2][1] = mat[2][1];
	newMat[2][2] = mat[2][2];
	newMat[2][3] = mat[2][3];

	newMat[3][0] = mat[3][0];
	newMat[3][1] = mat[3][1];
	newMat[3][2] = mat[3][2];
	newMat[3][3] = mat[3][3];
	//newMat = glm::transpose(newMat);
}
static void FBXtoGLM(glm::vec3& newVec, const FbxVector4& vec) {
	newVec.x = (float)vec[0];
	newVec.y = (float)vec[1];
	newVec.z = (float)vec[2];
}
static void FBXtoGLM(glm::vec2& newVec, const FbxVector2& vec) {
	newVec.x = (float)vec[0];
	newVec.y = (float)vec[1];
}

FBXLoader::FBXLoader(const std::string& filepath, Shader* shader)
	: m_filepath(filepath)
	, m_shader(shader)
{
	s_manager->SetIOSettings(s_ios);

	m_model = std::make_unique<Model>();
	m_scene = parseFBX(filepath);
	
	// Triangulate all meshes in the scene
	FbxGeometryConverter geoConverter(s_manager);
	geoConverter.Triangulate(m_scene, true);

	if (m_scene != nullptr) {

		loadNode(m_scene->GetRootNode());
		/*m_model->getMaterial()->setKa(m_matSettings.ka);
		m_model->getMaterial()->setKd(m_matSettings.kd);
		m_model->getMaterial()->setKs(m_matSettings.ks);
		m_model->getMaterial()->setShininess(m_matSettings.shininess);
		if (m_matDiffuseTex != "")
			m_model->getMaterial()->setDiffuseTexture(m_matDiffuseTex);
		if (m_matNormalTex != "")
			m_model->getMaterial()->setDiffuseTexture(m_matNormalTex);
		if (m_matSpecularTex != "")
			m_model->getMaterial()->setDiffuseTexture(m_matSpecularTex);*/

	} else {
		Logger::Warning("Failed to load fbx file '" + filepath + "', using default cube.");
		glm::vec3 halfSizes = glm::vec3(0.5, 0.5, 0.5);
		m_model = ModelFactory::CubeModel::Create(halfSizes, shader);
	}
}
FBXLoader::FBXLoader() {
	s_manager->SetIOSettings(s_ios);
	
}
FBXLoader::~FBXLoader() {
	//if (m_scene)
	//	m_scene->Destroy();	5


	
}

bool FBXLoader::initScene(const std::string& filePath) {
	if (m_scenes.find(filePath) == m_scenes.end()) {
		FbxScene* scene = makeScene(filePath, filePath);

		if (!scene) {
			return false;
		}
		FbxGeometryConverter geoConverter(s_manager);
		if (!geoConverter.Triangulate(scene, true)) {
			assert(scene);
		}
		
		m_scenes[filePath] = scene;
		m_sceneData[filePath] = { false, false, false, false, nullptr, nullptr}; //??
	}
	return true;
}
bool FBXLoader::importScene(const std::string& filePath, Shader* shader) {
	if (m_sceneData.find(filePath) != m_sceneData.end())
		return true;
	if (!initScene(filePath)) {
		return false;
	}


	if (m_scenes.find(filePath) == m_scenes.end())
		return false;
	const FbxScene* scene = m_scenes[filePath];
	assert(scene);


	Model* model = importStaticModel(filePath, shader);
	if (model) {
		m_sceneData[filePath].hasModel = true;
		m_sceneData[filePath].model = model;
		//here would getMaterial for model be
		
	}

	AnimationStack* stack = importAnimationStack(filePath);
	if (stack) {
		m_sceneData[filePath].hasAnimation = true;
		m_sceneData[filePath].stack = stack;
	}

	return true;
}
void FBXLoader::clearScene(const std::string& filePath) {
	m_scenes[filePath];
	m_scenes.erase(filePath);
}
void FBXLoader::clearAllScenes() {
	//todo?	
}



Model* FBXLoader::importStaticModel(const std::string& filePath, Shader* shader) {
	const FbxScene* scene = m_scenes[filePath];
	assert(scene);

	FbxNode* root = scene->GetRootNode();
	#ifdef _DEBUG
		printNodeTree(root, "");
	#endif
	Logger::Log("name: " + std::string(root->GetScene()->GetName()));
	// LOAD MODEL
	Mesh::Data data;
	fetchGeometry(root, data, filePath);
	Model* model = SAIL_NEW Model(data, shader);


	return model;
}
AnimationStack* FBXLoader::importAnimationStack(const std::string& filePath) {

	const FbxScene* scene = m_scenes[filePath];
	assert(scene);

	FbxNode* root = scene->GetRootNode();

	
	AnimationStack* stack = SAIL_NEW AnimationStack();
	fetchSkeleton(root, filePath);
	fetchAnimations(root, stack, filePath);



	return stack;


}

Model* FBXLoader::fetchModel(const std::string& filePath, Shader* shader) {
	if (!importScene(filePath, shader))
		return nullptr;
	if (m_sceneData.find(filePath) == m_sceneData.end())
		return nullptr;
	auto* temp = m_sceneData[filePath].model;
	m_sceneData[filePath].model = nullptr;
	m_sceneData[filePath].hasModel = false;
	return temp;
}
AnimationStack* FBXLoader::fetchAnimationStack(const std::string& filePath, Shader* shader) {
	if (!importScene(filePath, shader))
		return nullptr;
	if (m_sceneData.find(filePath) == m_sceneData.end())
		return nullptr;
	auto* temp = m_sceneData[filePath].stack;
	m_sceneData[filePath].stack = nullptr;
	m_sceneData[filePath].hasAnimation = false;
	return temp;
}

FbxScene* FBXLoader::makeScene(std::string filePath, std::string sceneName) {
	FbxImporter* importer = FbxImporter::Create(s_manager, "");
	if (!importer->Initialize(filePath.c_str(), -1, s_manager->GetIOSettings())) {
		importer->Destroy();
#if _DEBUG
		Logger::Error("Could not load ("+ filePath +")");
		std::cout << "Could not Load '" + filePath + "'" << std::endl;
		assert(0);
#endif
		return nullptr;
	}
	FbxScene* lScene = FbxScene::Create(s_manager, sceneName.c_str());

	
	importer->Import(lScene);
	importer->Destroy();
		
	return lScene;
}



std::unique_ptr<Model>& FBXLoader::getModel() {
	return m_model;
}

void FBXLoader::fetchGeometry(FbxNode* node, Mesh::Data& meshData, const std::string& name) {

	FbxScene* scene = node->GetScene();
	unsigned int numAttributes = (unsigned int)node->GetNodeAttributeCount();
	for (unsigned int attributeIndex = 0; attributeIndex < numAttributes; attributeIndex++) {
	
		FbxNodeAttribute* nodeAttribute = node->GetNodeAttributeByIndex(attributeIndex);
		FbxNodeAttribute::EType attributeType = nodeAttribute->GetAttributeType();
	
		if (attributeType == FbxNodeAttribute::eMesh) {
			FbxMesh* fbxmesh = node->GetMesh();
			getGeometry(fbxmesh, meshData, name);
			return;
		}
	}
	
	unsigned int childCount = (unsigned int)node->GetChildCount();
	for (unsigned int child = 0; child < childCount; child++) {
		fetchGeometry(node->GetChild(child), meshData, name);
	}

}

void FBXLoader::fetchAnimations(FbxNode* node, AnimationStack* stack, const std::string& name) {
	FbxScene* scene = node->GetScene();
	int numAttributes = node->GetNodeAttributeCount();
	for (unsigned int attributeIndex = 0; attributeIndex < numAttributes; attributeIndex++) {

		FbxNodeAttribute* nodeAttribute = node->GetNodeAttributeByIndex(attributeIndex);
		FbxNodeAttribute::EType attributeType = nodeAttribute->GetAttributeType();

		if (attributeType == FbxNodeAttribute::eMesh) {
			FbxMesh* fbxmesh = node->GetMesh();
			
			getAnimations(node, stack, name);
			return;
		}
	}

	unsigned int childCount = (unsigned int)node->GetChildCount();
	for (unsigned int child = 0; child < childCount; child++) {
		fetchAnimations(node->GetChild(child), stack, name);
	}
}


FbxScene* FBXLoader::parseFBX(const std::string& filename) {
	FbxImporter* importer = FbxImporter::Create(s_manager, "");

	if (!importer->Initialize(filename.c_str(), -1, s_manager->GetIOSettings()))
		return nullptr;

	FbxScene* scene = FbxScene::Create(s_manager, "scene");
	importer->Import(scene);

	importer->Destroy();

	return scene;
}

void FBXLoader::loadNode(FbxNode* pNode) {
	// The number of attributes for the current node (mesh for example)
	int numAttributes = pNode->GetNodeAttributeCount();

	for (int j = 0; j < numAttributes; j++) {
		FbxNodeAttribute *nodeAttributeFbx = pNode->GetNodeAttributeByIndex(j);
		FbxNodeAttribute::EType attributeType = nodeAttributeFbx->GetAttributeType();

		FbxMesh* mesh;
		if (attributeType == FbxNodeAttribute::eMesh) {
			// Gets the mesh (only one mesh per model is supported)
			mesh = (FbxMesh*)nodeAttributeFbx;

			Mesh::Data meshData;
			getGeometry(mesh, meshData, "");
			std::unique_ptr<Mesh> mesh = std::unique_ptr<Mesh>(Mesh::Create(meshData, m_shader));
			getMaterial(pNode, mesh->getMaterial());
			m_model->addMesh(std::move(mesh));

		}

		if (attributeType == FbxNodeAttribute::eSkeleton) {
			// To load skeleton data for future versions of the engine
		}
	}

	// Recursively load the child nodes
	int numChildren = pNode->GetChildCount();
	for (int i = 0; i < numChildren; i++)
		this->loadNode(pNode->GetChild(i));

}


FbxVector2 FBXLoader::getTexCoord(int cpIndex, FbxGeometryElementUV* geUV, FbxMesh* mesh, int polyIndex, int vertIndex) const {
	FbxVector2 texCoord;

	switch (geUV->GetMappingMode()) {

		// UV per vertex
		case FbxGeometryElement::eByControlPoint:
			switch (geUV->GetReferenceMode()) {
				// Mapped directly per vertex
				case FbxGeometryElement::eDirect:
					texCoord = geUV->GetDirectArray().GetAt(cpIndex);
					break;
				// Mapped by indexed vertex
				case FbxGeometryElement::eIndexToDirect:
					texCoord = geUV->GetDirectArray().GetAt(geUV->GetIndexArray().GetAt(cpIndex));
					break;
				default:
					break;
			}
			break;

		// UV per indexed Vertex
		case FbxGeometryElement::eByPolygonVertex:
			switch (geUV->GetReferenceMode()) {
				// Mapped by indexed vertex
				case FbxGeometryElement::eIndexToDirect:
					texCoord = geUV->GetDirectArray().GetAt(mesh->GetTextureUVIndex(polyIndex, vertIndex));
					break;
				default:
					break;
			}
			break;
	}

	return texCoord;
}

void FBXLoader::getGeometry(FbxMesh* mesh, Mesh::Data& buildData, const std::string& name) {

	// Number of polygon vertices 
	buildData.numVertices = mesh->GetPolygonVertexCount();
	int* indices = mesh->GetPolygonVertices();

	if (int(buildData.numVertices / 3) != mesh->GetPolygonCount()) {
		Logger::Error("The mesh in '" + m_filepath + "' has to be triangulated.");
		return;
	}

	unsigned int uniqueVertices = 0;
	buildData.numIndices = buildData.numVertices;
	buildData.indices = SAIL_NEW unsigned long[buildData.numVertices];
	buildData.positions = SAIL_NEW Mesh::vec3[buildData.numVertices];
	buildData.normals = SAIL_NEW Mesh::vec3[buildData.numVertices];
	buildData.texCoords = SAIL_NEW Mesh::vec2[buildData.numVertices];
	buildData.tangents = SAIL_NEW Mesh::vec3[buildData.numVertices];
	buildData.bitangents = SAIL_NEW Mesh::vec3[buildData.numVertices];

	bool norms = true, uvs = true, tangs = true, bitangs = true;

	int vertexIndex = 0;
	FbxVector4* cp = mesh->GetControlPoints();
	auto& cpMap = m_sceneData[name].cpToVertMap;
	cpMap.resize(mesh->GetControlPointsCount());
	if (cp == nullptr) {
		Logger::Error("Couldn't find any vertices in the mesh in the file " + m_filepath);
		return;
	}

	Mesh::vec3 vertPosition[3] = { { 0,0,0 },{ 0,0,0 },{ 0,0,0 } };
	Mesh::vec3 vertNormal[3] = { { 0,0,0 },{ 0,0,0 },{ 0,0,0 } };
	Mesh::vec3 vertTangent[3] = { { 0,0,0 },{ 0,0,0 },{ 0,0,0 } };
	Mesh::vec3 vertBitangent[3] = { { 0,0,0 },{ 0,0,0 },{ 0,0,0 } };
	Mesh::vec2 vertTexCoord[3] = { { 0,0 },{ 0,0 } };
	



	for (unsigned int polyIndex = 0; polyIndex < mesh->GetPolygonCount(); polyIndex++) {
		int numVertices = mesh->GetPolygonSize(polyIndex);
		unsigned int CPIndex[3] = { indices[vertexIndex], indices[vertexIndex + 1], indices[vertexIndex + 2] };
		for (unsigned int vertIndex = 0; vertIndex < numVertices; vertIndex += 3) {
			
#pragma region POSITIONS
			/*
			--	Positions
			*/

			FBXtoGLM(vertPosition[0].vec, cp[indices[vertexIndex]]);
			FBXtoGLM(vertPosition[1].vec, cp[indices[vertexIndex + 1]]);
			FBXtoGLM(vertPosition[2].vec, cp[indices[vertexIndex + 2]]);
#pragma endregion
#pragma region NORMALS
			/*
			--	Normals
			*/
			FbxLayerElementNormal* leNormal = mesh->GetLayer(0)->GetNormals();
			if (leNormal == nullptr && norms) {
				Logger::Warning("Couldn't find any normals in the mesh in the file " + m_filepath);
				norms = false;
			} 
			else if (norms) {
				if (leNormal->GetMappingMode() == FbxLayerElement::eByPolygonVertex) {
					int normIndex = 0;
					if (leNormal->GetReferenceMode() == FbxLayerElement::eDirect)
						normIndex = vertexIndex;
					if (leNormal->GetReferenceMode() == FbxLayerElement::eIndexToDirect)
						normIndex = leNormal->GetIndexArray().GetAt(vertexIndex);
					
					FbxVector4 norm = leNormal->GetDirectArray().GetAt(normIndex);
					FBXtoGLM(vertNormal[0].vec, norm);
					//for (unsigned int axis = 0; axis < 3; axis++) {
					//	vertNormal[0].vec[axis] = (float)norm[axis];
					//}
					//buildData.normals[vertexIndex].vec.x = (float)norm[0];
					//buildData.normals[vertexIndex].vec.y = (float)norm[1];
					//buildData.normals[vertexIndex].vec.z = (float)norm[2];

					norm = leNormal->GetDirectArray().GetAt(normIndex + 1);
					FBXtoGLM(vertNormal[1].vec, norm);
					//for (unsigned int axis = 0; axis < 3; axis++) {
					//	vertNormal[1].vec[axis] = (float)norm[axis];
					//}
					//buildData.normals[vertexIndex + 1].vec.x = (float)norm[0];
					//buildData.normals[vertexIndex + 1].vec.y = (float)norm[1];
					//buildData.normals[vertexIndex + 1].vec.z = (float)norm[2];

					norm = leNormal->GetDirectArray().GetAt(normIndex + 2);
					FBXtoGLM(vertNormal[2].vec, norm);
					//for (unsigned int axis = 0; axis < 3; axis++) {
					//	vertNormal[2].vec[axis] = (float)norm[axis];
					//}
					//buildData.normals[vertexIndex + 2].vec.x = (float)norm[0];
					//buildData.normals[vertexIndex + 2].vec.y = (float)norm[1];
					//buildData.normals[vertexIndex + 2].vec.z = (float)norm[2];
				}
			}
			else {
				vertNormal[0] = { 0,0,0 };
				vertNormal[1] = { 0,0,0 };
				vertNormal[2] = { 0,0,0 };
			}
#pragma endregion
#pragma region TANGENTS
			/*
			--	Tangents
			*/
			FbxGeometryElementTangent *geTang = mesh->GetElementTangent(0);
			if (geTang == nullptr && tangs) {
				Logger::Warning("Couldn't find any tangents in the mesh in the file " + m_filepath);
				tangs = false;
			} 
			else if (tangs) {
				if (geTang->GetMappingMode() == FbxLayerElement::eByPolygonVertex) {
					int tangIndex = 0;
					if (geTang->GetReferenceMode() == FbxLayerElement::eDirect)
						tangIndex = vertexIndex;
					if (geTang->GetReferenceMode() == FbxLayerElement::eIndexToDirect)
						tangIndex = geTang->GetIndexArray().GetAt(vertexIndex);

					FbxVector4 tangent = geTang->GetDirectArray().GetAt(tangIndex);
					FBXtoGLM(vertTangent[0].vec, tangent);

					tangent = geTang->GetDirectArray().GetAt(tangIndex + 1);
					FBXtoGLM(vertTangent[1].vec, tangent);

					tangent = geTang->GetDirectArray().GetAt(tangIndex + 2);
					FBXtoGLM(vertTangent[2].vec, tangent);
				}
			}
			else {
				vertTangent[0] = { 0,0,0 };
				vertTangent[1] = { 0,0,0 };
				vertTangent[2] = { 0,0,0 };
			}
#pragma endregion
#pragma region BINORMALS
			/*
			--	Binormals
			*/
			FbxGeometryElementBinormal *geBN = mesh->GetElementBinormal(0);
			if (geBN == nullptr && bitangs) {
				Logger::Warning("Couldn't find any binormals in the mesh in the file " + m_filepath);
				bitangs = false;
			} 
			else if (bitangs) {
				if (geBN->GetMappingMode() == FbxLayerElement::eByPolygonVertex) {
					int biNormIndex = 0;

					if (geBN->GetReferenceMode() == FbxLayerElement::eDirect)
						biNormIndex = vertexIndex;


					if (geBN->GetReferenceMode() == FbxLayerElement::eIndexToDirect)
						biNormIndex = geBN->GetIndexArray().GetAt(vertexIndex);

					FbxVector4 biNorm = geBN->GetDirectArray().GetAt(biNormIndex);
					for (unsigned int axis = 0; axis < 3; axis++) {
						vertBitangent[2].vec[axis] = (float)biNorm[axis];
					}

					biNorm = geBN->GetDirectArray().GetAt(biNormIndex + 1);
					for (unsigned int axis = 0; axis < 3; axis++) {
						vertBitangent[2].vec[axis] = (float)biNorm[axis];
					}

					biNorm = geBN->GetDirectArray().GetAt(biNormIndex + 2);
					for (unsigned int axis = 0; axis < 3; axis++) {
						vertBitangent[2].vec[axis] = (float)biNorm[axis];
					}
				}
			}
			else {
				vertBitangent[0] = { 0,0,0 };
				vertBitangent[1] = { 0,0,0 };
				vertBitangent[2] = { 0,0,0 };
			}
#pragma endregion
#pragma region TEXCOORDS
			/*
			--	UV Coords
			*/
			FbxGeometryElementUV* geUV = mesh->GetElementUV(0);
			if (geUV == nullptr && uvs) {
				Logger::Warning("Couldn't find any texture coordinates in the mesh in the file " + m_filepath);
				uvs = false;
			} 
			else if (uvs) {
				FbxVector2 texCoord;
				int cpIndex;

				cpIndex = mesh->GetPolygonVertex(polyIndex, vertIndex);
				texCoord = getTexCoord(cpIndex, geUV, mesh, polyIndex, vertIndex);
				vertTexCoord[0].vec[0] = static_cast<float>(texCoord[0]);
				vertTexCoord[0].vec[1] = -static_cast<float>(texCoord[1]);

				cpIndex = mesh->GetPolygonVertex(polyIndex, vertIndex + 1);
				texCoord = getTexCoord(cpIndex, geUV, mesh, polyIndex, vertIndex + 1);
				vertTexCoord[1].vec[0] = static_cast<float>(texCoord[0]);
				vertTexCoord[1].vec[1] = -static_cast<float>(texCoord[1]);

				cpIndex = mesh->GetPolygonVertex(polyIndex, vertIndex + 2);
				texCoord = getTexCoord(cpIndex, geUV, mesh, polyIndex, vertIndex + 2);
				vertTexCoord[2].vec[0] = static_cast<float>(texCoord[0]);
				vertTexCoord[2].vec[1] = -static_cast<float>(texCoord[1]);
			} 
			else {
				vertTexCoord[0] = { 0,0 };
				vertTexCoord[1] = { 0,0 };
				vertTexCoord[2] = { 0,0 };
			}
#pragma endregion		

			unsigned long oldUnique = uniqueVertices;
			addVertex(buildData, uniqueVertices, vertexIndex++, vertPosition[0], vertNormal[0], vertTangent[0], vertBitangent[0], vertTexCoord[0]);
			
			if(oldUnique != uniqueVertices)
				cpMap[CPIndex[0]].emplace_back(oldUnique);
			oldUnique = uniqueVertices;
			addVertex(buildData, uniqueVertices, vertexIndex++, vertPosition[1], vertNormal[1], vertTangent[1], vertBitangent[1], vertTexCoord[1]);
			if (oldUnique != uniqueVertices)
				cpMap[CPIndex[1]].emplace_back(oldUnique);
			oldUnique = uniqueVertices; 
			addVertex(buildData, uniqueVertices, vertexIndex++, vertPosition[2], vertNormal[2], vertTangent[2], vertBitangent[2], vertTexCoord[2]);
			if (oldUnique != uniqueVertices)
				cpMap[CPIndex[2]].emplace_back(oldUnique);

		}

	}
	buildData.resizeVertices(uniqueVertices);

}

void FBXLoader::getAnimations(FbxNode* node, AnimationStack* stack, const std::string& name) {

	auto& cpMap = m_sceneData[name].cpToVertMap;
	assert(cpMap.size() > 0 && "CPMAP empty :(");
	const FbxScene* scene = node->GetScene();
	std::string nodeName = node->GetName();
	int numAttributes = node->GetNodeAttributeCount();
	for (unsigned int attributeIndex = 0; attributeIndex < numAttributes; attributeIndex++) {

		
		FbxNodeAttribute* nodeAttribute = node->GetNodeAttributeByIndex(attributeIndex);
		FbxNodeAttribute::EType attributeType = nodeAttribute->GetAttributeType();

		if (attributeType == FbxNodeAttribute::eMesh) {
			FbxMesh* mesh = node->GetMesh();
			unsigned int cpCount = mesh->GetControlPointsCount();
			stack->reSizeConnections(m_sceneData[name].model->getMesh(0)->getNumVertices());

			FbxAMatrix geometryTransform(node->GetGeometricTranslation(FbxNode::eSourcePivot), node->GetGeometricRotation(FbxNode::eSourcePivot), node->GetGeometricScaling(FbxNode::eSourcePivot));

			unsigned int deformerCount = mesh->GetDeformerCount();
			for (unsigned int deformerIndex = 0; deformerIndex < deformerCount; deformerIndex++) {
				FbxSkin* skin = reinterpret_cast<FbxSkin*>(mesh->GetDeformer(deformerIndex, FbxDeformer::eSkin));
				if (!skin) {
					#ifdef _DEBUG
						Logger::Error("not a skin at deformer " + std::to_string(deformerIndex) + " in " + nodeName);
					#endif
					continue;
				}
				unsigned int clusterCount = skin->GetClusterCount();

				std::vector<int> limbIndexes;
				limbIndexes.resize(clusterCount);
				for (unsigned int clusterIndex = 0; clusterIndex < clusterCount; clusterIndex++) {
					FbxCluster* cluster = skin->GetCluster(clusterIndex);
					limbIndexes[clusterIndex] = getBoneIndex(cluster->GetLink()->GetUniqueID(), name);
					if (limbIndexes[clusterIndex] == -1) {
						Logger::Warning("Could not find limb at clusterIndex: " + std::to_string(clusterIndex));
					}
				}



				

				// set controlpoints?
				for (unsigned int clusterIndex = 0; clusterIndex < clusterCount; clusterIndex++) {
					FbxCluster* cluster = skin->GetCluster(clusterIndex);
					glm::mat4 globalBindposeInverse;

					FbxAMatrix transformMatrix;
					FbxAMatrix transformLinkMatrix;
					FbxAMatrix globalBindposeInverseMatrix;
					cluster->GetTransformMatrix(transformMatrix);
					cluster->GetTransformLinkMatrix(transformLinkMatrix);
					globalBindposeInverseMatrix = transformLinkMatrix.Inverse() * transformMatrix * geometryTransform;

					FBXtoGLM(globalBindposeInverse, globalBindposeInverseMatrix);

					m_sceneData[name].bones[limbIndexes[clusterIndex]].globalBindposeInverse = globalBindposeInverse;



					// Update the information in mSkeleton 
					//model->setGlobalBindposeInverse(limbIndexes[clusterIndex], convertToXMMatrix(globalBindposeInverseMatrix));


					unsigned int indexCount = cluster->GetControlPointIndicesCount();
					int* CPIndices = cluster->GetControlPointIndices();
					double* CPWeights = cluster->GetControlPointWeights();

					for (unsigned int index = 0; index < indexCount; ++index) {

						int indexCCPI = CPIndices[index];
						int limbIndex = limbIndexes[clusterIndex];
						float weightzzz = CPWeights[index];
						for (int i = 0; i < cpMap[indexCCPI].size(); i++) {
							unsigned long trueIndex = cpMap[indexCCPI][i];
							stack->setConnectionData(trueIndex, limbIndex, weightzzz);

						}

					}
				}

				stack->normalizeWeights();
				stack->checkWeights();



				/*  ANIMATION FETCHING FROM STACK*/
				unsigned int stackCount = scene->GetSrcObjectCount<FbxAnimStack>();
				for (int currentStack = 0; currentStack < stackCount; currentStack++) {
					Animation* animation = SAIL_NEW Animation();
					FbxAnimStack* currAnimStack = scene->GetSrcObject<FbxAnimStack>(currentStack);
					FbxTakeInfo* takeInfo = scene->GetTakeInfo(currAnimStack->GetName());
					std::string animationName = currAnimStack->GetName();
					node->GetScene()->SetCurrentAnimationStack(currAnimStack);

					//FbxAnimEvaluator* eval = node->GetAnimationEvaluator();
					FbxTime start = takeInfo->mLocalTimeSpan.GetStart();
					FbxTime end = takeInfo->mLocalTimeSpan.GetStop();



					//cout << "Animation Time: " << to_string((float)takeInfo->mLocalTimeSpan.GetDuration().GetSecondDouble()) << " Frame Count: " << to_string((int)end.GetFrameCount(FbxTime::eFrames24)) << endl;
					float firstFrameTime = 0.0f;

					//TODO: find way to import FPS from file.
					FbxTime::EMode fps = FbxTime::eFrames24;
					for (FbxLongLong frameIndex = start.GetFrameCount(fps); frameIndex <= end.GetFrameCount(fps); frameIndex++) {
						Animation::Frame* frame = SAIL_NEW Animation::Frame(m_sceneData[name].bones.size());
						FbxTime currTime;
						currTime.SetFrame(frameIndex, fps);
						if (firstFrameTime == 0.0f)
							firstFrameTime = float(currTime.GetSecondDouble());
						glm::mat4 matrix;		
						
						for (unsigned int clusterIndex = 0; clusterIndex < clusterCount; ++clusterIndex) {
							FbxCluster* cluster = skin->GetCluster(clusterIndex);

							FbxAMatrix currentTransformOffset = node->EvaluateGlobalTransform(currTime) * geometryTransform;

							FBXtoGLM(matrix, currentTransformOffset.Inverse() * cluster->GetLink()->EvaluateGlobalTransform(currTime));

							frame->setTransform(limbIndexes[clusterIndex],  matrix * m_sceneData[name].bones[limbIndexes[clusterIndex]].globalBindposeInverse);
						}
						float time = float(currTime.GetSecondDouble()) - firstFrameTime;
						animation->addFrame(frameIndex, time, frame);
					}
					
					stack->addAnimation(animationName, animation);
					Logger::Log("Added: " + animationName);
				}
			}//deformer end
		}
		
	} // attribute end
}

void FBXLoader::getMaterial(FbxNode* pNode, Material* material) {

	// Gets the model's phong constants
	if (pNode->GetSrcObjectCount<FbxSurfacePhong>() > 0) {

		auto phong = pNode->GetSrcObject<FbxSurfacePhong>();
		auto& resman = Application::getInstance()->getResourceManager();

		FbxFileTexture* diffTex = phong->Diffuse.GetSrcObject<FbxFileTexture>();
		if (diffTex) {
			std::string filename = diffTex->GetRelativeFileName();
			resman.loadTexture(filename);
			material->setDiffuseTexture(filename);
		}
		FbxFileTexture* specTex = phong->Specular.GetSrcObject<FbxFileTexture>();
		if (specTex) {
			std::string filename = specTex->GetRelativeFileName();
			resman.loadTexture(filename);
			material->setSpecularTexture(specTex->GetRelativeFileName());
		}
		FbxFileTexture* normTex = phong->NormalMap.GetSrcObject<FbxFileTexture>();
		if (normTex) {
			std::string filename = normTex->GetRelativeFileName();
			resman.loadTexture(filename);
			material->setNormalTexture(normTex->GetRelativeFileName());
		}

		material->setKa(static_cast<float>(phong->AmbientFactor.Get()));
		material->setKs(static_cast<float>(phong->SpecularFactor.Get()));
		material->setKd (static_cast<float>(phong->DiffuseFactor.Get()));
		material->setShininess(static_cast<float>(phong->Shininess.Get()));

	}

}

void FBXLoader::addVertex(Mesh::Data& buildData, unsigned int& uniqueVertices, const unsigned long& currentIndex, const Mesh::vec3& position, const Mesh::vec3& normal, const Mesh::vec3& tangent, const Mesh::vec3& bitangent, const Mesh::vec2& uv) {
	for (unsigned int vert = 0; vert < uniqueVertices; vert++) {
		if (buildData.positions[vert] == position && buildData.normals[vert] == normal && buildData.texCoords[vert] == uv) {
			buildData.indices[currentIndex] = vert;
			return;
		}
	}

	buildData.positions[uniqueVertices] = position;
	buildData.normals[uniqueVertices] = normal;
	buildData.tangents[uniqueVertices] = tangent;
	buildData.bitangents[uniqueVertices] = bitangent;
	buildData.texCoords[uniqueVertices] = uv;
	buildData.indices[currentIndex] = uniqueVertices;
	uniqueVertices += 1;
}

void FBXLoader::fetchSkeleton(FbxNode* node, const std::string& filename) {


	for (int childIndex = 0; childIndex < node->GetChildCount(); ++childIndex) {
		FbxNode* currNode = node->GetChild(childIndex);
		fetchSkeletonRecursive(currNode, filename, 0, 0, -1);
	}

}
void FBXLoader::fetchSkeletonRecursive(FbxNode* inNode, const std::string& filename, int inDepth, int myIndex, int inParentIndex) {

	if (inNode->GetNodeAttribute() && inNode->GetNodeAttribute()->GetAttributeType() && inNode->GetNodeAttribute()->GetAttributeType() == FbxNodeAttribute::eSkeleton) {
		m_sceneData[filename].bones;
		FBXLoader::Bone limb;
		limb.parentIndex = inParentIndex;
		limb.uniqueID = inNode->GetUniqueID();

		if(m_sceneData[filename].bones.size() > 0)
			m_sceneData[filename].bones[limb.parentIndex].childIndexes.push_back(int(m_sceneData[filename].bones.size()));
		m_sceneData[filename].bones.push_back(limb);

	}
	for (int i = 0; i < inNode->GetChildCount(); i++) {
		fetchSkeletonRecursive(inNode->GetChild(i), filename, inDepth + 1, unsigned int(m_sceneData[filename].bones.size()), myIndex);
	}


}

int FBXLoader::getBoneIndex(unsigned int uniqueID, const std::string& name) {
	auto& ref = m_sceneData[name].bones;
	for (unsigned int i = 0; i < ref.size(); i++) {
		if (ref[i].uniqueID == uniqueID)
			return i;
	}
	return -1;
}



std::string FBXLoader::GetAttributeTypeName(FbxNodeAttribute::EType type) {
	switch (type) {
	case FbxNodeAttribute::eUnknown: return "unidentified";
	case FbxNodeAttribute::eNull: return "null";
	case FbxNodeAttribute::eMarker: return "marker";
	case FbxNodeAttribute::eSkeleton: return "skeleton";
	case FbxNodeAttribute::eMesh: return "mesh";
	case FbxNodeAttribute::eNurbs: return "nurbs";
	case FbxNodeAttribute::ePatch: return "patch";
	case FbxNodeAttribute::eCamera: return "camera";
	case FbxNodeAttribute::eCameraStereo: return "stereo";
	case FbxNodeAttribute::eCameraSwitcher: return "camera switcher";
	case FbxNodeAttribute::eLight: return "light";
	case FbxNodeAttribute::eOpticalReference: return "optical reference";
	case FbxNodeAttribute::eOpticalMarker: return "marker";
	case FbxNodeAttribute::eNurbsCurve: return "nurbs curve";
	case FbxNodeAttribute::eTrimNurbsSurface: return "trim nurbs surface";
	case FbxNodeAttribute::eBoundary: return "boundary";
	case FbxNodeAttribute::eNurbsSurface: return "nurbs surface";
	case FbxNodeAttribute::eShape: return "shape";
	case FbxNodeAttribute::eLODGroup: return "lodgroup";
	case FbxNodeAttribute::eSubDiv: return "subdiv";
	default: return "unknown";
	}
}

std::string FBXLoader::PrintAttribute(FbxNodeAttribute* pAttribute) {
	if (!pAttribute) return "x";

	std::string typeName = GetAttributeTypeName(pAttribute->GetAttributeType());
	std::string attrName = pAttribute->GetName();

	return typeName + " " + attrName;
}

void FBXLoader::printNodeTree(FbxNode * node, const std::string& indent) {
	std::string name = node->GetName();
	std::string attributes = "";

	for (int i = 0; i < node->GetNodeAttributeCount(); i++) {
		attributes += ": "+ PrintAttribute(node->GetNodeAttributeByIndex(i));
	}
	
	Logger::Log(indent + name + ":" + attributes);
	for (int i = 0; i < node->GetChildCount(); i++) {
		printNodeTree(node->GetChild(i), indent + " ");
	}
}

void FBXLoader::printAnimationStack(const FbxNode* node) {
}
