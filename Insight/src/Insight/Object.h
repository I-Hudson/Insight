#pragma once

#include "Insight/Templates/TPointers.h"
#include "UUID.h"

namespace Insight
{
	class Object;
	typedef std::function<void(Object*)> ObjectCallback;

	class Object : public Insight::UUID
	{
	public:
		Object();
		virtual ~Object();

		bool Equals(const Object* pObject) const;
		virtual bool Equals(const Object& pObject) const;

		std::string GetType();

		void RegisterOnDestroyCallback(void* callerClass, ObjectCallback callback);
		void UnregisterOnDestroyCallback(void* callerClass);

		inline void AddRef();
		inline void Release();

		void CreateHash(std::string const& str);

		void operator delete(void* ptr);

	private:
		long m_refCount;
		std::string m_typeName;
		uint64_t m_hash;
		std::unordered_map<void*, ObjectCallback> m_onDestroyCallbacks;
	};
}
#define REG_ON_DESTROY(objectPtr, callerClass, func) Object::RegisterOnDestroyCallback(std::bind(&func, objectPtr, std::placeholders::_1), callerClass)
#define UNREG_ON_DESTROY() Object::UnregisterOnDestroyCallback(this)
