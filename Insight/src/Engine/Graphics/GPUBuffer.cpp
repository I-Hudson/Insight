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

		ASSERT(false && "[GPUBuffer::New] API not supported.");
		return nullptr;
	}

	GPUBuffer::GPUBuffer()
		: m_mappedData(nullptr)
		, m_mustBeMapped(false)
	{ }

	GPUBuffer::~GPUBuffer()
	{ }

	void GPUBuffer::SetData(void const* data, const U64& size)
	{
		ASSERT(data != nullptr && size > 0 && size <= GetSize());

		void* mapped = nullptr;
		if (m_mustBeMapped)
		{
			mapped = Map();
			if (!IsMapped())
			{
				return;
			}
		}
		Upload(mapped, data, size);
		//Platform::MemCopy(mapped, data, size);
		if (m_mustBeMapped)
		{
			UnMap();
		}
	}

	void GPUBuffer::GetData(std::vector<Byte>& data)
	{
		void* mapped = nullptr;
		if (m_mustBeMapped)
		{
			mapped = Map();
			if (!mapped)
			{
				return;
			}
		}
		data.resize(GetSize());
		Download(data, mapped);
		if (m_mustBeMapped)
		{
			UnMap();
		}
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