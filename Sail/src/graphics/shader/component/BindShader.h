#pragma once

namespace ShaderComponent {

	enum BIND_SHADER : UINT8 {
		VS = 1,
		PS = 2,
		GS = 4,
		CS = 8,
		DS = 16,
		HS = 32
	};

}