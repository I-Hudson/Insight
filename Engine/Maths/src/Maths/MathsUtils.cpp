#include "Maths/MathsUtils.h"

namespace Insight
{
	namespace Maths
	{
		constexpr MathsLibrary EnabledMathsLibrary()
		{
#ifdef IS_MATHS_DIRECTX_MATHS
			return MathsLibrary::DirectXMath;
#else
			return MathsLibrary::Insight;
#endif
		}
	}
}