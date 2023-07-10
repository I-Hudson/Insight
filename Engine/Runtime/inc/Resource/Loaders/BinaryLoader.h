#pragma once

#include "Resource/Loaders/IResourceLoader.h"

namespace Insight
{
	namespace Runtime
	{
		class BinaryLoader : public IResourceLoader
		{
		public:
			BinaryLoader();
			virtual ~BinaryLoader() override;

			virtual bool Load(IResource* resource) const override;
		};
	}
}