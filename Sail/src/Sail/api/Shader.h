#pragma once

#include <string>

class Shader {
public:
	enum Type {
		VERTEX = 0,
		PIXEL,
		GEOMETRY,
		TESS_INPUT,
		TESS_OUTPUT,
		COMPUTE
	};

public:
	Shader(const std::string& filename, Type type) {}
	virtual ~Shader() {}

	virtual void compile() const = 0;

protected:

private:

};