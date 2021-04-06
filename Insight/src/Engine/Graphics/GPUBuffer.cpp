#include "ispch.h"
#include "Engine/Graphics/GPUBuffer.h"
#include "Engine/Graphics/GPUDevice.h"

GPUBuffer* GPUBuffer::New()
{
	return nullptr;
}

GPUBuffer::GPUBuffer()
{
}

GPUBuffer::~GPUBuffer()
{
}

bool GPUBuffer::Init(const GPUBufferDescription& desc)
{
	ASSERT(desc.Size > 0 && desc.Stride > 0);

	ReleaseGPU();

	m_desc = desc;
	if (OnInit())
	{
		IS_WARN("[GPUBuffer::Init] Can not create GPU resources.");
		return false;
	}
	return true;
}

void GPUBuffer::SetData(void* data, const U64& size)
{
	ASSERT(data != nullptr && size > 0 && size <= GetSize());

	void* mapped = Map(GPUResourceMapMode::Write);
	if (!mapped)
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
	Platform::MemCopy(data.data(), mapped, GetSize());
	UnMap();
}

bool GPUBuffer::Resize(U32 newSize)
{
	ASSERT(IsAllocated());

	auto desc = m_desc;
	desc.Size = newSize;
	desc.InitData = nullptr;

	return Init(desc);
}
