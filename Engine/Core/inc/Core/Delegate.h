#pragma once

#include "Core/Defines.h"

#include <vector>

namespace Insight
{
	namespace Core
	{
		// Primary template intentionally left empty
		template <typename Signature>
		class Delegate;

		template<typename Signature>
		class Function;

		/// <summary>
		/// Template Function class to store a callback action.
		/// </summary>
		/// <typeparam name="ReturnType"></typeparam>
		/// <typeparam name="...Args"></typeparam>
		template<typename ReturnType, typename... Args>
		class Function<ReturnType(Args ...)>
		{
			using Signature = ReturnType(*)(Args...);
			using StubSignature = ReturnType(*)(const void*, Args...);

		public:
			Function()
			{
				m_callee = nullptr;
				m_function = nullptr;
			}
			Function(Signature method)
			{
				m_callee = (void*)method;
				m_function = &StaticFreeFunctionStub;
			}

			operator bool() const { return m_function != nullptr; }

			// Bind a static/free function function.
			template<ReturnType(*Function)(Args...)>
			void Bind()
			{
				m_callee = nullptr;
				m_function = &StaticFunctionStub<Function>;
			}
			// Bind a lambda.
			void Bind(Signature function)
			{
				m_callee = (void*)function;
				m_function = &StaticFreeFunctionStub;
			}
			// Bind a non const member function.
			template<typename Class, ReturnType(Class::*Function)(Args...)>
			void Bind(Class* callee)
			{
				m_callee = callee;
				m_function = &StaticMemberFunctionStub<Class, Function>;
			}
			// Bind a const member function.
			template<typename Class, ReturnType(Class::* Function)(Args...) const>
			void Bind(const Class* callee)
			{
				m_callee = callee;
				m_function = &StaticMemberFunctionStub<Class, Function>;
			}

			void Unbind()
			{
				m_callee = nullptr;
				m_function = nullptr;
			}

			bool IsBinded() const
			{
				return m_function != nullptr;
			}

			ReturnType operator()(Args... args) const
			{
				if (m_function != nullptr)
				{
					return (*m_function)(m_callee, std::forward<Args>(args)...);
				}
			}

			template <typename... UArgs,
				typename = std::enable_if_t<std::is_invocable_v<ReturnType(Args...), UArgs...>>>
			ReturnType operator()(UArgs... args) const
			{
				if (m_stub != nullptr)
				{
					return (*m_function)(m_instance, std::forward<Args>(args)...);
				}
			}

			FORCE_INLINE bool operator==(const Function& other) const
			{
				return m_function == other.m_function && m_callee == other.m_callee;
			}

			FORCE_INLINE bool operator!=(const Function& other) const
			{
				return !(*this == other);
			}

		private:
			// Free function, no ownering class/struct.
			template<typename ReturnType(*Function)(Args...)>
			static ReturnType StaticFunctionStub(const void* callee, Args... args)
			{
				return (Function)(std::forward<Args>(args)...);
			}

			// Lambda function.
			static ReturnType StaticFreeFunctionStub(const void* callee, Args... args)
			{
				return reinterpret_cast<Signature>(callee)(std::forward<Args>(args)...);
			}

			// Member function.
			template<typename Class, ReturnType(Class::*Function)(Args...)>
			static ReturnType StaticMemberFunctionStub(const void* callee, Args... args)
			{
				return (reinterpret_cast<Class*>(const_cast<void*>(callee))->*Function)(std::forward<Args>(args)...);
			}

			// Const member function.
			template<typename Class, ReturnType(Class::* Function)(Args...) const>
			static ReturnType StaticMemberFunctionStub(const void* callee, Args... args)
			{
				return (reinterpret_cast<const Class*>(callee)->*Function)(std::forward<Args>(args)...);
			}

		private:
			const void* m_callee = nullptr;
			StubSignature m_function = nullptr;
		};

		template<typename Return, typename... Args>
		class Delegate<Return(Args...)>
		{
			using Signature = Return(*)(Args...);
			using stub_function = Return(*)(const void*, Args...);

		public:
			Delegate() = default;
			Delegate(const Delegate& other) = default;

			operator bool() const { return m_function; }

			auto operator=(const Delegate& other) ->Delegate& = default;

			auto operator()(Args&&... args) const->Return
			{
				return m_function(std::forward<Args>(args)...);
			}
			template <typename... UArgs,
				typename = std::enable_if_t<std::is_invocable_v<Return(Args...), UArgs...>>>
			auto operator()(UArgs&&... args) const -> Return
			{
				return m_function(std::forward<Args>(args)...);
			}

			// Free function bind.
			template <auto Function,
				typename = std::enable_if_t<std::is_invocable_r_v<Return, decltype(Function), Args...>>>
			void Bind()
			{
				m_function.Bind<Function>();
			}
			// Free function lambda bind.
			void Bind(Signature lambda)
			{
				m_function.Bind(lambda);
			}

			// Const member function bind.
			template <auto MemberFunction, typename Class,
				typename = std::enable_if_t<std::is_invocable_r_v<Return, decltype(MemberFunction), const Class*, Args...>>>
			void Bind(const Class* c)
			{
				m_function.Bind<Class, MemberFunction>(c);
			}

			// Non const member function bind.
			template <auto MemberFunction, typename Class,
				typename = std::enable_if_t<std::is_invocable_r_v<Return, decltype(MemberFunction), Class*, Args...>>>
			void Bind(Class* c)
			{
				m_function.Bind<Class, MemberFunction>(c);
			}

			void UnbindAll()
			{
				m_function.Unbind();
			}

			// Free function unbind.
			template <auto Function,
				typename = std::enable_if_t<std::is_invocable_r_v<Return, decltype(Function), Args...>>>
			void Unbind()
			{
				m_function.Unbind();
			}

			// Const member function unbind.
			template <auto MemberFunction, typename Class,
				typename = std::enable_if_t<std::is_invocable_r_v<Return, decltype(MemberFunction), const Class*, Args...>>>
			void Unbind(const Class* c)
			{
				m_function.Unbind();
			}

			// Non const member function unbind.
			template <auto MemberFunction, typename Class,
				typename = std::enable_if_t<std::is_invocable_r_v<Return, decltype(MemberFunction), Class*, Args...>>>
			void Unbind(Class* c)
			{
				m_function.Unbind();
			}

		private:
			//void BindNewFunction(const void* callee, stub_function& function)
			//{
			//	m_instances.push_back(callee);
			//	m_stubs.push_back(function);
			//}

			//void UnbindFunction(const void* callee, stub_function& function)
			//{
			//	for (size_t i = 0; i < m_instances.size(); ++i)
			//	{
			//		if (m_stubs.at(i) == function
			//			&& m_instances.at(i) == callee)
			//		{
			//			auto itrInstance = m_instances.begin() + i;
			//			auto itrStub = m_stubs.begin() + i;

			//			m_instances.erase(itrInstance);
			//			m_stubs.erase(itrStub);

			//			break;
			//		}
			//	}
			//}

		private:
			Function<Return(Args...)> m_function;
		};
	}
}