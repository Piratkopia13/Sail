#pragma once

namespace ShaderComponent {

	enum BIND_SHADER : uint16_t {
		VS = 1 << 0,
		PS = 1 << 1,
		GS = 1 << 2,
		CS = 1 << 3,
		DS = 1 << 4,
		HS = 1 << 5,
		RAY_GEN			= 1 << 6,
		RAY_CLOSEST_HIT = 1 << 7,
		RAY_MISS		= 1 << 8,
	};

}