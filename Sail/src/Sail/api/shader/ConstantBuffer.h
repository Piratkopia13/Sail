#pragma once

#include <string>

namespace ShaderComponent {

	class ConstantBuffer {
	public:
		static ConstantBuffer* ConstantBuffer::Create(const std::string& name);
		ConstantBuffer(const std::string& name) {}
		virtual ~ConstantBuffer() {}

		virtual void setData(const void* data, unsigned int size) = 0;
		virtual void bind() const = 0;
	};

}