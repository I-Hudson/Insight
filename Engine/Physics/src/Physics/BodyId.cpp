#include "Physics/BodyId.h"

namespace Insight::Physics
{
#ifdef IS_PHYSICS_JOLT

    BodyId::operator bool() const
    {
        return !JoltBodyId.IsInvalid();
    }

    bool BodyId::operator==(const BodyId& other) const
    {
        return JoltBodyId == other.JoltBodyId;
    }

    bool BodyId::operator!=(const BodyId& other) const
    {
        return !(*this == other);
    }
#endif
}