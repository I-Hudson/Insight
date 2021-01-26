#pragma once
#include "Engine/Core/Common.h"

class Asset
{
	DECLARE_ENUM_8(AssetSate, Loaded, Loading, Unloaded, Unloading, Missing, Croupt, NonLoaded)

private:
	AssetSate m_state;
};