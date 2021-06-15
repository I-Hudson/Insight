#pragma once

#include "Engine/Core/Common.h"
#include "glm/glm.hpp"
#include "glm/gtc/quaternion.hpp"

struct aiNodeAnim;
struct aiNode;
struct aiScene;

namespace Insight::Animation
{
	class Skeleton;

	class Bone
	{
	public:
		//Bone(std::string name, u32 ID, const aiNodeAnim* channel);
		Bone(std::string name, u32 ID, glm::mat4 bindLocalTransform, Skeleton* skelton);

		void Update(const float& animationTime);
		u32 GetBoneId() const { return m_ID; }
		u32 GetParentBoneId() const { return m_parentId; }
		std::string GetBoneName() const { return m_name; }
		void AddChildBone(u32 childBoneId);
		std::vector<u32>& GetChildren() { return m_children; }
		u32 GetChildrenCount() const { return static_cast<u32>(m_children.size()); }

		glm::mat4 GetLocalBindTransform() const { return m_localBindTransform; }
		glm::mat4 GetInverseBindTransform() const { return m_inverseBindTransform; }

		void CalcInverseBindTransform(glm::mat4 parentBindTransform);
		void SetNodeTransform(glm::mat4 nodeTransform);

	//	u32 GetPositionIndex(const float& animationTime);
	//	u32 GetRotationIndex(const float& animationTime);
	//	u32 GetScaleIndex(const float& animationTime);

	//private:
	//	float GetScaleFactor(const float& lastTimeStamp, const float& nextTimeStamp, const float& animationTime);

	//	glm::mat4 InterpolatePosition(float animationTime);
	//	glm::mat4 InterpolateRotation(float animationTime);
	//	glm::mat4 InterpolateScaling(float animationTime);

	private:
		u32 m_ID;
		u32 m_parentId;
		std::string m_name;
		std::vector<u32> m_children;
		glm::mat4 m_animatedTransform; // Use this to move a vertex into the correct position. This should be model space.
		glm::mat4 m_nodeTranform;

		glm::mat4 m_localBindTransform; // Original transform in relation to parent bone.
		glm::mat4 m_inverseBindTransform; // Original transform in relate to model space.

		Skeleton* m_skeleton;
	};

	class Skeleton
	{
	public:
		Skeleton();
		Skeleton(u32 boneCount, glm::mat4 globalTransform);

		void PopulateBoneData(const aiScene* aiScene);

		Bone& GetRootBone() { return m_bones.at(m_rootBoneId); }
		Bone& GetBone(u32 index);
		Bone& GetBone(std::string name);
		u32 GetBoneCount() const { return m_boneCount; }
		const std::vector<Bone>& GetAllBones() { return m_bones; }

		void AddBone(const std::string& name, glm::mat4 localBindTransform);
		bool HasBone(const std::string& name) { return m_boneNameToIndex.find(name) != m_boneNameToIndex.end(); }

		void ReadHierarchyData(const aiNode* node);

	private:
		glm::mat4 m_globalTransform;
		u32 m_rootBoneId = -1;
		std::vector<Bone> m_bones;
		std::unordered_map<std::string, u32> m_boneNameToIndex;
		u32 m_boneCount = 0;
	};
}