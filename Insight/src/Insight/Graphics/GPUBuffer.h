#pragma once

#include "GPUResource.h"
#include <glm/glm.hpp>

struct Vertex
{
	glm::vec3 Position;
	glm::vec3 Normal;
	glm::vec4 Colour;
	glm::vec2 UV1;
};

class IS_API GPUBuffer : public GPUResource
{
public:
	static SharedPtr<GPUBuffer> Create();

	virtual ~GPUBuffer() = 0;

protected:


	GPUBuffer() { }
};