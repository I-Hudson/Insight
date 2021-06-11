#pragma once

#include "Engine/Core/Common.h"
#include "Engine/Model/Bone.h"

struct aiNodeAnim;
struct aiAnimation;
struct aiNode;
struct aiScene;

namespace Insight
{
	class Model;
}

namespace Insight::Animation
{
	struct NodeData
	{
		glm::mat4 Transform;
		std::string Name;
		u32 ChildrenCount;
		std::vector<NodeData> Children;
	};

	class Animation
	{
	public:
		Animation() = default;
		Animation(const aiScene* aiScene, Model* model);

		Bone* FindBone(const std::string& name);

		INLINE float GetTicksPerSecond() const { return m_ticksPerSecond; }
		INLINE float GetDuration() const { return m_duration; }
		INLINE NodeData& GetRootNode() { return m_rootNode; }
		INLINE const std::unordered_map<std::string, BoneInfo>& GetBoneInfoMap() { return m_boneInfoMap; }

	private:
		void ReadMissingBones(const aiAnimation* animation, Model* model);
		void ReadHeirarchyData(NodeData& dest, const aiNode* src);

	private:
		float m_ticksPerSecond;
		float m_duration;
		NodeData m_rootNode;
		std::vector<Bone> m_bones;
		std::unordered_map<std::string, BoneInfo> m_boneInfoMap;
	};
}