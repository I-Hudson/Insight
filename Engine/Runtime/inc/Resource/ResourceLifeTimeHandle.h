#pragma once

namespace Insight
{
	namespace Runtime
	{
		class IResource;

		/**
			@brief Resource life time handle.
			Store a non owning pointer to a resource and invalidate it when the resource is no longer loaded.
		*/
		template<typename ResourceType>
		class ResourceLFHandle
		{
			static_assert(std::is_base_of_v<IResource, ResourceType>, "!");
			using Class = ResourceLFHandle<ResourceType>;
		public:
			ResourceLFHandle() = default;
			ResourceLFHandle(const ResourceLFHandle& other) { UnbindUnLoad(); m_resourcePtr = other.m_resourcePtr; BindUnload(); }
			ResourceLFHandle(ResourceLFHandle&& other) { UnbindUnLoad(); m_resourcePtr = other.m_resourcePtr; BindUnload(); }
			~ResourceLFHandle() { UnbindUnLoad(); m_resourcePtr = nullptr; }

			operator bool() const { return m_resourcePtr != nullptr; }
			operator ResourceType* () const { return static_cast<ResourceType*>(m_resourcePtr); }

			bool operator==(const ResourceLFHandle& other) const { return m_resourcePtr == other.m_resourcePtr; }
			bool operator!=(const ResourceLFHandle& other) const { return !((*this) == other); }

			ResourceType* operator->() const { return static_cast<ResourceType*>(m_resourcePtr); }

			ResourceLFHandle& operator=(const ResourceLFHandle& other) { UnbindUnLoad(); m_resourcePtr = other.m_resourcePtr; BindUnload(); }
			ResourceLFHandle& operator=(ResourceLFHandle&& other) { UnbindUnLoad(); m_resourcePtr = other.m_resourcePtr; BindUnload(); }

			ResourceLFHandle& operator=(std::nullptr_t) { UnbindUnLoad(); m_resourcePtr = nullptr; return *this; }
			ResourceLFHandle& operator=(ResourceType* other) { UnbindUnLoad(); m_resourcePtr = other; BindUnload(); return *this; }
			ResourceLFHandle& operator=(const ResourceType* other) { UnbindUnLoad(); m_resourcePtr = other; BindUnload(); return *this; }

		private:
			void BindUnload()
			{
				if (m_resourcePtr)
				{
					m_resourcePtr->OnUnloaded.Bind<&Class::OnUnload>(this);
				}
			}
			void UnbindUnLoad()
			{
				if (m_resourcePtr)
				{
					m_resourcePtr->OnUnloaded.Unbind<&Class::OnUnload>(this);
				}
			}
			void OnUnload(IResource* resource)
			{
				UnbindUnLoad();
				m_resourcePtr = nullptr;
			}

		private:
			IResource* m_resourcePtr = nullptr;
		};
	}
}