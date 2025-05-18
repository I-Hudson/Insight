#include "Resource/AnimationClip.h"
#include "Core/Asserts.h"

#include "Asset/AssetInfo.h"

namespace Insight
{
    namespace Runtime
    {
        //=======================================
        // AnimationClip
        //=======================================
        AnimationBoneTrack::AnimationBoneTrack()
        {
        }

        AnimationBoneTrack::AnimationBoneTrack(const u32 boneId, std::string name, std::vector<PositionKeyFrame> positions, std::vector<RotationKeyFrame> rotations, std::vector<ScaleKeyFrame> scales)
            : BoneId(boneId)
            , Name(std::move(name))
            , Positions(std::move(positions))
            , Rotations(std::move(rotations))
            , Scales(std::move(scales))
        { }

        const u32 AnimationBoneTrack::GetPositionKeyFrameIndex(const double time) const
        {
            // Return the index to the position to interpolate to.
            for (size_t posIdx = 0; posIdx < Positions.size() - 1; ++posIdx)
            {
                const PositionKeyFrame& positionKeyFrame = Positions[posIdx + 1];
                if (time < positionKeyFrame.TimeStamp)
                {
                    return (u32)posIdx;
                }
            }
            FAIL_ASSERT();
            return 0;
        }

        const u32 AnimationBoneTrack::GetRotationKeyFrameIndex(const double time) const
        {
            // Return the index to the rotation to interpolate to.
            for (size_t rotIdx = 0; rotIdx < Rotations.size() - 1; ++rotIdx)
            {
                const RotationKeyFrame& rotationKeyFrame = Rotations[rotIdx + 1];
                if (time < rotationKeyFrame.TimeStamp)
                {
                    return (u32)rotIdx;
                }
            }
            FAIL_ASSERT();
            return 0;
        }

        const u32 AnimationBoneTrack::GetScaleKeyFrameIndex(const double time) const
        {
            // Return the index to the scale to interpolate to.
            for (size_t scaleIdx = 0; scaleIdx < Scales.size() - 1; ++scaleIdx)
            {
                const ScaleKeyFrame& scaleKeyFrame = Scales[scaleIdx + 1];
                if (time < scaleKeyFrame.TimeStamp)
                {
                    return (u32)scaleIdx;
                }
            }
            FAIL_ASSERT();
            return 0;
        }

        //=======================================
        // AnimationClip
        //=======================================
        AnimationClip::AnimationClip(const AssetInfo* assetInfo, Ref<Skeleton> skeleton, std::string name)
            : m_assetInfo(std::move(assetInfo))
            , m_skeleton(std::move(skeleton))
            , m_name(std::move(name))
        { }
        AnimationClip::AnimationClip()
        { }
        AnimationClip::~AnimationClip()
        { }

        const AnimationBoneTrack* AnimationClip::GetBoneTrack(const std::string_view boneName) const
        {
            for (size_t boneIdx = 0; boneIdx < m_boneTracks.size(); ++boneIdx)
            {
                const AnimationBoneTrack& boneTrack = m_boneTracks[boneIdx];
                if (boneTrack.Name == boneName)
                {
                    return &boneTrack;
                }
            }
            return nullptr;
        }

        const AnimationBoneTrack* AnimationClip::GetBoneTrack(const u32 boneId) const
        {
            if (auto iter = m_boneIdToBoneTrack.find(boneId);
                iter != m_boneIdToBoneTrack.end())
            {
                return &m_boneTracks[iter->second];
            }

            for (size_t boneIdx = 0; boneIdx < m_boneTracks.size(); ++boneIdx)
            {
                const AnimationBoneTrack& boneTrack = m_boneTracks[boneIdx];
                if (boneTrack.BoneId == boneId)
                {
                    return &boneTrack;
                }
            }

            return nullptr;
        }

        double AnimationClip::GetDuration() const
        {
            return m_duration;
        }

        double AnimationClip::GetTickPerSecond() const
        {
            return m_ticksPerSecond;
        }

        void AnimationClip::AddBoneTrack(AnimationBoneTrack animationBoneTrack)
        {
            ASSERT(m_boneIdToBoneTrack.find(animationBoneTrack.BoneId) == m_boneIdToBoneTrack.end());
            m_boneIdToBoneTrack[animationBoneTrack.BoneId] = static_cast<u32>(m_boneTracks.size());
            m_boneTracks.push_back(animationBoneTrack);
        }
    }
}