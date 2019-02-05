#include "pch.h"
#include "DXCubeMap.h"
#include "../api/Application.h"

using namespace DirectX;
using namespace Microsoft::WRL;

DXCubeMap::DXCubeMap(const std::wstring& filename) {

	auto dxm = Application::getInstance()->getAPI();

	DirectX::CreateDDSTextureFromFile(dxm->getDevice(), (DEFAULT_CUBEMAP_LOCATION + filename).c_str(), (ID3D11Resource**)&m_texture, &m_resourceView);

}
DXCubeMap::~DXCubeMap() {
	Memory::safeRelease(m_texture);
	Memory::safeRelease(m_resourceView);
}

ID3D11ShaderResourceView** DXCubeMap::getResourceView() {
	return &m_resourceView;
}
ID3D11Texture3D* DXCubeMap::getTexture3D() {
	return m_texture;
}