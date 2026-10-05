#include "Animation/Animator.h"
#include "Core/Profiler.h"
#include "Core/Asserts.h"

#define OPTIMISE_MATRIX_MATHS 1

namespace Insight
{
    namespace Runtime
    {
        Animator::Animator()
        { }
        Animator::~Animator()
        { }

        Ref<Skeleton> Animator::GetSkelton() const
        {
            return m_skeleton;
        }

        void Animator::SetSkelton(Ref<Skeleton> skeleton)
        {
            if (m_skeleton != skeleton)
            {
                m_skeleton = skeleton;
                m_boneMatrices.resize(m_skeleton->GetNumberOfBones(), Maths::Matrix4::Identity);
                Reset();
                SetBindPose();
            }
        }

        Ref<AnimationClip> Animator::GetAnimationClip() const
        {
            return m_animationClip;
        }

        void Animator::SetAnimationClip(Ref<AnimationClip> animationClip)
        {
            if (m_animationClip != animationClip)
            {
                m_animationClip = animationClip;
                Reset();
            }
        }

        void Animator::Update(const float deltaTime)
        {
            IS_PROFILE_FUNCTION();

            if (m_isPlaying 
                && m_skeleton 
                && m_animationClip)
            {
                m_currentAnimationTimeTicks += m_animationClip->GetTickPerSecond() * static_cast<double>(deltaTime);
                m_currentAnimationTimeTicks = fmod(m_currentAnimationTimeTicks, m_animationClip->GetDurationTicks());
                CalculateBoneTransform(m_skeleton->GetRootBone().Id, Maths::Matrix4::Identity);
#if ANIMATION_NODE_TRANSFORMS
                //CalculateBoneTransform(&m_animationClip->GetRootNode(), Maths::Matrix4::Identity);
#endif
            }
        }

        void Animator::Play(const bool resetClip)
        {
            if (resetClip)
            {
                Reset();
            }
            m_isPlaying = true;
        }

        void Animator::Stop()
        {
            m_isPlaying = false;
        }

        const std::vector<Maths::Matrix4>& Animator::GetBoneTransforms() const
        {
            return m_boneMatrices;
        }

#if 0
        void Animator::CalculateBoneTransform(const u32 boneId, const Maths::Vector3 parentPosition, const Maths::Quaternion parentQuaternion, const Maths::Vector3 parentScale)
        {
            IS_PROFILE_FUNCTION();

            const SkeletonBone& bone = m_skeleton->GetBone(boneId);
            ASSERT(bone);

            const Maths::Vector3 bonePositionVector = InterpolatePositionVec(boneId);
            const Maths::Quaternion boneRotationQuat = InterpolateRotationQuat(boneId);
            const Maths::Vector3 boneScaleVector = InterpolateScaleVec(boneId);

            const Maths::Vector3 globalPosition = parentPosition * bonePositionVector;
            const Maths::Quaternion globalRotation = parentQuaternion * boneRotationQuat;
            const Maths::Vector3 globalScale = parentScale * boneScaleVector;

            const Maths::Matrix4 globalTransform = Maths::Matrix4::Identity
                .Scaled(Maths::Vector4(globalScale, 1.0f))
                    .Rotated(globalRotation)
                    .Translated(Maths::Vector4(globalPosition, 1.0f));

            const Maths::Matrix4 boneOffsetTransform = m_skeleton->GetGlobalInverseTransform() * globalTransform * bone.Offset;
            m_boneMatrices[boneId] = boneOffsetTransform;

            for (size_t childBoneIdx = 0; childBoneIdx < bone.ChildrenBoneIds.size(); ++childBoneIdx)
            {
                const u32 childBoneId = bone.ChildrenBoneIds[childBoneIdx];
                CalculateBoneTransform(childBoneId, globalPosition, globalRotation, globalScale);
            }
        }
#endif

        void Animator::CalculateBoneTransform(const u32 boneId, const Maths::Matrix4 parentTransform)
        {
            IS_PROFILE_FUNCTION();

            const SkeletonBone& bone = m_skeleton->GetBone(boneId);
            ASSERT(bone);

            Maths::Matrix4 boneTransform;
            {
                IS_PROFILE_SCOPE("BoneTransform");
#if OPTIMISE_MATRIX_MATHS
            const Maths::Vector4 bonePosition = InterpolatePositionVec(bone.Id);
            const Maths::Quaternion boneRotation = InterpolateRotationQuat(bone.Id);
            const Maths::Vector4 boneScale = InterpolateScaleVec(bone.Id);
            
            boneTransform = ConstructBoneMatrix(bonePosition, boneRotation, boneScale);
#else
            const Maths::Matrix4 bonePositionMatrix = InterpolatePosition(boneId);
            const Maths::Matrix4 boneRotationMatrix = InterpolateRotation(boneId);
            const Maths::Matrix4 boneScaleMatrix = InterpolateScale(boneId);
            
            boneTransform = bonePositionMatrix * boneRotationMatrix * boneScaleMatrix;
#endif
            }
            const Maths::Matrix4 globalTransform = parentTransform * boneTransform;

            const Maths::Matrix4 boneOffsetTransform = m_skeleton->GetGlobalInverseTransform() * globalTransform * bone.Offset;
            {
                IS_PROFILE_SCOPE("Place BoneOffsetTransform");
                m_boneMatrices[boneId] = boneOffsetTransform;
            }

            for (size_t childBoneIdx = 0; childBoneIdx < bone.ChildrenBoneIds.size(); ++childBoneIdx)
            {
                const u32 childBoneId = bone.ChildrenBoneIds[childBoneIdx];
                CalculateBoneTransform(childBoneId, globalTransform);
            }
        }

