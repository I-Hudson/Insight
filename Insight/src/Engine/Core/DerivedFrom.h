#pragma once

#include "Engine/Platform/Platform.h"

template<typename T>
class DerivedFrom
{
public:
	DerivedFrom()
	{
		ASSERT(typeid(this).name != typeid(T).name())
	}
};