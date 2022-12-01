#pragma once

#include "Core/Defines.h"
#include "Core/TypeAlias.h"

#include "Platform/Platform.h"

#include <vector>

namespace Insight
{
	namespace Core
	{
		// Primary template intentionally left empty
		template <typename Signature>
		class DelegateBase;

		template<typename Signature>
		class Function;

		template<typename Signature>
		using Action = DelegateBase<Signature>;

		template<typename... Args>
		using Delegate = DelegateBase<void(Args...)>;

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
			template<typename Class, ReturnType(Class::* Function)(Args...)>
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
			template<typename Class, ReturnType(Class::* Function)(Args...)>
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
		class DelegateBase<Return(Args...)>
		{
			using Signature = Return(*)(Args...);
			using stub_function = Return(*)(const void*, Args...);

		public:
			DelegateBase() = default;
			DelegateBase(const DelegateBase& other) = default;

			operator bool() const { return m_functions.size() > 0; }

			auto operator=(const DelegateBase& other)->DelegateBase& = default;

			auto operator()(Args... args) const->Return
			{
				if constexpr (std::is_void_v<Return>)
				{
					for (size_t i = 0; i < m_functions.size(); ++i)
					{
						if (m_functions.at(i))
						{
							m_functions.at(i)(std::forward<Args>(args)...);
						}
					}
					return void();
				}
				else
				{
					if (m_functions.size() > 0)
					{
						return m_functions.at(0)(std::forward<Args>(args)...);
					}
				}
			}
			template <typename... UArgs,
				typename = std::enable_if_t<std::is_invocable_v<Return(Args...), UArgs...>>>
			auto operator()(UArgs... args) const -> Return
			{
				if constexpr (std::is_void_v<Return>)
				{
					for (size_t i = 0; i < m_functions.size(); ++i)
					{
						if (m_functions.at(i))
						{
							m_functions.at(i)(std::forward<Args>(args)...);
						}
					}
					return void();
				}
				else
				{
					if (m_functions.size() > 0)
					{
						return m_functions.at(0)(std::forward<Args>(args)...);
					}
				}
			}

			// Free function bind.
			template <auto Function,
				typename = std::enable_if_t<std::is_invocable_r_v<Return, decltype(Function), Args...>>>
			void Bind()
			{
				Core::Function<Return(Args...)> f;
				f.Bind<Function>();
				BindNewFunction(f);
			}
			// Free function lambda bind.
			void Bind(Signature lambda)
			{
				Core::Function<Return(Args...)> f;
				f.Bind(lambda);
				BindNewFunction(f);
			}

			// Const member function bind.
			template <auto MemberFunction, typename Class,
				typename = std::enable_if_t<std::is_invocable_r_v<Return, decltype(MemberFunction), const Class*, Args...>>>
			void Bind(const Class* c)
			{
				Core::Function<Return(Args...)> f;
				f.Bind<Class, MemberFunction>(c);
				BindNewFunction(f);
			}

			// Non const member function bind.
			template <auto MemberFunction, typename Class,
				typename = std::enable_if_t<std::is_invocable_r_v<Return, decltype(MemberFunction), Class*, Args...>>>
			void Bind(Class* c)
			{
				Core::Function<Return(Args...)> f;
				f.Bind<Class, MemberFunction>(c);
				BindNewFunction(f);
			}

			void UnbindAll()
			{
				m_function.clear();
			}

			// Free function unbind.
			template <auto Function,
				typename = std::enable_if_t<std::is_invocable_r_v<Return, decltype(Function), Args...>>>
			void Unbind()
			{
				Core::Function<Return(Args...)> f;
				f.Bind<Function>();
				UnbindFunction(f);
			}

			// Const member function unbind.
			template <auto MemberFunction, typename Class,
				typename = std::enable_if_t<std::is_invocable_r_v<Return, decltype(MemberFunction), const Class*, Args...>>>
			void Unbind(const Class* c)
			{
				Core::Function<Return(Args...)> f;
				f.Bind<Class, MemberFunction>(c);
				UnbindFunction(f);
			}

			// Non const member function unbind.
			template <auto MemberFunction, typename Class,
				typename = std::enable_if_t<std::is_invocable_r_v<Return, decltype(MemberFunction), Class*, Args...>>>
			void Unbind(Class* c)
			{
				Core::Function<Return(Args...)> f;
				f.Bind<Class, MemberFunction>(c);
				UnbindFunction(f);
			}

		private:
			void BindNewFunction(Function<Return(Args...)> f)
			{
				if constexpr (std::is_void_v<Return>)
				{
					for (size_t i = 0; i < m_functions.size(); ++i)
					{
						if (f == m_functions.at(i))
						{
							return;
						}
					}
					m_functions.push_back(f);
				}
				else
				{
					if (m_functions.size() == 0)
					{
						m_functions.push_back(f);
					}
				}
			}

			void UnbindFunction(Function<Return(Args...)> f)
			{
				int indexToRemove = -1;
				for (size_t i = 0; i < m_functions.size(); ++i)
				{
					if (f == m_functions.at(i))
					{
						indexToRemove = (int)i;
						break;
					}
				}
				if (indexToRemove != -1)
				{
					m_functions.erase(m_functions.begin() + indexToRemove);
				}
			}

		private:
			//std::vector<IntPtr> m_callees;
			//std::vector<IntPtr> m_functions;
			std::vector<Function<Return(Args...)>> m_functions;
		};
	}
}