        Maths::Matrix4 Animator::ConstructBoneMatrix(const Maths::Vector4& position, const Maths::Quaternion& rotation, const Maths::Vector4& scale) const
        {
            // 1. Calculate intermediate quaternion values
            float x2 = rotation.x + rotation.x;
            float y2 = rotation.y + rotation.y;
            float z2 = rotation.z + rotation.z;

            float xx = rotation.x * x2; float xy = rotation.x * y2; float xz = rotation.x * z2;
            float yy = rotation.y * y2; float yz = rotation.y * z2; float zz = rotation.z * z2;
            float wx = rotation.w * x2; float wy = rotation.w * y2; float wz = rotation.w * z2;

            Maths::Matrix4 outMatrix;
            // 2. Compute the scaled Basis Vectors (Columns 0, 1, 2)
            // Column 0 (X-axis) * Scale.x
            outMatrix[0][0] = (1.0f - (yy + zz)) * scale.x;
            outMatrix[0][1] = (xy + wz) * scale.x;
            outMatrix[0][2] = (xz - wy) * scale.x;
            outMatrix[0][3] = 0.0f;

            // Column 1 (Y-axis) * Scale.y
            outMatrix[1][0] = (xy - wz) * scale.y;
            outMatrix[1][1] = (1.0f - (xx + zz)) * scale.y;
            outMatrix[1][2] = (yz + wx) * scale.y;
            outMatrix[1][3] = 0.0f;

            // Column 2 (Z-axis) * Scale.z
            outMatrix[2][0] = (xz + wy) * scale.z;
            outMatrix[2][1] = (yz - wx) * scale.z;
            outMatrix[2][2] = (1.0f - (xx + yy)) * scale.z;
            outMatrix[2][3] = 0.0f;

            // 3. Insert Position directly into Column 3 (Translation)
            outMatrix[3][0] = position.x;
            outMatrix[3][1] = position.y;
            outMatrix[3][2] = position.z;
            outMatrix[3][3] = 1.0f; // or pos.w if you explicitly track it

            return outMatrix;
        }

#if ANIMATION_NODE_TRANSFORMS
        void Animator::CalculateBoneTransform(const AnimationNode* node, const Maths::Matrix4 parentTransform)
        {
            IS_PROFILE_FUNCTION();

            const AnimationBoneTrack* bone = m_animationClip->GetBoneTrack(node->Name);

            Maths::Matrix4 nodeTransform = node->Transform;
            if (bone)
            {
                const Maths::Matrix4 bonePositionMatrix = InterpolatePosition(bone->BoneId);
                const Maths::Matrix4 boneRotationMatrix = InterpolateRotation(bone->BoneId);
                const Maths::Matrix4 boneScaleMatrix = InterpolateScale(bone->BoneId);


                const Maths::Matrix4 boneTransform = bonePositionMatrix * boneRotationMatrix * boneScaleMatrix;
                nodeTransform = boneTransform;
            }

            const Maths::Matrix4 globalTransform = parentTransform * nodeTransform;

            auto boneInfoMap = m_animationClip->GetBoneIDMap();
            if (boneInfoMap.find(node->Name) != boneInfoMap.end())
            {
                int index = boneInfoMap[node->Name].Id;
                Maths::Matrix4 offset = boneInfoMap[node->Name].Offset;
                m_boneMatrices[index] = globalTransform * offset;
            }

            /*
            if (bone)
            {
                const Maths::Matrix4 boneOffsetTransform = globalTransform * bone.Offset;
                m_boneMatrices[bone.Id] = boneOffsetTransform;
            }
            */
            for (size_t childBoneIdx = 0; childBoneIdx < node->ChildrenCount; ++childBoneIdx)
            {
                CalculateBoneTransform(&node->Children[childBoneIdx], globalTransform);
            }
        }
#endif

        float Animator::GetScaleFactor(const double lastTimeStamp, const double nextTimeStamp) const
        {
            float scaleFactor = 0.0f;
            const double midWayLength = m_currentAnimationTimeTicks - lastTimeStamp;
            const double framesDiff = nextTimeStamp - lastTimeStamp;
            scaleFactor = static_cast<float>(midWayLength / framesDiff);
            return scaleFactor;
        }

