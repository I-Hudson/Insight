#include "Graphics/SimplygonWrapper.h"

#include "Core/Logger.h"

#include <SimplygonLoader.h>

namespace Insight
{
    ::Simplygon::ISimplygon* SimplygonWrapper::s_instance = nullptr;

    bool SimplygonWrapper::Initialise()
    {
        if (s_instance)
        {
            return true;
        }

        ::Simplygon::EErrorCodes initValue = ::Simplygon::Initialize(&s_instance);
        if (initValue != Simplygon::EErrorCodes::NoError)
        {
            s_instance = nullptr;
            IS_CORE_ERROR("[SimplygonWrapper::Initialise] Simplygon error '{}'.", Simplygon::GetError(initValue));
            return false;
        }
        return true;
    }

    void SimplygonWrapper::Shutdown()
    {
        if (s_instance)
        {
            Simplygon::Deinitialize(s_instance);
            s_instance = nullptr;
        }
    }
}