#include "Resource/AnimationClip.h"

namespace Insight
{
    namespace Runtime
    {
        AnimationBoneTrack::AnimationBoneTrack()
        {
        }

        AnimationBoneTrack::AnimationBoneTrack(const u32 boneId, const std::string name, const std::vector<PositionKeyFrame> positions, const std::vector<RotationKeyFrame> rotations, const std::vector<ScaleKeyFrame> scales)
            : BoneId(boneId)
            , Name(std::move(name))
            , Positions(std::move(positions))
            , Rotations(std::move(rotations))
            , Scales(std::move(scales))
        { }
    }
}