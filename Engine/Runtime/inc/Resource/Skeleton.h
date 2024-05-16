#pragma once

#include "Runtime/Defines.h"

#include "Core/RefCount.h"

#include "Maths/Matrix4.h"

namespace Insight
{
    namespace Runtime
    {
        class ModelImporter;

        struct SkeletonBone
        {
            SkeletonBone();
            SkeletonBone(const u32 id, std::string name, const Maths::Matrix4 offset);
            ~SkeletonBone();
            
            operator bool() const;
            bool IsValid() const;

            Maths::Matrix4 Offset = Maths::Matrix4::Identity;
            std::string Name = "";
            u32 Id = -1;

            u32 ParentBoneId = -1;
            std::vector<u32> ChildrenBoneIds;
        };

        /// @brief Class to store information about a skeleton and its bones.
        class IS_RUNTIME Skeleton : public Core::RefCount
        {
        public:
            Skeleton();
            virtual ~Skeleton() override;

            bool HasBone(const std::string_view boneName) const;

            const SkeletonBone& GetRootBone() const;

            const std::vector<SkeletonBone>& GetBones() const;
            const SkeletonBone& GetBone(const u32 idx) const;
            const SkeletonBone& GetBone(const std::string_view boneName) const;

            u32 GetNumberOfBones() const;

        private:
            bool HasBone(const u32 boneId) const;
            void AddBone(const SkeletonBone& bone);
            SkeletonBone& GetBone(const std::string_view boneName);

        private:
            u32 m_rootBoneIdx = -1;
            std::vector<SkeletonBone> m_bones;
            SkeletonBone m_inValidBone;

            friend ModelImporter;
        };
    }
}