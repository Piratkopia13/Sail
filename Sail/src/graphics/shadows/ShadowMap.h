#pragma once

#include "../shader/ShaderSet.h"
#include "../RenderableTexture.h"
#include "../../utils/Utils.h"
#include "../camera/Camera.h"

class ShadowMap{

public:
	ShadowMap();
	virtual ~ShadowMap();

	//virtual void updateTexture(RenderableTexture& depthTexture) = 0;

protected:
	ShaderSet* m_shader;
	Camera* m_cam;
	int resolution[2];

};