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
	if (m_scene)
		m_scene->Destroy();	


	
}

bool FBXLoader::initScene(const std::string& filePath) {
	if (m_scenes.find(filePath) == m_scenes.end()) {
		FbxScene* scene = makeScene(filePath, filePath);
		if (!scene) {
			return false;
		}
		m_scenes[filePath] = scene;
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



Model* FBXLoader::importStaticModel(const std::string& filePath, Shader* path) {
	const FbxScene* scene = m_scenes[filePath];
	if (!scene) {
		initScene(filePath);
		scene = m_scenes[filePath];
		if (!scene)
			assert(0);
	}

	Model* model = new Model();
	FbxNode* root = scene->GetRootNode();





	return nullptr;
}

AnimationStack* FBXLoader::importStack(const std::string& filePath) {




	return nullptr;
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

void FBXLoader::fetchGeometry(const FbxNode* node, Mesh* mesh) {

//	FbxScene* scene = node->GetScene();
//	int numAttributes = node->GetNodeAttributeCount();
//	for (int j = 0; j < numAttributes; j++) {
//
//		FbxNodeAttribute* nodeAttribute = node->GetNodeAttributeByIndex(j);
//		FbxNodeAttribute::EType attributeType = nodeAttribute->GetAttributeType();
//
//		if (attributeType == FbxNodeAttribute::eMesh) {
//			FbxMesh* mesh = node->GetMesh();
//			FbxAMatrix geometryTransform(node->GetGeometricTranslation(FbxNode::eSourcePivot), node->GetGeometricRotation(FbxNode::eSourcePivot), node->GetGeometricScaling(FbxNode::eSourcePivot));
//
//			unsigned int cpCount = mesh->GetControlPointsCount();
//			FbxVector4* cps = mesh->GetControlPoints();
//			model->reSizeControlPoints(cpCount);
//
//			int polygonVertexCount = mesh->GetPolygonVertexCount();
//			int* indices = mesh->GetPolygonVertices();
//			if (int(polygonVertexCount / 3) != mesh->GetPolygonCount()) {
//				cout << "The mesh in '" << filename << "' has to be triangulated.";
//				return;
//			}
//
//			int vertexIndex = 0;
//			if (cps == nullptr) {
//				cout << "Couldn't find any vertices in the mesh in the file " << filename << endl;
//				return;
//			}
//			for (int polyIndex = 0; polyIndex < mesh->GetPolygonCount(); polyIndex++) {
//				int numVertices = mesh->GetPolygonSize(polyIndex);
//
//				for (int vertIndex = 0; vertIndex < numVertices; vertIndex += 3) {
//
//					/*NORMALS*/
//					FbxLayerElementNormal* leNormal = mesh->GetLayer(0)->GetNormals();
//					FbxVector4 norm[3] = { {0, 0, 0},{0, 0, 0},{0, 0, 0} };
//					FbxVector2 texCoord[3] = { {0,0}, {0,0} };
//					if (leNormal == nullptr) {
//						cout << "Couldn't find any normals in the mesh in the file " << filename << endl;
//					}
//					else if (leNormal) {
//						if (leNormal->GetMappingMode() == FbxLayerElement::eByPolygonVertex) {
//							int normIndex = 0;
//							if (leNormal->GetReferenceMode() == FbxLayerElement::eDirect)
//								normIndex = vertexIndex;
//							if (leNormal->GetReferenceMode() == FbxLayerElement::eIndexToDirect)
//								normIndex = leNormal->GetIndexArray().GetAt(vertexIndex);
//							norm[0] = leNormal->GetDirectArray().GetAt(normIndex);
//							norm[1] = leNormal->GetDirectArray().GetAt(normIndex + 1);
//							norm[2] = leNormal->GetDirectArray().GetAt(normIndex + 2);
//						}
//					}
//
//					/*UV COORDS*/
//					FbxGeometryElementUV* geUV = mesh->GetElementUV(0);
//					if (geUV == nullptr) {
//						cout << "Couldn't find any texture coordinates in the mesh in the file " << filename << endl;
//					}
//					else if (geUV) {
//						int cpIndex;
//						cpIndex = mesh->GetPolygonVertex(polyIndex, vertIndex);
//						texCoord[0] = getTexCoord(cpIndex, geUV, mesh, polyIndex, vertIndex);
//						cpIndex = mesh->GetPolygonVertex(polyIndex, vertIndex + 2);
//						texCoord[1] = getTexCoord(cpIndex, geUV, mesh, polyIndex, vertIndex + 2);
//						cpIndex = mesh->GetPolygonVertex(polyIndex, vertIndex + 1);
//						texCoord[2] = getTexCoord(cpIndex, geUV, mesh, polyIndex, vertIndex + 1);
//					}
//
//					model->addVertex({
//						DirectX::XMFLOAT3((float)cps[indices[vertexIndex]][0], (float)cps[indices[vertexIndex]][1],(float)cps[indices[vertexIndex]][2]),
//						DirectX::XMFLOAT3((float)norm[0][0], (float)norm[0][1], (float)norm[0][2]),
//						DirectX::XMFLOAT2(static_cast<float>(texCoord[0][0]),-static_cast<float>(texCoord[0][1]))
//						}, polyIndex * 3 + vertIndex);
//					model->addVertex({
//						DirectX::XMFLOAT3((float)cps[indices[vertexIndex + 1]][0], (float)cps[indices[vertexIndex + 1]][1],(float)cps[indices[vertexIndex + 1]][2]),
//						DirectX::XMFLOAT3((float)norm[1][0], (float)norm[1][1], (float)norm[1][2]),
//						DirectX::XMFLOAT2(static_cast<float>(texCoord[2][0]),-static_cast<float>(texCoord[2][1]))
//						}, polyIndex * 3 + vertIndex + 2);
//					model->addVertex({
//						DirectX::XMFLOAT3((float)cps[indices[vertexIndex + 2]][0], (float)cps[indices[vertexIndex + 2]][1],(float)cps[indices[vertexIndex + 2]][2]),
//						DirectX::XMFLOAT3((float)norm[2][0], (float)norm[2][1], (float)norm[2][2]),
//						DirectX::XMFLOAT2(static_cast<float>(texCoord[1][0]),-static_cast<float>(texCoord[1][1]))
//						}, polyIndex * 3 + vertIndex + 1);
//
//					vertexIndex += 3;
//				}
//			}
//
//			/*CONTROLPOINTS*/
//			for (unsigned int i = 0; i < cpCount; i++) {
//				model->addControlPoint({ (float)cps[i][0], (float)cps[i][1], (float)cps[i][2] }, i);
//			}
//
//
//			/*BONE CONNECTIONS*/
//			int largestIndex = -1;
//			unsigned int deformerCount = mesh->GetDeformerCount();
//			//cout << "deformers: " << to_string(deformerCount) << endl;
//
//
//
//			for (unsigned int deformerIndex = 0; deformerIndex < deformerCount; deformerIndex++) {
//				FbxSkin* skin = reinterpret_cast<FbxSkin*>(mesh->GetDeformer(deformerIndex, FbxDeformer::eSkin));
//				if (!skin) {
//					cout << "not a skin at deformer " << to_string(deformerIndex) << endl;
//					continue;
//				}
//
//				unsigned int clusterCount = skin->GetClusterCount();
//				//cout << "  clusters: " << to_string(clusterCount) << endl;
//
//
//				// CONNECTION AND GLOBALBINDPOSE FOR EACH CONNECTION FOR EACH LIMB
//
//				std::vector<int> limbIndexes;
//				limbIndexes.resize(clusterCount);
//				for (unsigned int clusterIndex = 0; clusterIndex < clusterCount; clusterIndex++) {
//					FbxCluster* cluster = skin->GetCluster(clusterIndex);
//					limbIndexes[clusterIndex] = model->findLimbIndex(cluster->GetLink()->GetUniqueID());
//					if (limbIndexes[clusterIndex] == -1) {
//						cout << "Could not find limb at clusterIndex: " << to_string(clusterIndex) << endl;
//					}
//				}
//
//
//				for (unsigned int clusterIndex = 0; clusterIndex < clusterCount; ++clusterIndex) {
//					FbxCluster* cluster = skin->GetCluster(clusterIndex);
//					FbxAMatrix transformMatrix;
//					FbxAMatrix transformLinkMatrix;
//					FbxAMatrix globalBindposeInverseMatrix;
//
//					cluster->GetTransformMatrix(transformMatrix);	// The transformation of the mesh at binding time
//					cluster->GetTransformLinkMatrix(transformLinkMatrix);	// The transformation of the cluster(joint) at binding time from joint space to world space
//					globalBindposeInverseMatrix = transformLinkMatrix.Inverse() * transformMatrix * geometryTransform;
//
//					// Update the information in mSkeleton 
//					model->setGlobalBindposeInverse(limbIndexes[clusterIndex], convertToXMMatrix(globalBindposeInverseMatrix));
//
//
//					unsigned int indexCount = cluster->GetControlPointIndicesCount();
//					int* CPIndices = cluster->GetControlPointIndices();
//					double* CPWeights = cluster->GetControlPointWeights();
//
//					for (unsigned int index = 0; index < indexCount; ++index) {
//						if (CPIndices[index] > largestIndex)
//							largestIndex = CPIndices[index];
//
//						model->addConnection(CPIndices[index], limbIndexes[clusterIndex], (float)CPWeights[index]);
//
//					}
//				}
//
//
//				// ANIMATION STACK FOR EACH LIMB
//				int stackCount = scene->GetSrcObjectCount<FbxAnimStack>();
//#ifdef PERFORMANCE_TESTING
//				stackCount = 1;
//#endif
//				for (unsigned int clusterIndex = 0; clusterIndex < clusterCount; ++clusterIndex) {
//
//					FbxCluster* cluster = skin->GetCluster(clusterIndex);
//					//cout << filename << " StackSize: " << to_string(stackCount) << endl;
//					for (int currentStack = 0; currentStack < stackCount; currentStack++) {
//						FbxAnimStack* currAnimStack = scene->GetSrcObject<FbxAnimStack>(currentStack);
//
//						FbxTakeInfo* takeInfo = scene->GetTakeInfo(currAnimStack->GetName());
//						node->GetScene()->SetCurrentAnimationStack(currAnimStack);
//						//FbxAnimEvaluator* eval = node->GetAnimationEvaluator();
//						FbxTime start = takeInfo->mLocalTimeSpan.GetStart();
//						FbxTime end = takeInfo->mLocalTimeSpan.GetStop();
//						//cout << "Animation Time: " << to_string((float)takeInfo->mLocalTimeSpan.GetDuration().GetSecondDouble()) << " Frame Count: " << to_string((int)end.GetFrameCount(FbxTime::eFrames24)) << endl;
//						float firstFrameTime = 0.0f;
//						for (FbxLongLong frame = start.GetFrameCount(FbxTime::eFrames24); frame <= end.GetFrameCount(FbxTime::eFrames24); frame++) {
//							FbxTime currTime;
//							currTime.SetFrame(frame, FbxTime::eFrames24);
//							if (firstFrameTime == 0.0f)
//								firstFrameTime = float(currTime.GetSecondDouble());
//							//eval->GetNodeGlobalTransform(node, currTime);
//							//eval->GetNodeGlobalTransform(node, currTime);
//							FbxAMatrix currentTransformOffset = node->EvaluateGlobalTransform(currTime) * geometryTransform;
//							model->addFrame(UINT(currentStack), limbIndexes[clusterIndex], float(currTime.GetSecondDouble()) - firstFrameTime,
//								convertToXMMatrix(currentTransformOffset.Inverse() * cluster->GetLink()->EvaluateGlobalTransform(currTime)));
//						}
//
//					}
//				}
//			}
//		}
//
//	}
//
//
//
//
//

	unsigned int childCount = (unsigned int)node->GetChildCount();
	for (unsigned int child = 0; child < childCount; child++) {
		fetchGeometry(node->GetChild(child), mesh);
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
			getGeometry(mesh, meshData);
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

void FBXLoader::getGeometry(FbxMesh* mesh, Mesh::Data& buildData) {

	// Number of polygon vertices 
	buildData.numVertices = mesh->GetPolygonVertexCount();
	int* indices = mesh->GetPolygonVertices();

	if (int(buildData.numVertices / 3) != mesh->GetPolygonCount()) {
		Logger::Error("The mesh in '" + m_filepath + "' has to be triangulated.");
		return;
	}

	buildData.positions = SAIL_NEW Mesh::vec3[buildData.numVertices];
	buildData.normals = SAIL_NEW Mesh::vec3[buildData.numVertices];
	buildData.texCoords = SAIL_NEW Mesh::vec2[buildData.numVertices];
	buildData.tangents = SAIL_NEW Mesh::vec3[buildData.numVertices];
	buildData.bitangents = SAIL_NEW Mesh::vec3[buildData.numVertices];

	bool norms = true, uvs = true, tangs = true, bitangs = true;

	int vertexIndex = 0;
	FbxVector4* cp = mesh->GetControlPoints();
	if (cp == nullptr) {
		Logger::Error("Couldn't find any vertices in the mesh in the file " + m_filepath);
		return;
	}
	for (int polyIndex = 0; polyIndex < mesh->GetPolygonCount(); polyIndex++) {
		int numVertices = mesh->GetPolygonSize(polyIndex);

		for (int vertIndex = 0; vertIndex < numVertices; vertIndex += 3) {

			/*
			--	Positions
			*/
			buildData.positions[vertexIndex].vec.x = -(float)cp[indices[vertexIndex]][0];
			buildData.positions[vertexIndex].vec.y = (float)cp[indices[vertexIndex]][1];
			buildData.positions[vertexIndex].vec.z = (float)cp[indices[vertexIndex]][2];

			buildData.positions[vertexIndex + 1].vec.x = -(float)cp[indices[vertexIndex + 2]][0];
			buildData.positions[vertexIndex + 1].vec.y = (float)cp[indices[vertexIndex + 2]][1];
			buildData.positions[vertexIndex + 1].vec.z = (float)cp[indices[vertexIndex + 2]][2];

			buildData.positions[vertexIndex + 2].vec.x = -(float)cp[indices[vertexIndex + 1]][0];
			buildData.positions[vertexIndex + 2].vec.y = (float)cp[indices[vertexIndex + 1]][1];
			buildData.positions[vertexIndex + 2].vec.z = (float)cp[indices[vertexIndex + 1]][2];


			/*
			--	Normals
			*/
			FbxLayerElementNormal* leNormal = mesh->GetLayer(0)->GetNormals();
			if (leNormal == nullptr && norms) {
				Logger::Warning("Couldn't find any normals in the mesh in the file " + m_filepath);
				norms = false;
			} else if (norms) {
				if (leNormal->GetMappingMode() == FbxLayerElement::eByPolygonVertex) {
					int normIndex = 0;

					if (leNormal->GetReferenceMode() == FbxLayerElement::eDirect)
						normIndex = vertexIndex;


					if (leNormal->GetReferenceMode() == FbxLayerElement::eIndexToDirect)
						normIndex = leNormal->GetIndexArray().GetAt(vertexIndex);


					FbxVector4 norm = leNormal->GetDirectArray().GetAt(normIndex);
					buildData.normals[vertexIndex].vec.x = -(float)norm[0];
					buildData.normals[vertexIndex].vec.y = (float)norm[1];
					buildData.normals[vertexIndex].vec.z = (float)norm[2];

					norm = leNormal->GetDirectArray().GetAt(normIndex + 2);
					buildData.normals[vertexIndex + 1].vec.x = -(float)norm[0];
					buildData.normals[vertexIndex + 1].vec.y = (float)norm[1];
					buildData.normals[vertexIndex + 1].vec.z = (float)norm[2];

					norm = leNormal->GetDirectArray().GetAt(normIndex + 1);
					buildData.normals[vertexIndex + 2].vec.x = -(float)norm[0];
					buildData.normals[vertexIndex + 2].vec.y = (float)norm[1];
					buildData.normals[vertexIndex + 2].vec.z = (float)norm[2];
				}
			}

			/*
			--	Tangents
			*/
			FbxGeometryElementTangent *geTang = mesh->GetElementTangent(0);
			if (geTang == nullptr && tangs) {
				Logger::Warning("Couldn't find any tangents in the mesh in the file " + m_filepath);
				tangs = false;
			} else if (tangs) {
				if (geTang->GetMappingMode() == FbxLayerElement::eByPolygonVertex) {
					int tangIndex = 0;

					if (geTang->GetReferenceMode() == FbxLayerElement::eDirect)
						tangIndex = vertexIndex;


					if (geTang->GetReferenceMode() == FbxLayerElement::eIndexToDirect)
						tangIndex = geTang->GetIndexArray().GetAt(vertexIndex);

					FbxVector4 tangent = geTang->GetDirectArray().GetAt(tangIndex);
					buildData.tangents[vertexIndex].vec.x = (float)tangent[0];
					buildData.tangents[vertexIndex].vec.y = (float)tangent[1];
					buildData.tangents[vertexIndex].vec.z = (float)tangent[2];

					tangent = geTang->GetDirectArray().GetAt(tangIndex + 2);
					buildData.tangents[vertexIndex + 1].vec.x = (float)tangent[0];
					buildData.tangents[vertexIndex + 1].vec.y = (float)tangent[1];
					buildData.tangents[vertexIndex + 1].vec.z = (float)tangent[2];

					tangent = geTang->GetDirectArray().GetAt(tangIndex + 1);
					buildData.tangents[vertexIndex + 2].vec.x = (float)tangent[0];
					buildData.tangents[vertexIndex + 2].vec.y = (float)tangent[1];
					buildData.tangents[vertexIndex + 2].vec.z = (float)tangent[2];
				}
			}

			/*
			--	Binormals
			*/
			FbxGeometryElementBinormal *geBN = mesh->GetElementBinormal(0);
			if (geBN == nullptr && bitangs) {
				Logger::Warning("Couldn't find any binormals in the mesh in the file " + m_filepath);
				bitangs = false;
			} else if (bitangs) {
				if (geBN->GetMappingMode() == FbxLayerElement::eByPolygonVertex) {
					int biNormIndex = 0;

					if (geBN->GetReferenceMode() == FbxLayerElement::eDirect)
						biNormIndex = vertexIndex;


					if (geBN->GetReferenceMode() == FbxLayerElement::eIndexToDirect)
						biNormIndex = geBN->GetIndexArray().GetAt(vertexIndex);

					FbxVector4 biNorm = geBN->GetDirectArray().GetAt(biNormIndex);
					buildData.bitangents[vertexIndex].vec.x = (float)biNorm[0];
					buildData.bitangents[vertexIndex].vec.y = (float)biNorm[1];
					buildData.bitangents[vertexIndex].vec.z = (float)biNorm[2];

					biNorm = geBN->GetDirectArray().GetAt(biNormIndex + 2);
					buildData.bitangents[vertexIndex + 1].vec.x = (float)biNorm[0];
					buildData.bitangents[vertexIndex + 1].vec.y = (float)biNorm[1];
					buildData.bitangents[vertexIndex + 1].vec.z = (float)biNorm[2];

					biNorm = geBN->GetDirectArray().GetAt(biNormIndex + 1);
					buildData.bitangents[vertexIndex + 2].vec.x = (float)biNorm[0];
					buildData.bitangents[vertexIndex + 2].vec.y = (float)biNorm[1];
					buildData.bitangents[vertexIndex + 2].vec.z = (float)biNorm[2];
				}
			}

			/*
			--	UV Coords
			*/
			FbxGeometryElementUV* geUV = mesh->GetElementUV(0);
			if (geUV == nullptr && uvs) {
				Logger::Warning("Couldn't find any texture coordinates in the mesh in the file " + m_filepath);
				uvs = false;
			} else if (uvs) {
				FbxVector2 texCoord;
				int cpIndex;

				cpIndex = mesh->GetPolygonVertex(polyIndex, vertIndex);
				texCoord = getTexCoord(cpIndex, geUV, mesh, polyIndex, vertIndex);
				buildData.texCoords[vertexIndex].vec.x = static_cast<float>(texCoord[0]);
				buildData.texCoords[vertexIndex].vec.y = -static_cast<float>(texCoord[1]);

				cpIndex = mesh->GetPolygonVertex(polyIndex, vertIndex + 2);
				texCoord = getTexCoord(cpIndex, geUV, mesh, polyIndex, vertIndex + 2);
				buildData.texCoords[vertexIndex + 1].vec.x = static_cast<float>(texCoord[0]);
				buildData.texCoords[vertexIndex + 1].vec.y = -static_cast<float>(texCoord[1]);

				cpIndex = mesh->GetPolygonVertex(polyIndex, vertIndex + 1);
				texCoord = getTexCoord(cpIndex, geUV, mesh, polyIndex, vertIndex + 1);
				buildData.texCoords[vertexIndex + 2].vec.x = static_cast<float>(texCoord[0]);
				buildData.texCoords[vertexIndex + 2].vec.y = -static_cast<float>(texCoord[1]);
			}

			vertexIndex += 3;
		}

	}

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
	if (!pAttribute) return "";

	std::string typeName = GetAttributeTypeName(pAttribute->GetAttributeType());
	std::string attrName = pAttribute->GetName();

	return typeName + " " + attrName;
}

void FBXLoader::printAnimationStack(const FbxNode* node) {
}
