#pragma once

#include "Resource/Loaders/IResourceLoader.h"

namespace Insight
{

	namespace Runtime
	{
		class TextureLoader : public IResourceLoader
		{
		public:
			TextureLoader();
			virtual ~TextureLoader() override;

			virtual bool Load(IResource* resource) const override;
		};
	}
}