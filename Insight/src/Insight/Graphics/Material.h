#pragma once

#include "Insight/Core/Core.h"
#include "../vendor/glm/glm/glm.hpp"

	namespace Render
	{
		class Shader;
		class Texture;
	}

// Single struct defining a dynamic uniform offset and index.
struct MaterialDynamicUniformInfo
{
	U32 Index;
	U32 DynamicOffset;
	MaterialDynamicUniformInfo()
	{ }

	MaterialDynamicUniformInfo(U32 const& index)
		: Index(index)
	{ }
};

// Struct for a single draw defining material data for the shader.
struct MaterialBlockData
{
	std::unordered_map<std::string, MaterialDynamicUniformInfo> DynamicBuffers;
	U128 UUID;
	bool InUse;

	MaterialBlockData()
		: InUse(false)
	{ }
};

struct MaterialUniformDynamicBlock
{
	void* DynamicBuffer;
	U64 DynamicBufferSize;
	U64 DynamicUniformAlign;
	U64 TopIndex;

	MaterialUniformDynamicBlock()
		: DynamicUniformAlign(0)
		, DynamicBuffer(nullptr)
		, DynamicBufferSize(0)
		, TopIndex(0)
	{ }
};

class IS_API Material : public Object
{
public:
	virtual ~Material() { }
	static SharedPtr<Material> Create();

	virtual void CreateDefault() = 0;

	virtual void Update() = 0;
	virtual void ResetUniformInfo() = 0;

	virtual void UploadUniform(const std::string& key, void* data, const U32& dataSize, MaterialBlockData& materialBlockData) = 0;
	virtual void UploadTexture(const std::string& key, WeakPtr<Render::Texture> texture) = 0;
	virtual void UploadTexture(const std::string& key, void* imageView, void* sampler, const U32& format) = 0;
};
