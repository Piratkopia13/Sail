#include "pch.h"
#include "DX12Texture.h"

Texture* Texture::Create(const std::string& filename) {
	return new DX12Texture(filename);
}

DX12Texture::DX12Texture(const std::string& filename) {

}

DX12Texture::~DX12Texture() {

}

SailTexture* DX12Texture::getHandle() {
	throw std::logic_error("The method or operation is not implemented.");
}
