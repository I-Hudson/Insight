#pragma once

#include "Engine/Core/Core.h"
#include "../vendor/glm/glm/glm.hpp"
#include "Texture.h"


// Single struct defining a dynamic uniform offset and index.
struct MaterialDynamicUniformInfo
{
	u32 Index;
	u32 DynamicOffset;
	MaterialDynamicUniformInfo()
	{ }

	MaterialDynamicUniformInfo(u32 const& index)
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
	u64 DynamicBufferSize;
	u64 DynamicUniformAlign;
	u64 TopIndex;

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
	Material();
	virtual ~Material() { }
	static Material* New();

	virtual void CreateDefault() = 0;

	virtual void Update() = 0;
	virtual void ResetUniformInfo() = 0;

	virtual void UploadUniform(const std::string& key, void* data, const u32& dataSize, MaterialBlockData& materialBlockData) = 0;
	virtual void UploadTexture(const std::string& key, Render::Texture* texture) = 0;
	virtual void UploadTexture(const std::string& key, void* imageView, void* sampler, const u32& format) = 0;
};
