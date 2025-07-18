#include "Editor/HotReload/HotReloadMetaData.h"

namespace Insight::Editor
{
    HotReloadMetaData::HotReloadMetaData()
    {
    }

    HotReloadMetaData::HotReloadMetaData(HotReloadMetaData&& other)
    {
        (*this) = std::move(other);
    }

    HotReloadMetaData::~HotReloadMetaData()
    {
    }

    HotReloadMetaData& HotReloadMetaData::operator=(HotReloadMetaData&& other)
    {
        EditorWindowNames = std::move(other.EditorWindowNames);
        ComponentNames = std::move(other.ComponentNames);

        EditorWindowTypeInfos = std::move(other.EditorWindowTypeInfos);
        ComponentTypeInfos = std::move(other.ComponentTypeInfos);


        other.EditorWindowNames.clear();
        other.EditorWindowNames.clear();

        other.EditorWindowTypeInfos.clear();
        other.ComponentTypeInfos.clear();

        return *this;
    }
}