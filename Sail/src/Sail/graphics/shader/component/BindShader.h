#pragma once

namespace ShaderComponent {

	enum BIND_SHADER : unsigned char {
		VS = 1,
		PS = 2,
		GS = 4,
		CS = 8,
		DS = 16,
		HS = 32
	};

}