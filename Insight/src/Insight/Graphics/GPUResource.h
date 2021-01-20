#pragma once
#include "Core/Core.h"
#include "Core/NonCopyable.h"

	class IS_API GPUResource : public Object, public NonCopyable
	{
	public:
		DECLARE_ENUM_2(ResourceType, Texture, Buffer);

	protected:
		virtual void ReleaseGPUResource() = 0;

		U64 m_memoryUsage = 0;

	public:

        GPUResource()
        {
        }

		virtual ~GPUResource()
		{
#ifdef IS_DEBUG
			IS_CORE_ASSERT(m_memoryUsage == 0, "GPUResource has not been released.");
#endif
		}

		virtual ResourceType GetResourceType() const = 0;

		FORCE_INLINE U64 GetMemoryUsage() const { return m_memoryUsage; }

		virtual std::string GetName() const { return ""; }

		void ReleaseGPU()
		{
			if (m_memoryUsage != 0)
			{
				ReleaseGPUResource();
				m_memoryUsage = 0;
			}
		}
	};

	/// <summary>
/// Interface for GPU resources views. Shared base class for texture and buffer views.
/// </summary>
	class IS_API GPUResourceView
	{
	protected:
		explicit GPUResourceView()
		{
		}

	public:

		/// <summary>
		/// Gets the native pointer to the underlying view. It's a platform-specific handle.
		/// </summary>
		/// <returns>The pointer.</returns>
		virtual void* GetNativePtr() const = 0;
	};