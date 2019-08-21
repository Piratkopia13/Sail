#pragma once

#include <Windows.h>
#include <vector>
#include <dxcapi.h>

class DXILShaderCompiler {
public:

	struct Desc {
		LPCVOID source = nullptr;
		UINT32 sourceSize;
		LPCWSTR filePath;
		LPCWSTR entryPoint;
		LPCWSTR targetProfile;
		std::vector<LPCWSTR> compileArguments;
		std::vector<DxcDefine> defines;
	};

	DXILShaderCompiler();
	~DXILShaderCompiler();

	HRESULT init();

	// Compiles a shader into a blob
	// Compiles from source if source != nullptr, otherwise from file
	HRESULT compile(Desc* desc, IDxcBlob** ppResult);

private:
	IDxcLibrary* m_library;
	IDxcCompiler* m_compiler;
	IDxcLinker* m_linker;
	IDxcIncludeHandler* m_includeHandler;

};