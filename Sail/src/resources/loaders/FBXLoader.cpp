#include "FBXLoader.h"

#include <d3d11.h>
#include <SimpleMath.h>
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
			// Gets the mesh (only one mesh per model is supported)
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

void FBXLoader::getGeometry(FbxMesh* mesh, Mesh::Data& buildData) {

	// Number of polygon vertices 
	buildData.numVertices = mesh->GetPolygonVertexCount();
	int* indices = mesh->GetPolygonVertices();

	if (int(buildData.numVertices / 3) != mesh->GetPolygonCount()) {
		Logger::Error("The mesh in '" + m_filepath + "' has to be triangulated.");
		return;
	}

	buildData.positions = new Vector3[buildData.numVertices];
	buildData.normals = new Vector3[buildData.numVertices];
	buildData.texCoords = new Vector2[buildData.numVertices];
	buildData.tangents = new Vector3[buildData.numVertices];
	buildData.bitangents = new Vector3[buildData.numVertices];

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
			buildData.positions[vertexIndex].x = -(float)cp[indices[vertexIndex]][0];
			buildData.positions[vertexIndex].y = (float)cp[indices[vertexIndex]][1];
			buildData.positions[vertexIndex].z = (float)cp[indices[vertexIndex]][2];

			buildData.positions[vertexIndex + 1].x = -(float)cp[indices[vertexIndex + 2]][0];
			buildData.positions[vertexIndex + 1].y = (float)cp[indices[vertexIndex + 2]][1];
			buildData.positions[vertexIndex + 1].z = (float)cp[indices[vertexIndex + 2]][2];

			buildData.positions[vertexIndex + 2].x = -(float)cp[indices[vertexIndex + 1]][0];
			buildData.positions[vertexIndex + 2].y = (float)cp[indices[vertexIndex + 1]][1];
			buildData.positions[vertexIndex + 2].z = (float)cp[indices[vertexIndex + 1]][2];


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
					buildData.normals[vertexIndex].x = -(float)norm[0];
					buildData.normals[vertexIndex].y = (float)norm[1];
					buildData.normals[vertexIndex].z = (float)norm[2];

					norm = leNormal->GetDirectArray().GetAt(normIndex + 2);
					buildData.normals[vertexIndex + 1].x = -(float)norm[0];
					buildData.normals[vertexIndex + 1].y = (float)norm[1];
					buildData.normals[vertexIndex + 1].z = (float)norm[2];

					norm = leNormal->GetDirectArray().GetAt(normIndex + 1);
					buildData.normals[vertexIndex + 2].x = -(float)norm[0];
					buildData.normals[vertexIndex + 2].y = (float)norm[1];
					buildData.normals[vertexIndex + 2].z = (float)norm[2];
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
					buildData.tangents[vertexIndex].x = (float)tangent[0];
					buildData.tangents[vertexIndex].y = (float)tangent[1];
					buildData.tangents[vertexIndex].z = (float)tangent[2];

					tangent = geTang->GetDirectArray().GetAt(tangIndex + 2);
					buildData.tangents[vertexIndex + 1].x = (float)tangent[0];
					buildData.tangents[vertexIndex + 1].y = (float)tangent[1];
					buildData.tangents[vertexIndex + 1].z = (float)tangent[2];

					tangent = geTang->GetDirectArray().GetAt(tangIndex + 1);
					buildData.tangents[vertexIndex + 2].x = (float)tangent[0];
					buildData.tangents[vertexIndex + 2].y = (float)tangent[1];
					buildData.tangents[vertexIndex + 2].z = (float)tangent[2];
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
					buildData.bitangents[vertexIndex].x = (float)biNorm[0];
					buildData.bitangents[vertexIndex].y = (float)biNorm[1];
					buildData.bitangents[vertexIndex].z = (float)biNorm[2];

					biNorm = geBN->GetDirectArray().GetAt(biNormIndex + 2);
					buildData.bitangents[vertexIndex + 1].x = (float)biNorm[0];
					buildData.bitangents[vertexIndex + 1].y = (float)biNorm[1];
					buildData.bitangents[vertexIndex + 1].z = (float)biNorm[2];

					biNorm = geBN->GetDirectArray().GetAt(biNormIndex + 1);
					buildData.bitangents[vertexIndex + 2].x = (float)biNorm[0];
					buildData.bitangents[vertexIndex + 2].y = (float)biNorm[1];
					buildData.bitangents[vertexIndex + 2].z = (float)biNorm[2];
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
				buildData.texCoords[vertexIndex].x = static_cast<float>(texCoord[0]);
				buildData.texCoords[vertexIndex].y = -static_cast<float>(texCoord[1]);

				cpIndex = mesh->GetPolygonVertex(polyIndex, vertIndex + 2);
				texCoord = getTexCoord(cpIndex, geUV, mesh, polyIndex, vertIndex + 2);
				buildData.texCoords[vertexIndex + 1].x = static_cast<float>(texCoord[0]);
				buildData.texCoords[vertexIndex + 1].y = -static_cast<float>(texCoord[1]);

				cpIndex = mesh->GetPolygonVertex(polyIndex, vertIndex + 1);
				texCoord = getTexCoord(cpIndex, geUV, mesh, polyIndex, vertIndex + 1);
				buildData.texCoords[vertexIndex + 2].x = static_cast<float>(texCoord[0]);
				buildData.texCoords[vertexIndex + 2].y = -static_cast<float>(texCoord[1]);
			}

			vertexIndex += 3;
		}

	}

}

