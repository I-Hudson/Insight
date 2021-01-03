#pragma once
#include "Insight/Core/Core.h"
#include <mutex>

namespace Insight
{
	template<typename T>
	class TThreadSafe
	{
	public:
		//Arithmetic Operators
		/* @brief Add operator (Read only) */
		T operator +(const T& b);
		/* @brief Add/equal operator (Read only) */
		T& operator +=(const T& b);

		/* @brief Minus operator (Read only) */
		T operator -(const T& b);
		/* @brief Minus/equal operator (Read only) */
		T& operator -=(const T& b);

		/* @brief Multiplies operator (Read Only); */
		T operator *(const T& b);
		/* @brief Multiplies/equal operator (Read Only); */
		T& operator *=(const T& b);

		/* @brief Divide operator (Read only) */
		T operator /(const T& b);
		/* @brief Divide/equal operator (Read only) */
		T& operator /=(const T& b);

		/* @brief Modulus operator (Read only) */
		T& operator %(const T& b);
		/* @brief Increment operator (Read/Write) */
		T& operator ++();
		/* @brief Decrement operator (Read/Write) */
		T& operator --();

		//Assignment Operators
		/* @brief Assignment crement operator (Read/Write) */
		T& operator=(const T& b);

		T& GetValue();

		bool Lock();
		void Unlock();

	private:
		std::mutex m_mutex;
		T m_var;
	};

	template<typename T>
	inline T TThreadSafe<T>::operator+(const T& b)
	{
		Lock();
		T t = m_var + b;
		Unlock();
		return t;
	}
	template<typename T>
	inline T& TThreadSafe<T>::operator+=(const T& b)
	{
		Lock();
		m_var = m_var + b;
		Unlock();
		return m_var;
	}

	template<typename T>
	inline T TThreadSafe<T>::operator-(const T& b)
	{
		Lock();
		T t = m_var - b;
		Unlock();
		return t;
	}
	template<typename T>
	inline T& TThreadSafe<T>::operator-=(const T& b)
	{
		Lock();
		m_var = m_var - b;
		Unlock();
		return m_var;
	}

	template<typename T>
	inline T TThreadSafe<T>::operator*(const T& b)
	{
		Lock();
		T t = m_var * b;
		Unlock();
		return t;
	}
	template<typename T>
	inline T& TThreadSafe<T>::operator*=(const T& b)
	{
		Lock();
		m_var = m_var * b;
		Unlock();
		return m_var;
	}

	template<typename T>
	inline T TThreadSafe<T>::operator/(const T& b)
	{
		Lock();
		T t = m_var / b;
		Unlock();
		return t;
	}
	template<typename T>
	inline T& TThreadSafe<T>::operator/=(const T& b)
	{
		Lock();
		m_var = m_var / b;
		Unlock();
		return m_var;
	}

	template<typename T>
	inline T& TThreadSafe<T>::operator%(const T& b)
	{
		Lock();
		T t = m_var % b;
		Unlock();
		return m_var;
	}
	template<typename T>
	inline T& TThreadSafe<T>::operator++()
	{
		Lock();
		++m_var;
		Unlock();
		return m_var;
	}
	template<typename T>
	inline T& TThreadSafe<T>::operator--()
	{
		Lock();
		--m_var;
		Unlock();
		return m_var;
	}

	template<typename T>
	inline T& TThreadSafe<T>::operator=(const T& b)
	{
		Lock();
		m_var = b;
		Unlock();
		return m_var;
	}

	template<typename T>
	inline T& TThreadSafe<T>::GetValue()
	{
		return m_var;
	}

	template<typename T>
	inline bool TThreadSafe<T>::Lock()
	{
		m_mutex.lock();
		return true;
	}

	template<typename T>
	inline void TThreadSafe<T>::Unlock()
	{
		m_mutex.unlock();
	}
}