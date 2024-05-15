#pragma once

#include "Core/RefCount.h"

#include "Maths/Vector3.h"
#include "Maths/Quaternion.h"

namespace Insight
{
    namespace Runtime
    {
        class ModelImporter;

        /// @brief Store all the key frame data for a specific bone
        struct AnimationBoneTrack
        {
            struct PositionKeyFrame
            {
                Maths::Vector3 Position;
                double TimeStamp;
            };
            struct RotationKeyFrame
            {
                Maths::Quaternion Rotation;
                double TimeStamp;
            };
            struct ScaleKeyFrame
            {
                Maths::Vector3 Scale;
                double TimeStamp;
            };

            AnimationBoneTrack();
            AnimationBoneTrack(const u32 boneId, const std::string name, const std::vector<PositionKeyFrame> positions, const std::vector<RotationKeyFrame> rotations, const std::vector<ScaleKeyFrame> scales);

            std::vector<PositionKeyFrame> Positions;
            std::vector<RotationKeyFrame> Rotations;
            std::vector<ScaleKeyFrame> Scales;

            std::string Name;
            u32 BoneId;
        };

        class AnimationClip : public Core::RefCount
        {
        public:

        private:
            std::vector<AnimationBoneTrack> m_boneTracks;

            float m_duration = 0.0f;
            u32 m_ticksPerSecond = 0;

            friend ModelImporter;
        };
    }
}