void FBXLoader::getMaterial(FbxNode* pNode, Material* material) {

	// Gets the model's phong constants
	if (pNode->GetSrcObjectCount<FbxSurfacePhong>() > 0) {

		auto phong = pNode->GetSrcObject<FbxSurfacePhong>();

		material->setKa(static_cast<float>(phong->AmbientFactor.Get()));
		material->setKs(static_cast<float>(phong->SpecularFactor.Get()));
		material->setKd (static_cast<float>(phong->DiffuseFactor.Get()));
		material->setShininess(static_cast<float>(phong->Shininess.Get()));

	}

	auto storeTextureNames = [&](const std::string& filename) {
		if (filename.find("specular") != std::string::npos) {
			Application::getInstance()->getResourceManager().loadDXTexture(filename);
			material->setSpecularTexture(filename);
		}

		if (filename.find("diffuse") != std::string::npos) {
			Application::getInstance()->getResourceManager().loadDXTexture(filename);
			material->setDiffuseTexture(filename);
		}

		if (filename.find("normal") != std::string::npos) {
			Application::getInstance()->getResourceManager().loadDXTexture(filename);
			material->setNormalTexture(filename);
		}
	};

	int materialIndex;
	FbxProperty fbxProperty;
	int materialCount = pNode->GetSrcObjectCount<FbxSurfaceMaterial>();

	// Loops through all materials to find the textures of the model
	for (materialIndex = 0; materialIndex < materialCount; materialIndex++) {
		FbxSurfaceMaterial* material = pNode->GetSrcObject<FbxSurfaceMaterial>(materialIndex);
		if (material) {
			int textureIndex;
			FBXSDK_FOR_EACH_TEXTURE(textureIndex) {
				fbxProperty = material->FindProperty(FbxLayerElement::sTextureChannelNames[textureIndex]);
				if (fbxProperty.IsValid()) {
					int textureCount = fbxProperty.GetSrcObjectCount<FbxTexture>();
					for (int i = 0; i < textureCount; ++i) {
						FbxLayeredTexture* layeredTexture = fbxProperty.GetSrcObject<FbxLayeredTexture>(i);
						// Checks if there's multiple layers to the texture
						if (layeredTexture) {
							int numTextures = layeredTexture->GetSrcObjectCount<FbxTexture>();

							// Loops through all the textures of the material (Engine currently only supports one texture per spec/diff/norm.)
							for (int j = 0; j < numTextures; j++) {

								FbxTexture* texture = layeredTexture->GetSrcObject<FbxTexture>(j);
								FbxFileTexture* fileTexture = FbxCast<FbxFileTexture>(texture);
								std::string filename = fileTexture->GetRelativeFileName();

								storeTextureNames(filename);

							}
						}
						// If it isn't a layered texture, just load the textures right away.
						else {

							FbxTexture* texture = fbxProperty.GetSrcObject<FbxTexture>(i);
							FbxFileTexture* fileTexture = FbxCast<FbxFileTexture>(texture);
							std::string filename = fileTexture->GetRelativeFileName();

							Logger::Log(filename + " is of texture use: " + std::string(texture->GetTextureType()));

							storeTextureNames(filename);

						}
					}
				}
			}
		}
	}
}
