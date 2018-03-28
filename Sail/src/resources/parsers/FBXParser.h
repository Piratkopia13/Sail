#pragma once

#include <string>
#include <fbxsdk.h>
#include "../../utils/Utils.h"


class FBXParser {
public:
	FBXParser();
	~FBXParser();

	FbxScene* parseFBX(const std::string& filename);

	FbxManager* getManager();
private:
	FbxManager* m_manager;
	FbxIOSettings* m_ios;
};
