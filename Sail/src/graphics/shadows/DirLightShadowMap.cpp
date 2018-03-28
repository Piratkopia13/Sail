#include "DirLightShadowMap.h"

DirLightShadowMap::DirLightShadowMap(int width, int height)
: m_depthTexture(1, width, height, true, true){
}

DirLightShadowMap::~DirLightShadowMap(){
}

ID3D11DepthStencilView* const * DirLightShadowMap::getDSV() {
	return m_depthTexture.getDepthStencilView();
}

ID3D11ShaderResourceView** DirLightShadowMap::getSRV() {
	return m_depthTexture.getDepthSRV();
}

D3D11_VIEWPORT* DirLightShadowMap::getViewPort() {
	return m_depthTexture.getViewPort();
}

ID3D11Texture2D * DirLightShadowMap::getTexture2D() {
	return m_depthTexture.getDepthTexture2D();
}

//DeferredDirectionalLightShader* DirLightShadowMap::getDLShader() {
//	return m_shader;
//}