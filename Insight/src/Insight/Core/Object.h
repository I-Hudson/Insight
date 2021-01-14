#pragma once

#if !defined(IS_STANDARD_POINTER)
#include "Insight/Templates/TPointers.h"
#endif
#include "InsightAlias.h"
#include "Log.h"
#include "UUID.h"
#include "Type.h"

namespace Insight
{
	class Object;

	typedef std::function<void(Object*)> ObjectCallback;

	class Object : public Insight::UUID
	{
	public:
		Object();
		virtual ~Object();

		virtual void OnCreate() { }
		virtual void OnDestroy() { }
		virtual bool IsValid() { return m_refCount > 0; }

		template<typename T, typename... Args>
		static SharedPtr<T> CreateObject(Args&& ...);

		bool Equals(const Object* pObject) const;
		virtual bool Equals(const Object& pObject) const;

		const Type& GetType() const { return m_type; }

		void RegisterOnDestroyCallback(void* callerClass, ObjectCallback callback);
		void UnregisterOnDestroyCallback(void* callerClass);

		inline void AddRef();
		inline void Release();

	private:
		uint64_t m_refCount;
		Type m_type;
		std::unordered_map<void*, ObjectCallback> m_onDestroyCallbacks;
	};

	template<typename T, typename ...Args>
	inline SharedPtr<T> Object::CreateObject(Args&&... args)
	{
		IS_CORE_STATIC_ASSERT((std::is_base_of<Object, T>::value), "'T' does not inherit from 'Object'.");

		SharedPtr<T> ptr = CreateSharedPtrNoExpect<T>(std::forward<Args>(args)...);

		Object* objPtr = static_cast<Object*>(ptr.get());
		objPtr->m_type.SetType<T>();

		return ptr;
	}
}
#define REG_ON_DESTROY(objectPtr, callerClass, func) Object::RegisterOnDestroyCallback(std::bind(&func, objectPtr, std::placeholders::_1), callerClass)
#define UNREG_ON_DESTROY() Object::UnregisterOnDestroyCallback(this)
