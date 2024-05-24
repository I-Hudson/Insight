#include "Resource/Skeleton.h"

#include "Core/Logger.h"

namespace Insight
{
    namespace Runtime
    {
        SkeletonBone::SkeletonBone()
        { }
        //========================
        // SkeletonBone
        //========================
        SkeletonBone::SkeletonBone(const u32 id, std::string name, const Maths::Matrix4 offset)
            : Id(id), Name(std::move(name)), Offset(offset)
        { }
        SkeletonBone::~SkeletonBone()
        { }

        SkeletonBone::operator bool() const
        {
            return IsValid();
        }

        bool SkeletonBone::IsValid() const
        {
            return Id != -1;
        }

        //========================
        // Skeleton
        //========================
        Skeleton::Skeleton()
        { }
        Skeleton::~Skeleton()
        { }

        bool Skeleton::HasBone(const std::string_view boneName) const
        {
            for (size_t boneIdx = 0; boneIdx < m_bones.size(); ++boneIdx)
            {
                const SkeletonBone& bone = m_bones[boneIdx];
                if (bone.Name == boneName)
                {
                    return true;
                }
            }
            return false;
        }

        const SkeletonBone& Skeleton::GetRootBone() const
        {
            return m_bones[m_rootBoneIdx];
        }

        const SkeletonNode& Skeleton::GetRootNode() const
        {
            return m_skeletonNodes[0];
        }

        const std::vector<SkeletonBone>& Skeleton::GetBones() const
        {
            return m_bones;
        }

        const SkeletonBone& Skeleton::GetBone(const u32 idx) const
        {
            ASSERT(idx < m_bones.size());
            return m_bones[idx];
        }

        const SkeletonBone& Skeleton::GetBone(const std::string_view boneName) const
        {
            for (size_t boneIdx = 0; boneIdx < m_bones.size(); ++boneIdx)
            {
                const SkeletonBone& bone = m_bones[boneIdx];
                if (bone.Name == boneName)
                {
                    return bone;
                }
            }
            return m_inValidBone;
        }

        u32 Skeleton::GetNumberOfBones() const
        {
            return static_cast<u32>(m_bones.size());
        }

        const Maths::Matrix4& Skeleton::GetGlobalInverseTransform() const
        {
            return m_globalInverseTransforms;
        }

        bool Skeleton::HasBone(const u32 boneId) const
        {
            for (size_t boneIdx = 0; boneIdx < m_bones.size(); ++boneIdx)
            {
                const SkeletonBone& bone = m_bones[boneIdx];
                if (bone.Id == boneId)
                {
                    return true;
                }
            }
            return false;
        }

        void Skeleton::AddBone(const SkeletonBone& bone)
        {
            ASSERT(GetNumberOfBones() < c_MaxBoneCount);
            if (!HasBone(bone.Name) && !HasBone(bone.Id))
            {
                m_bones.push_back(bone);
                m_boneMaps[bone.Name] = bone;
            }
            else
            {
                IS_LOG_CORE_ERROR("[Skeleton::AddBone] Bones needs to unique.");
            }
        }

        SkeletonBone& Skeleton::GetBone(const std::string_view boneName)
        {
            for (size_t boneIdx = 0; boneIdx < m_bones.size(); ++boneIdx)
            {
                SkeletonBone& bone = m_bones[boneIdx];
                if (bone.Name == boneName)
                {
                    return bone;
                }
            }
            return m_inValidBone;
        }

        SkeletonNode* Skeleton::GetNode(std::string_view name)
        {
            for (size_t i = 0; i < m_skeletonNodes.size(); i++)
            {
                if (m_skeletonNodes[i].Name == name) 
                {
                    return &m_skeletonNodes[i];
                }
            }
            return nullptr;
        }
    }
}