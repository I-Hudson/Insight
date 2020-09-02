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

		void ShaderLibrary::Serialize(tinyxml2::XMLNode* out, tinyxml2::XMLDocument* doc, bool force)
		{
		}

		void ShaderLibrary::Deserialize(tinyxml2::XMLNode* data, bool force)
		{
		}
	}
}
