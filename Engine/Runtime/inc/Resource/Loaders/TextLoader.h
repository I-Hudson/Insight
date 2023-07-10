#pragma once

#include "Resource/Loaders/IResourceLoader.h"

namespace Insight
{
	namespace Runtime
	{
		class TextLoader : public IResourceLoader
		{
		public:
			TextLoader();
			virtual ~TextLoader() override;

			virtual bool Load(IResource* resource) const override;
		};
	}
}