#include "FBXLoader.h"

#include "../../utils/Utils.h"
#include "../../graphics/geometry/factory/CubeModel.h"
#include "../../api/Application.h"

using namespace DirectX::SimpleMath;

FbxManager* FBXLoader::s_manager = FbxManager::Create();
FbxIOSettings* FBXLoader::s_ios = FbxIOSettings::Create(s_manager, IOSROOT);

FBXLoader::FBXLoader(const std::string& filepath, ShaderSet* shaderSet) 
	: m_filepath(filepath)
	, m_shaderSet(shaderSet)
{
	s_manager->SetIOSettings(s_ios);

	m_model = std::make_unique<Model>();
	m_scene = parseFBX(filepath);
	
	// Triangulate all meshes in the scene
	/*FbxGeometryConverter geoConverter(s_manager);
	geoConverter.Triangulate(m_scene, true);*/

	if (m_scene != nullptr) {

		loadNode(m_scene->GetRootNode());

	} else {
		Logger::Warning("Failed to load fbx file '" + filepath + "', using default cube.");
		DirectX::SimpleMath::Vector3 halfSizes = DirectX::SimpleMath::Vector3(0.5, 0.5, 0.5);
		m_model = ModelFactory::CubeModel::Create(halfSizes, shaderSet);
	}
}

FBXLoader::~FBXLoader() {
	if (m_scene)
		m_scene->Destroy();
}

