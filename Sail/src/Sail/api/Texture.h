#pragma once

class Texture {
public:
	static Texture* Create(const std::string& filename);
	Texture() {}
	virtual ~Texture() {}

};