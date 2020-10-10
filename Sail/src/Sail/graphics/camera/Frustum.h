#pragma once
#include <glm/glm.hpp>

struct Frustum {
	// In order of left, right, bottom, top, near, far
	glm::vec4 planes[6];

	void extractPlanes(const glm::mat4& vp) {
		// In order of left, right, bottom, top, near, far
		planes[0].x = -(vp[0][3] + vp[0][0]);
		planes[0].y = -(vp[1][3] + vp[1][0]);
		planes[0].z = -(vp[2][3] + vp[2][0]);
		planes[0].w = -(vp[3][3] + vp[3][0]);

		planes[1].x = -(vp[0][3] - vp[0][0]);
		planes[1].y = -(vp[1][3] - vp[1][0]);
		planes[1].z = -(vp[2][3] - vp[2][0]);
		planes[1].w = -(vp[3][3] - vp[3][0]);

		planes[2].x = -(vp[0][3] + vp[0][1]);
		planes[2].y = -(vp[1][3] + vp[1][1]);
		planes[2].z = -(vp[2][3] + vp[2][1]);
		planes[2].w = -(vp[3][3] + vp[3][1]);

		planes[3].x = -(vp[0][3] - vp[0][1]);
		planes[3].y = -(vp[1][3] - vp[1][1]);
		planes[3].z = -(vp[2][3] - vp[2][1]);
		planes[3].w = -(vp[3][3] - vp[3][1]);

		planes[4].x = -(vp[0][2] + vp[0][2]);
		planes[4].y = -(vp[1][2] + vp[1][2]);
		planes[4].z = -(vp[2][2] + vp[2][2]);
		planes[4].w = -(vp[3][2] + vp[3][2]);

		planes[5].x = -(vp[0][3] - vp[0][2]);
		planes[5].y = -(vp[1][3] - vp[1][2]);
		planes[5].z = -(vp[2][3] - vp[2][2]);
		planes[5].w = -(vp[3][3] - vp[3][2]);
	}
};