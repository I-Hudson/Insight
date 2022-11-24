#pragma once

#include "Core/Defines.h"
#include <execution>

namespace Insight
{
	namespace Core
	{
		// Primary template intentionally left empty
		template <typename Signature>
		class Delegate;

		class BadDelegateCall : public std::exception 
		{
		public:
			explicit BadDelegateCall(char const* const _Message) noexcept
				: std::exception(_Message)
			{ }
		};

		template<typename Return, typename... Args>
		class Delegate<Return(Args...)>
		{
			using Signature = Return(*)(Args...);

		public:
			Delegate() = default;
			Delegate(const Delegate& other) = default;

			auto operator=(const Delegate& other) ->Delegate& = default;

			auto operator()(Args&&... args) const->Return
			{
				if (m_stub != nullptr)
				{
					return (*m_stub)(m_instance, std::forward<Args>(args)...);
				}
			}

			template <typename... UArgs,
				typename = std::enable_if_t<std::is_invocable_v<Return(Args...), UArgs...>>>
			auto operator()(UArgs&&... args) const->Return
			{
				if (m_stub != nullptr)
				{
					return (*m_stub)(m_instance, std::forward<Args>(args)...);
				}
			}

			// Free function bind.
			template <Return(*Function)(Args...)>
			auto bind() -> void
			{
				m_instance = nullptr;
				m_stub = static_cast<stub_function>([](const void* callee, Args...args) -> Return
					{
						return (*Function)(std::forward<Args>(args)...);
					});
			}
			// Free function lambda bind.
			auto bind(Signature lambda) -> void
			{
				m_instance = (void*)lambda;
				m_stub = static_cast<stub_function>([](const void* callee, Args...args) -> Return
					{
						return reinterpret_cast<Signature>(callee)(std::forward<Args>(args)...);
					});
			}

			// Const member function bind.
			template <typename Class, Return(Class::*MemberFunction)(Args...) const>
			auto bind(const Class* c) -> void
			{
				m_instance = c; // store the class pointer
				m_stub = static_cast<stub_function>([](const void* p, Args...args) -> Return
					{
						const auto* cls = static_cast<const Class*>(p);
						return (cls->*MemberFunction)(std::forward<Args>(args)...);
					});
			}

			// Non const member function bind.
			template <typename Class, Return(Class::*MemberFunction)(Args...)>
			auto bind(Class* c) -> void
			{
				m_instance = c; // store the class pointer
				m_stub = static_cast<stub_function>([](const void* p, Args...args) -> Return
					{
						// Safe, because we know the pointer was bound to a non-const instance
						auto* cls = const_cast<Class*>(static_cast<const Class*>(p));
						return (cls->*MemberFunction)(std::forward<Args>(args)...);
					});
			}

			void Unbind()
			{
				m_instance = nullptr;
				m_stub = nullptr;
			}

		private:
			using stub_function = Return(*)(const void*, Args...);

			const void* m_instance = nullptr;	 ///< A pointer to the instance (if it exists)
			stub_function m_stub = nullptr;   ///< A pointer to the function to invoke
		};
	}
}