#pragma once

#include "Engine/Core/Common.h"
#include "SparseSet.h"

namespace Insight::Collections
{

	class TSystemCollectionPool
	{
		SparseSet* Poo;
	};

	class TSystemCollection
	{
	private:
		std::vector<TSystemCollectionPool*> m_collection;
	};
}