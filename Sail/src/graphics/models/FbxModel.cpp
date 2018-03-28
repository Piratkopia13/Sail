#include "FbxModel.h"

FbxModel::FbxModel(const std::string& filename) {
	m_scene = Application::getInstance()->getResourceManager().getFBXParser().parseFBX(DEFAULT_MODEL_LOCATION + filename);
	if (m_scene != nullptr) {
		//m_model = std::make_unique<Model>(); // TODO: FIX
		loadNode(m_scene->GetRootNode(), filename);

		//m_model->setBuildData(m_buildData); // TODO: FIX
	}
	else {
		Logger::Warning("Failed to load fbx file '" + filename + "', using default cube.");
		DirectX::SimpleMath::Vector3 halfSizes = DirectX::SimpleMath::Vector3(0.5, 0.5, 0.5);
		//m_model = ModelFactory::CubeModel::Create(halfSizes); //TODO: uncomment and fix
	}
}

FbxModel::~FbxModel() {
	if (m_scene != nullptr)
		m_scene->Destroy();
}

FbxVector2 FbxModel::getTexCoord(int cpIndex, FbxGeometryElementUV* geUV, FbxMesh* mesh, int polyIndex, int vertIndex) const {
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

void FbxModel::fetchGeometry(FbxMesh* mesh, const std::string& filename) {

	// Number of polygon vertices 
	m_buildData.numVertices = mesh->GetPolygonVertexCount();
	int* indices = mesh->GetPolygonVertices();

	if (int(m_buildData.numVertices / 3) != mesh->GetPolygonCount()) {
		Logger::Error("The mesh in '" + filename + "' has to be triangulated.");
		return;
	}

	m_buildData.positions = new DirectX::SimpleMath::Vector3[m_buildData.numVertices];
	m_buildData.normals = new DirectX::SimpleMath::Vector3[m_buildData.numVertices];
	m_buildData.texCoords = new DirectX::SimpleMath::Vector2[m_buildData.numVertices];
	m_buildData.tangents = new DirectX::SimpleMath::Vector3[m_buildData.numVertices];
	m_buildData.bitangents = new DirectX::SimpleMath::Vector3[m_buildData.numVertices];

	bool norms = true, uvs = true, tangs = true, bitangs = true;

	int vertexIndex = 0;
	FbxVector4* cp = mesh->GetControlPoints();
	if (cp == nullptr) {
		Logger::Error("Couldn't find any vertices in the mesh in the file " + filename);
		return;
	}
	for (int polyIndex = 0; polyIndex < mesh->GetPolygonCount(); polyIndex++) {
		int numVertices = mesh->GetPolygonSize(polyIndex);

		for (int vertIndex = 0; vertIndex < numVertices; vertIndex += 3) {

			/*
			--	Positions
			*/
			m_buildData.positions[vertexIndex].x = -(float)cp[indices[vertexIndex]][0];
			m_buildData.positions[vertexIndex].y = (float)cp[indices[vertexIndex]][1];
			m_buildData.positions[vertexIndex].z = (float)cp[indices[vertexIndex]][2];

			m_buildData.positions[vertexIndex + 1].x = -(float)cp[indices[vertexIndex + 2]][0];
			m_buildData.positions[vertexIndex + 1].y = (float)cp[indices[vertexIndex + 2]][1];
			m_buildData.positions[vertexIndex + 1].z = (float)cp[indices[vertexIndex + 2]][2];

			m_buildData.positions[vertexIndex + 2].x = -(float)cp[indices[vertexIndex + 1]][0];
			m_buildData.positions[vertexIndex + 2].y = (float)cp[indices[vertexIndex + 1]][1];
			m_buildData.positions[vertexIndex + 2].z = (float)cp[indices[vertexIndex + 1]][2];


			/*
			--	Normals
			*/
			FbxLayerElementNormal* leNormal = mesh->GetLayer(0)->GetNormals();
			if (leNormal == nullptr && norms) {
				Logger::Warning("Couldn't find any normals in the mesh in the file " + filename);
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
					m_buildData.normals[vertexIndex].x = -(float)norm[0];
					m_buildData.normals[vertexIndex].y = (float)norm[1];
					m_buildData.normals[vertexIndex].z = (float)norm[2];

					norm = leNormal->GetDirectArray().GetAt(normIndex + 2);
					m_buildData.normals[vertexIndex + 1].x = -(float)norm[0];
					m_buildData.normals[vertexIndex + 1].y = (float)norm[1];
					m_buildData.normals[vertexIndex + 1].z = (float)norm[2];

					norm = leNormal->GetDirectArray().GetAt(normIndex + 1);
					m_buildData.normals[vertexIndex + 2].x = -(float)norm[0];
					m_buildData.normals[vertexIndex + 2].y = (float)norm[1];
					m_buildData.normals[vertexIndex + 2].z = (float)norm[2];
				}
			}

			/*
			--	Tangents
			*/
			FbxGeometryElementTangent *geTang = mesh->GetElementTangent(0);
			if (geTang == nullptr && tangs) {
				Logger::Warning("Couldn't find any tangents in the mesh in the file " + filename);
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
					m_buildData.tangents[vertexIndex].x = (float)tangent[0];
					m_buildData.tangents[vertexIndex].y = (float)tangent[1];
					m_buildData.tangents[vertexIndex].z = (float)tangent[2];

					tangent = geTang->GetDirectArray().GetAt(tangIndex + 2);
					m_buildData.tangents[vertexIndex + 1].x = (float)tangent[0];
					m_buildData.tangents[vertexIndex + 1].y = (float)tangent[1];
					m_buildData.tangents[vertexIndex + 1].z = (float)tangent[2];

					tangent = geTang->GetDirectArray().GetAt(tangIndex + 1);
					m_buildData.tangents[vertexIndex + 2].x = (float)tangent[0];
					m_buildData.tangents[vertexIndex + 2].y = (float)tangent[1];
					m_buildData.tangents[vertexIndex + 2].z = (float)tangent[2];
				}
			}

			/*
			--	Binormals
			*/
			FbxGeometryElementBinormal *geBN = mesh->GetElementBinormal(0);
			if (geBN == nullptr && bitangs) {
				Logger::Warning("Couldn't find any binormals in the mesh in the file " + filename);
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
					m_buildData.bitangents[vertexIndex].x = (float)biNorm[0];
					m_buildData.bitangents[vertexIndex].y = (float)biNorm[1];
					m_buildData.bitangents[vertexIndex].z = (float)biNorm[2];

					biNorm = geBN->GetDirectArray().GetAt(biNormIndex + 2);
					m_buildData.bitangents[vertexIndex + 1].x = (float)biNorm[0];
					m_buildData.bitangents[vertexIndex + 1].y = (float)biNorm[1];
					m_buildData.bitangents[vertexIndex + 1].z = (float)biNorm[2];

					biNorm = geBN->GetDirectArray().GetAt(biNormIndex + 1);
					m_buildData.bitangents[vertexIndex + 2].x = (float)biNorm[0];
					m_buildData.bitangents[vertexIndex + 2].y = (float)biNorm[1];
					m_buildData.bitangents[vertexIndex + 2].z = (float)biNorm[2];
				}
			}

			/*
			--	UV Coords
			*/
			FbxGeometryElementUV* geUV = mesh->GetElementUV(0);
			if (geUV == nullptr && uvs) {
				Logger::Warning("Couldn't find any texture coordinates in the mesh in the file " + filename);
				uvs = false;
			}
			else if (uvs) {
				FbxVector2 texCoord;
				int cpIndex;

				cpIndex = mesh->GetPolygonVertex(polyIndex, vertIndex);
				texCoord = getTexCoord(cpIndex, geUV, mesh, polyIndex, vertIndex);
				m_buildData.texCoords[vertexIndex].x = static_cast<float>(texCoord[0]);
				m_buildData.texCoords[vertexIndex].y = -static_cast<float>(texCoord[1]);

				cpIndex = mesh->GetPolygonVertex(polyIndex, vertIndex + 2);
				texCoord = getTexCoord(cpIndex, geUV, mesh, polyIndex, vertIndex + 2);
				m_buildData.texCoords[vertexIndex + 1].x = static_cast<float>(texCoord[0]);
				m_buildData.texCoords[vertexIndex + 1].y = -static_cast<float>(texCoord[1]);

				cpIndex = mesh->GetPolygonVertex(polyIndex, vertIndex + 1);
				texCoord = getTexCoord(cpIndex, geUV, mesh, polyIndex, vertIndex + 1);
				m_buildData.texCoords[vertexIndex + 2].x = static_cast<float>(texCoord[0]);
				m_buildData.texCoords[vertexIndex + 2].y = -static_cast<float>(texCoord[1]);
			}

			vertexIndex += 3;
		}

	}

}

