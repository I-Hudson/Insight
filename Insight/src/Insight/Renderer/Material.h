#pragma once

#include "Insight/Core/Core.h"
#include "../vendor/glm/glm/glm.hpp"

namespace Insight
{
	namespace Render
	{
		class Shader;
		class Texture;
	}
}

struct MaterialDynamicUniformInfo
{
	U32 Index;
	bool InUse;

	MaterialDynamicUniformInfo()
		: InUse(false)
	{ }

	MaterialDynamicUniformInfo(U32 const& index, bool const& inUse)
		: Index(index)
		, InUse(inUse)
	{ }
};

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
	std::vector<MaterialBlockData> DynamicBlocks;

	MaterialUniformDynamicBlock()
		: DynamicUniformAlign(0)
		, DynamicBuffer(nullptr)
		, DynamicBufferSize(0)
	{ }
};

class IS_API Material : public Insight::Object
{
public:
	virtual ~Material() { }
	static SharedPtr<Material> Create();

	virtual void CreateDefault() = 0;

	virtual void Update() = 0;

	virtual void UploadUniform(const std::string& key, void* data, const U32& dataSize, MaterialBlockData& materialBlockData) = 0;
	virtual void UploadTexture(const std::string& key, WeakPtr<Insight::Render::Texture> texture) = 0;
	virtual void UploadTexture(const std::string& key, void* imageView, void* sampler, const U32& format) = 0;
};
