#pragma once
#include "Engine/Core/Common.h"
#include "Engine/Core/NonCopyable.h"

	class IS_API GPUResource : public Object, public NonCopyable
	{
	public:
		DECLARE_ENUM_2(ResourceType, Texture, Buffer);

	protected:
		virtual void ReleaseGPUResource() = 0;
		virtual bool CreateGPUResource() = 0;
		U64 m_memoryUsage = 0;

	public:

        GPUResource()
        { }

		virtual ~GPUResource()
		{
#ifdef IS_DEBUG
			IS_CORE_ASSERT(m_memoryUsage == 0, "GPUResource has not been released.");
#endif
		}

		virtual ResourceType GetResourceType() const = 0;

		FORCE_INLINE U64 GetMemoryUsage() const { return m_memoryUsage; }

		void ReleaseGPU()
		{
			if (m_memoryUsage != 0)
			{
				ReleaseGPUResource();
				m_memoryUsage = 0;
			}
		}
	};