void FbxModel::fetchMaterial(FbxNode* pNode, const std::string& fbxFilename) {

	// Gets the model's phong constants
	if (pNode->GetSrcObjectCount<FbxSurfacePhong>() > 0) {

		auto phong = pNode->GetSrcObject<FbxSurfacePhong>();

		m_model->getMaterial()->setKa(static_cast<float>(phong->AmbientFactor.Get()));
		m_model->getMaterial()->setKs(static_cast<float>(phong->SpecularFactor.Get()));
		m_model->getMaterial()->setKd(static_cast<float>(phong->DiffuseFactor.Get()));

		m_model->getMaterial()->setShininess(static_cast<float>(phong->Shininess.Get()));

	}

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

								if (filename.find("specular")) {
									Application::getInstance()->getResourceManager().LoadDXTexture(filename);								
									getModel()->getMaterial()->setSpecularTexture(filename);
								}

								if (filename.find("diffuse")) {
									Application::getInstance()->getResourceManager().LoadDXTexture(filename);
									getModel()->getMaterial()->setDiffuseTexture(filename);
								}

								if (filename.find("normal")) {
									Application::getInstance()->getResourceManager().LoadDXTexture(filename);									
									getModel()->getMaterial()->setNormalTexture(filename);
								}

							}
						}
						// If it isn't a layered texture, just load the textures right away.
						else {

							FbxTexture* texture = fbxProperty.GetSrcObject<FbxTexture>(i);

							FbxFileTexture* fileTexture = FbxCast<FbxFileTexture>(texture);

							std::string filename = fileTexture->GetRelativeFileName();

							if (filename.find("specular") != std::string::npos) {
								Application::getInstance()->getResourceManager().LoadDXTexture(filename);								
								getModel()->getMaterial()->setSpecularTexture(filename);
							}
							
							if (filename.find("diffuse") != std::string::npos) {
								Application::getInstance()->getResourceManager().LoadDXTexture(filename);
								getModel()->getMaterial()->setDiffuseTexture(filename);
							}

							if (filename.find("normal") != std::string::npos) {
								Application::getInstance()->getResourceManager().LoadDXTexture(filename);
								getModel()->getMaterial()->setNormalTexture(filename);
							}

						}
					}
				}
			}
		}
	}
}

void FbxModel::loadNode(FbxNode* pNode, const std::string& filename) {
	// The number of attributes for the current node (mesh for example)
	int numAttributes = pNode->GetNodeAttributeCount();

	for (int j = 0; j < numAttributes; j++) {
		FbxNodeAttribute *nodeAttributeFbx = pNode->GetNodeAttributeByIndex(j);
		FbxNodeAttribute::EType attributeType = nodeAttributeFbx->GetAttributeType();

		FbxMesh* mesh;
		switch (attributeType) {

		case FbxNodeAttribute::eMesh:
			// Gets the mesh (only one mesh per model is supported)
			mesh = (FbxMesh*)nodeAttributeFbx;
			fetchGeometry(mesh, filename);
			fetchMaterial(pNode, filename);
			break;

			// To load skeleton data for future versions of the engine
		case FbxNodeAttribute::eSkeleton:
			break;

		}
	}

	// Recursively load the child nodes
	int numChildren = pNode->GetChildCount();
	for (int i = 0; i < numChildren; i++)
		this->loadNode(pNode->GetChild(i), filename);
}

Model* FbxModel::getModel() {
	return m_model.get();
}