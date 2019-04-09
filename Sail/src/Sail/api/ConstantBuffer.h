#pragma once

#include <string>

class ConstantBuffer {
public:
	ConstantBuffer(const std::string& name) {}
	virtual ~ConstantBuffer() {}

	virtual void setData(const void* data, unsigned int size) = 0;
	virtual void bind() const = 0;

protected:

private:

};