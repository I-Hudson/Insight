#pragma once

#include "Runtime/Defines.h"

#include "Core/RefCount.h"

#include "Maths/Matrix4.h"

#include <map>

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

            /// @brief This isn't actual it's parents transform, just a transform which needs to be applied before the offset is.
            Maths::Matrix4 ParentTransform;
            Maths::Matrix4 Offset;
            std::string Name = "";
            u32 Id = -1;

            u32 ParentBoneId = -1;
            std::vector<u32> ChildrenBoneIds;
        };

        struct SkeletonNode
        {
            std::string Name;
            Maths::Matrix4 Transforms;
            std::string ParentName;
            std::vector<std::string> ChildrenNames;
            
            u32 BoneId;
            std::string BoneName;
        };

        /// @brief Class to store information about a skeleton and its bones.
        class IS_RUNTIME Skeleton : public Core::RefCount
        {
        public:
            Skeleton();
            virtual ~Skeleton() override;

            bool HasBone(const std::string_view boneName) const;

            const SkeletonBone& GetRootBone() const;
            const SkeletonNode& GetRootNode() const;

            const std::vector<SkeletonBone>& GetBones() const;
            const SkeletonBone& GetBone(const u32 idx) const;
            const SkeletonBone& GetBone(const std::string_view boneName) const;

            u32 GetNumberOfBones() const;
        
            SkeletonNode* GetNode(std::string_view name);
            SkeletonBone& GetBone(const std::string_view boneName);
        private:
            bool HasBone(const u32 boneId) const;
            void AddBone(const SkeletonBone& bone);

        private:
            SkeletonBone m_inValidBone;
            Maths::Matrix4 m_globalInverseTransforms = Maths::Matrix4::Identity;
            std::vector<SkeletonBone> m_bones;
            std::map<std::string, SkeletonBone> m_boneMaps;
            u32 m_rootBoneIdx = -1;

            std::vector<SkeletonNode> m_skeletonNodes;

            friend ModelImporter;
        };
    }
}