std::unique_ptr<Model>& FBXLoader::getModel() {
	return m_model;
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
			// Get the mesh
			mesh = (FbxMesh*)nodeAttributeFbx;

			Mesh::Data meshData;
			getGeometry(mesh, meshData);
			std::unique_ptr<Mesh> mesh = std::make_unique<Mesh>(meshData, m_shaderSet);
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

bool FBXLoader::findIndex(const Mesh::Data& buildData, unsigned int numVertices, const VertexData& vertexData, unsigned int& foundIndex) {
	
	for (unsigned int i = 0; i < numVertices; i++) {
		if (buildData.positions[i] == vertexData.position &&
			(!buildData.normals || buildData.normals[i] == vertexData.normal) &&
			(!buildData.tangents || buildData.tangents[i] == vertexData.tangent) &&
			(!buildData.bitangents || buildData.bitangents[i] == vertexData.bitangent) &&
			(!buildData.texCoords || buildData.texCoords[i] == vertexData.texCoord)) {
			foundIndex = i;
			return true;
		}
	}
	return false;
}

void FBXLoader::getGeometry(FbxMesh* mesh, Mesh::Data& buildData) {

	// Number of polygon vertices 
	//buildData.numIndices = mesh->GetPolygonVertexCount(); // A polygon vertex is an index to a control point
	//buildData.numVertices = mesh->GetControlPointsCount(); // A control point is an XYZ coordinate, it is synonym of vertex.
	buildData.numIndices = 0;
	buildData.numVertices = 0;

	unsigned int maxVertices = mesh->GetPolygonVertexCount();
	//maxVertices = mesh->GetPolygonVertexCount();

	Logger::Log(std::string(mesh->GetName()) + " # verts: " + std::to_string(maxVertices));

	if (maxVertices / 3 != mesh->GetPolygonCount()) {
		Logger::Error("The mesh in '" + m_filepath + "' has to be triangulated.");
		return;
	}

	buildData.positions = new Vector3[maxVertices];
	buildData.indices = new ULONG[maxVertices]; // Allocate the max memory we might need

	FbxLayerElementNormal* vertexNormal = mesh->GetLayer(0)->GetNormals();
	if (vertexNormal)	buildData.normals = new Vector3[maxVertices];
	else				Logger::Warning("Couldn't find any normals in the mesh in the file " + m_filepath);

	FbxGeometryElementTangent* vertexTangent = mesh->GetElementTangent(0);
	if (vertexTangent)	buildData.tangents = new Vector3[maxVertices];
	else				Logger::Warning("Couldn't find any tangents in the mesh in the file " + m_filepath);
	
	FbxGeometryElementBinormal* vertexBinormal = mesh->GetElementBinormal(0);
	if (vertexBinormal)	buildData.bitangents = new Vector3[maxVertices];
	else				Logger::Warning("Couldn't find any binormals in the mesh in the file " + m_filepath);
	
	FbxGeometryElementUV* vertexUV = mesh->GetElementUV(0);
	if (vertexUV)		buildData.texCoords = new Vector2[maxVertices];
	else				Logger::Warning("Couldn't find any texture coordinates in the mesh in the file " + m_filepath);
	

	FbxVector4* cp = mesh->GetControlPoints();
	if (cp == nullptr) {
		Logger::Error("Couldn't find any vertices in the mesh in the file " + m_filepath);
		return;
	}

	int* indices = mesh->GetPolygonVertices();
	/*for (unsigned int i = 0; i < buildData.numIndices; i+=3) {
		buildData.indices[i] = indices[i];
		buildData.indices[i + 1] = indices[i + 2];
		buildData.indices[i + 2] = indices[i + 1];
	}*/


	for (int polyIndex = 0; polyIndex < mesh->GetPolygonCount(); polyIndex++) {
		int numVertices = mesh->GetPolygonSize(polyIndex);

		for (int vertIndex = 0; vertIndex < numVertices; vertIndex++) {

			unsigned int vertexIndex = mesh->GetPolygonVertexIndex(polyIndex) + vertIndex;

			VertexData vertexData;


			/*
			--	Positions
			*/
			vertexData.position.x = static_cast<float>(cp[indices[vertexIndex]][0]);
			vertexData.position.y = static_cast<float>(cp[indices[vertexIndex]][1]);
			vertexData.position.z = static_cast<float>(cp[indices[vertexIndex]][2]);

			/*
			--	Normals
			*/
			if (buildData.normals) {
				if (vertexNormal->GetMappingMode() == FbxLayerElement::eByPolygonVertex) {

					int normIndex = 0;
					if (vertexNormal->GetReferenceMode() == FbxLayerElement::eDirect)
						normIndex = vertexIndex;
					if (vertexNormal->GetReferenceMode() == FbxLayerElement::eIndexToDirect)
						normIndex = vertexNormal->GetIndexArray().GetAt(vertexIndex);

					FbxVector4& norm = vertexNormal->GetDirectArray().GetAt(normIndex);
					vertexData.normal.x = static_cast<float>(norm[0]);
					vertexData.normal.y = static_cast<float>(norm[1]);
					vertexData.normal.z = static_cast<float>(norm[2]);

				}
			}

			/*
			--	Tangents
			*/
			if (buildData.tangents) {
				if (vertexTangent->GetMappingMode() == FbxLayerElement::eByPolygonVertex) {

					int tangIndex = 0;
					if (vertexTangent->GetReferenceMode() == FbxLayerElement::eDirect)
						tangIndex = vertexIndex;
					if (vertexTangent->GetReferenceMode() == FbxLayerElement::eIndexToDirect)
						tangIndex = vertexTangent->GetIndexArray().GetAt(vertexIndex);

					FbxVector4 tangent = vertexTangent->GetDirectArray().GetAt(tangIndex);
					vertexData.tangent.x = static_cast<float>(tangent[0]);
					vertexData.tangent.y = static_cast<float>(tangent[1]);
					vertexData.tangent.z = static_cast<float>(tangent[2]);

				}
			}

			/*
			--	Binormals
			*/
			if (buildData.bitangents) {
				if (vertexBinormal->GetMappingMode() == FbxLayerElement::eByPolygonVertex) {

					int biNormIndex = 0;
					if (vertexBinormal->GetReferenceMode() == FbxLayerElement::eDirect)
						biNormIndex = vertexIndex;
					if (vertexBinormal->GetReferenceMode() == FbxLayerElement::eIndexToDirect)
						biNormIndex = vertexBinormal->GetIndexArray().GetAt(vertexIndex);

					FbxVector4 biNorm = vertexBinormal->GetDirectArray().GetAt(biNormIndex);
					vertexData.bitangent.x = static_cast<float>(biNorm[0]);
					vertexData.bitangent.y = static_cast<float>(biNorm[1]);
					vertexData.bitangent.z = static_cast<float>(biNorm[2]);

				}
			}

			/*
			--	UV Coords
			*/
			if (buildData.texCoords) {
				FbxVector2 texCoord;
				int cpIndex;

				cpIndex = mesh->GetPolygonVertex(polyIndex, vertIndex);
				texCoord = getTexCoord(cpIndex, vertexUV, mesh, polyIndex, vertIndex);
				vertexData.texCoord.x = static_cast<float>(texCoord[0]);
				vertexData.texCoord.y = -static_cast<float>(texCoord[1]);

			}

			unsigned int foundIndex = 0;
			if (findIndex(buildData, buildData.numVertices, vertexData, foundIndex)) {
				// add foundIndex to buildData indices, but no vertex data
				buildData.indices[buildData.numIndices++] = foundIndex;
			} else {
				// add vertex data and index
				buildData.positions[buildData.numVertices] = vertexData.position;
				if (buildData.normals) buildData.normals[buildData.numVertices] = vertexData.normal;
				if (buildData.tangents) buildData.tangents[buildData.numVertices] = vertexData.tangent;
				if (buildData.bitangents) buildData.bitangents[buildData.numVertices] = vertexData.bitangent;
				if (buildData.texCoords) buildData.texCoords[buildData.numVertices] = vertexData.texCoord;
				buildData.indices[buildData.numIndices++] = buildData.numVertices++;
			}

		}
	}

	/*for (unsigned int i = 0; i < buildData.numIndices; i+=3) {
		UINT temp = buildData.indices[i + 1];
		buildData.indices[i + 1] = buildData.indices[i + 2];
		buildData.indices[i + 2] = temp;
	}*/

}

void FBXLoader::getMaterial(FbxNode* pNode, Material* material) {

	// Gets the model's phong constants
	if (pNode->GetSrcObjectCount<FbxSurfacePhong>() > 0) {

		auto phong = pNode->GetSrcObject<FbxSurfacePhong>();
		auto& resman = Application::getInstance()->getResourceManager();

		FbxFileTexture* diffTex = phong->Diffuse.GetSrcObject<FbxFileTexture>();
		if (diffTex) {
			std::string filename = diffTex->GetRelativeFileName();
			resman.loadDXTexture(filename);
			material->setDiffuseTexture(filename);
		}
		FbxFileTexture* specTex = phong->Specular.GetSrcObject<FbxFileTexture>();
		if (specTex) {
			std::string filename = specTex->GetRelativeFileName();
			resman.loadDXTexture(filename);
			material->setSpecularTexture(specTex->GetRelativeFileName());
		}
		FbxFileTexture* normTex = phong->NormalMap.GetSrcObject<FbxFileTexture>();
		if (normTex) {
			std::string filename = normTex->GetRelativeFileName();
			resman.loadDXTexture(filename);
			material->setNormalTexture(normTex->GetRelativeFileName());
		}

		material->setKa(static_cast<float>(phong->AmbientFactor.Get()));
		material->setKs(static_cast<float>(phong->SpecularFactor.Get()));
		material->setKd (static_cast<float>(phong->DiffuseFactor.Get()));
		material->setShininess(static_cast<float>(phong->Shininess.Get()));

	}

}