        Maths::Matrix4 Animator::InterpolatePosition(const u32 boneId) const
        {
            return Maths::Matrix4::Identity.Translated(Maths::Vector4(InterpolatePositionVec(boneId), 1.0f));
        }

        Maths::Matrix4 Animator::InterpolateRotation(const u32 boneId) const
        {
            return Maths::Matrix4(InterpolateRotationQuat(boneId));
        }

        Maths::Matrix4 Animator::InterpolateScale(const u32 boneId) const
        {
            return Maths::Matrix4::Identity.Scaled(Maths::Vector4(InterpolateScaleVec(boneId), 1.0f));
        }

        Maths::Vector3 Animator::InterpolatePositionVec(const u32 boneId) const
        {
            IS_PROFILE_FUNCTION();

            if (!m_animationClip)
            {
                return Maths::Vector3::Zero;
            }

            const AnimationBoneTrack* boneTrack = m_animationClip->GetBoneTrack(boneId);
            if (!boneTrack)
            {
                return Maths::Vector3::Zero;
            }
            else if (boneTrack->Positions.size() == 1)
            {
                const Maths::Vector4 position = Maths::Vector4(boneTrack->Positions[0].Position, 1.0f);
                return position;
            }
                
            const u32 p0Index = boneTrack->GetPositionKeyFrameIndex(m_currentAnimationTimeTicks);
            const u32 p1Index = p0Index + 1;

            const AnimationBoneTrack::PositionKeyFrame& p0KeyFrame = boneTrack->Positions[p0Index];
            const AnimationBoneTrack::PositionKeyFrame& p1KeyFrame = boneTrack->Positions[p1Index];

            const float scaleFactor = GetScaleFactor(p0KeyFrame.TimeStamp, p1KeyFrame.TimeStamp);
            const Maths::Vector3 finalPosition = p0KeyFrame.Position.Lerp(p1KeyFrame.Position, scaleFactor);

            return finalPosition;
        }

        Maths::Quaternion Animator::InterpolateRotationQuat(const u32 boneId) const
        {
            IS_PROFILE_FUNCTION();

            if (!m_animationClip)
            {
                return Maths::Quaternion::Identity;
            }

            const AnimationBoneTrack* boneTrack = m_animationClip->GetBoneTrack(boneId);
            if (!boneTrack)
            {
                return Maths::Quaternion::Identity;
            }
            else if (boneTrack->Rotations.size() == 1)
            {
                return boneTrack->Rotations[0].Rotation.Normalised();
            }

            const u32 p0Index = boneTrack->GetRotationKeyFrameIndex(m_currentAnimationTimeTicks);
            const u32 p1Index = p0Index + 1;

            const AnimationBoneTrack::RotationKeyFrame& p0KeyFrame = boneTrack->Rotations[p0Index];
            const AnimationBoneTrack::RotationKeyFrame& p1KeyFrame = boneTrack->Rotations[p1Index];

            const float scaleFactor = GetScaleFactor(p0KeyFrame.TimeStamp, p1KeyFrame.TimeStamp);
            const Maths::Quaternion finalRotation = p0KeyFrame.Rotation.Slerp(p1KeyFrame.Rotation, scaleFactor);

            return finalRotation.Normalised();
        }

        Maths::Vector3 Animator::InterpolateScaleVec(const u32 boneId) const
        {
            IS_PROFILE_FUNCTION();

            if (!m_animationClip)
            {
                return Maths::Vector3::One;
            }

            const AnimationBoneTrack* boneTrack = m_animationClip->GetBoneTrack(boneId);
            if (!boneTrack)
            {
                return Maths::Vector3::One;
            }
            else if (boneTrack->Scales.size() == 1)
            {
                const Maths::Vector3 scale = boneTrack->Scales[0].Scale;
                return scale;
            }

            const u32 p0Index = boneTrack->GetScaleKeyFrameIndex(m_currentAnimationTimeTicks);
            const u32 p1Index = p0Index + 1;

            const AnimationBoneTrack::ScaleKeyFrame& p0KeyFrame = boneTrack->Scales[p0Index];
            const AnimationBoneTrack::ScaleKeyFrame& p1KeyFrame = boneTrack->Scales[p1Index];

            const float scaleFactor = GetScaleFactor(p0KeyFrame.TimeStamp, p1KeyFrame.TimeStamp);
            const Maths::Vector3 finalScale = p0KeyFrame.Scale.Lerp(p1KeyFrame.Scale, scaleFactor);

            return finalScale;
        }

        void Animator::Reset()
        {
            m_currentAnimationTimeTicks = 0;
        }

        void Animator::SetBindPose()
        {
            const u32 boneSize = m_skeleton->GetNumberOfBones();
            for (u32 i = 0; i < boneSize; ++i)
            {
                const SkeletonBone& bone = m_skeleton->GetBone(i);
                m_boneMatrices[bone.Id] = Maths::Matrix4::Identity;
            }
        }
    }
}