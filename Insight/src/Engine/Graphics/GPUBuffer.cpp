#include "ispch.h"
#include "Engine/Graphics/GPUBuffer.h"
#include "Engine/Graphics/GPUDevice.h"
#include "Engine/Module/GraphicsModule.h"
#include "Engine/GraphicsAPI/Vulkan/GPUBufferVulkan.h"

namespace Insight::Graphics
{
	GPUBuffer* GPUBuffer::New()
	{
		switch (Module::GraphicsModule::Instance()->GetAPI())
		{
			case GraphicsRendererAPI::Vulkan: return ::New<GraphicsAPI::Vulkan::GPUBufferVulkan>();
		}

		ASSERT(false && "[GPUCommandBuffer::New] API not supported.");
		return nullptr;
	}

	GPUBuffer::GPUBuffer()
	{
	}

	GPUBuffer::~GPUBuffer()
	{
	}

	void GPUBuffer::SetData(void const* data, const U64& size)
	{
		ASSERT(data != nullptr && size > 0 && size <= GetSize());

		void* mapped = Map(GPUResourceMapMode::Write);
		if (!IsMapped())
		{
			return;
		}
		Platform::MemCopy(mapped, data, size);
		UnMap();
	}

	void GPUBuffer::GetData(std::vector<Byte>& data)
	{
		void* mapped = Map(GPUResourceMapMode::Read);
		if (!mapped)
		{
			return;
		}
		data.resize(GetSize());
		Platform::MemCopy(data.data(), mapped, GetSize());
		UnMap();
	}

	void GPUBuffer::Resize(U32 newSize)
	{
		ASSERT(IsAllocated());

		auto desc = m_desc;
		desc.Size = newSize;
		desc.InitData = nullptr;

		Init(desc);
	}
}