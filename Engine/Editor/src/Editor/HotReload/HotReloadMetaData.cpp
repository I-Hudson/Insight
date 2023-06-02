#include "Editor/HotReload/HotReloadMetaData.h"

namespace Insight::Editor
{
    HotReloadMetaData::HotReloadMetaData()
    {
    }

    HotReloadMetaData::HotReloadMetaData(HotReloadMetaData&& other) NO_EXPECT
    {
        (*this) = std::move(other);
    }

    HotReloadMetaData::~HotReloadMetaData()
    {
    }

    HotReloadMetaData& HotReloadMetaData::operator=(HotReloadMetaData&& other) NO_EXPECT
    {
        EditorWindowClasses = std::move(other.EditorWindowClasses);
        ComponentsClasses = std::move(other.ComponentsClasses);

        other.EditorWindowClasses.clear();
        other.ComponentsClasses.clear();

        return *this;
    }
}