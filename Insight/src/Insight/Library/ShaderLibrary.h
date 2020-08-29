#pragma once

#include "Insight/Core.h"
#include "Insight/Library/Library.h"
#include "Insight/Serialization/Serializable.h"
#include "Insight/Renderer/Shader.h"

namespace Insight
{
	namespace Library
	{
		class ShaderLibrary : public Library<Render::Shader>
			, public TSingleton<ShaderLibrary>
			, public Serialization::Serializable
		{
		public:
			ShaderLibrary();
			virtual ~ShaderLibrary() override;

			virtual void Serialize(json& data, bool force = false) override;
			virtual void Deserialize(json data, bool force = false) override;
		};
	}
}