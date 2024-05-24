#pragma once

#include "Core/RefCount.h"

#include "Maths/Vector3.h"
#include "Maths/Quaternion.h"

#include "Core/ReferencePtr.h"
#include "Resource/Skeleton.h"

#include <map>

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
                PositionKeyFrame(Maths::Vector3 position, double timeStamp)
                    : Position(std::move(position)), TimeStamp(timeStamp)
                { }

                Maths::Vector3 Position;
                double TimeStamp = 0.0;
            };
            struct RotationKeyFrame
            {
                RotationKeyFrame(Maths::Quaternion rotation, double timeStamp)
                    : Rotation(std::move(rotation)), TimeStamp(timeStamp)
                { }

                Maths::Quaternion Rotation;
                double TimeStamp = 0.0;
            };
            struct ScaleKeyFrame
            {
                ScaleKeyFrame(Maths::Vector3 scale, double timeStamp)
                    : Scale(std::move(scale)), TimeStamp(timeStamp)
                { }

                Maths::Vector3 Scale;
                double TimeStamp = 0.0;
            };

            AnimationBoneTrack();
            AnimationBoneTrack(const u32 boneId, std::string name, std::vector<PositionKeyFrame> positions, std::vector<RotationKeyFrame> rotations, std::vector<ScaleKeyFrame> scales);

            const u32 GetPositionKeyFrameIndex(const double time) const;
            const u32 GetRotationKeyFrameIndex(const double time) const;
            const u32 GetScaleKeyFrameIndex(const double time) const;

            std::vector<PositionKeyFrame> Positions;
            std::vector<RotationKeyFrame> Rotations;
            std::vector<ScaleKeyFrame> Scales;

            std::string Name;
            u32 BoneId = -1;
        };

#if ANIMATION_NODE_TRANSFORMS
        struct AnimationNode
        {
            std::string Name;
            Maths::Matrix4 Transform;
            int ChildrenCount;
            std::vector<AnimationNode> Children;
        };
#endif
        class AnimationClip : public Core::RefCount
        {
        public:
            AnimationClip();
            virtual ~AnimationClip() override;

            const AnimationBoneTrack* GetBoneTrack(const std::string_view boneName) const;
            const AnimationBoneTrack* GetBoneTrack(const u32 boneId) const;
            double GetDuration() const;
            double GetTickPerSecond() const;

#if ANIMATION_NODE_TRANSFORMS
            const AnimationNode& GetRootNode() const { return m_rootNode; }
            const auto& GetBoneIDMap() const { return m_BoneInfoMap; }
#endif
        private:
            void AddBoneTrack(AnimationBoneTrack animationBoneTrack);

        private:
            std::unordered_map<u32, u32> m_boneIdToBoneTrack;
            std::vector<AnimationBoneTrack> m_boneTracks;
            // Reference the skeleton this animation is for.
            Ref<Skeleton> m_skeleton;

#if ANIMATION_NODE_TRANSFORMS
            std::map<std::string, SkeletonBone> m_BoneInfoMap;
            AnimationNode m_rootNode;
#endif


            double m_duration = 0.0f;
            double m_ticksPerSecond = 0;

            friend ModelImporter;
        };
    }
}