#include "ispch.h"

#include "Insight/Library/ShaderLibrary.h"

namespace Insight
{
	namespace Library
	{
		ShaderLibrary::ShaderLibrary()
			: Serializable(this, false, "ShaderLibrary.json")
		{
			SetInstancePtr(this);
		}

		ShaderLibrary::~ShaderLibrary()
		{
			ClearPtr();
		}

		void ShaderLibrary::Serialize(json& data, bool force)
		{
		}

		void ShaderLibrary::Deserialize(json data, bool force)
		{
		}
	}
}
