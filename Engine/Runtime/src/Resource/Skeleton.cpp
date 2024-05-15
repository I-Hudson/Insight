#include "Resource/Skeleton.h"

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
    }
}