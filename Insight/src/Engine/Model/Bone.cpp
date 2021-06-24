#include "ispch.h"
#include "Engine/Model/Bone.h"
#include "Engine/Model/ModelLoading.h"

#include "assimp/anim.h"
#include "assimp/scene.h"
#include "glm/gtx/quaternion.hpp"
#include "tiny_gltf.h"

namespace Insight::Animation
{
	/*Bone::Bone(std::string name, u32 ID, const aiNodeAnim* channel)
		: m_name(std::move(name))
		, m_ID(ID)
		, m_animatedTransform(glm::mat4(1.0f))
		, m_localBindTransform(glm::mat4(1.0f))
		, m_inverseBindTransform(glm::mat4(1.0f))
	{*/
		/*m_positions.reserve(channel->mNumPositionKeys);
		m_rotations.reserve(channel->mNumRotationKeys);
		m_scales.reserve(channel->mNumScalingKeys);

		for (u32 i = 0; i < channel->mNumPositionKeys; ++i)
		{
			KeyPosition key;
			key.Position = ModelLoading::AssimpLoader::GetGLMVec(channel->mPositionKeys[i].mValue);
			key.TimeStamp = (float)channel->mPositionKeys[i].mTime;
			m_positions.push_back(key);
		}

		for (u32 i = 0; i < channel->mNumRotationKeys; ++i)
		{
			KeyRotation key;
			key.Orientation = ModelLoading::AssimpLoader::GetGLMQuat(channel->mRotationKeys[i].mValue);
			key.TimeStamp = (float)channel->mRotationKeys[i].mTime;
			m_rotations.push_back(key);
		}

		for (u32 i = 0; i < channel->mNumScalingKeys; ++i)
		{
			KeyScale key;
			key.Scale = ModelLoading::AssimpLoader::GetGLMVec(channel->mScalingKeys[i].mValue);
			key.TimeStamp = (float)channel->mScalingKeys[i].mTime;
			m_scales.push_back(key);
		}*/
	//}

	Bone::Bone(std::string name, u32 ID, glm::mat4 bindLocalTransform, Skeleton* skeleton)
		: m_name(std::move(name))
		, m_ID(ID)
		, m_localBindTransform(bindLocalTransform)
		, m_inverseBindTransform(glm::mat4(1.0f))
		, m_skeleton(skeleton)
	{}

	/* Interpolates b/w positions,rotations & scaling keys based on the curren time of the
	animation and prepares the local transformation matrix by combining all keys tranformations */
	void Bone::Update(const float& animationTime)
	{
		/*glm::mat4 translation = InterpolatePosition(animationTime);
		glm::mat4 rotation = InterpolateRotation(animationTime);
		glm::mat4 scale = InterpolateScaling(animationTime);
		m_localTransform = translation * rotation * scale;*/
	}

	void Bone::AddChildBone(u32 childBone)
	{
		m_children.push_back(childBone);
	}

	void Bone::CalcInverseBindTransform(glm::mat4 parentBindTransform)
	{
		glm::mat4 bindTransform = parentBindTransform * m_localBindTransform;
		m_inverseBindTransform = glm::inverse(bindTransform);
		for (auto& c : m_children)
		{
			m_skeleton->GetBone(c).CalcInverseBindTransform(bindTransform);
		}
	}

	void Bone::SetNodeTransform(glm::mat4 nodeTransform)
	{
		m_nodeTranform = nodeTransform;
	}

	/* Gets the current index on mKeyPositions to interpolate to based on the current 
	animation time */
	//u32 Bone::GetPositionIndex(const float& animationTime)
	//{
	//	for (u32 index = 0; index < m_positions.size() - 1; ++index)
	//	{
	//		if (animationTime < m_positions[index + 1].TimeStamp)
	//			return index;
	//	}
	//	ASSERT(false && "[Bone::GetPositionIndex] Unable to get position index.");
	//	return -1;
	//}
	///* Gets the current index on mKeyRotations to interpolate to based on the current 
	//animation time */
	//u32 Bone::GetRotationIndex(const float& animationTime)
	//{
	//	for (u32 index = 0; index < m_rotations.size() - 1; ++index)
	//	{
	//		if (animationTime < m_rotations[index + 1].TimeStamp)
	//			return index;
	//	}
	//	ASSERT(false && "[Bone::GetRotationIndex] Unable to get rotation index.");
	//	return -1;
	//}
	///* Gets the current index on mKeyScalings to interpolate to based on the current 
	//animation time */
	//u32 Bone::GetScaleIndex(const float& animationTime)
	//{
	//	for (u32 index = 0; index < m_scales.size() - 1; ++index)
	//	{
	//		if (animationTime < m_scales[index + 1].TimeStamp)
	//			return index;
	//	}
	//	ASSERT(false && "[]");
	//	return -1;
	//}
	///* Gets normalized value for Lerp & Slerp*/
	//float Bone::GetScaleFactor(const float& lastTimeStamp, const float& nextTimeStamp, const float& animationTime)
	//{
	//	float scaleFactor = 0.0f;
	//	float midWayLength = animationTime - lastTimeStamp;
	//	float framesDiff = nextTimeStamp - lastTimeStamp;
	//	scaleFactor = midWayLength / framesDiff;
	//	return scaleFactor;
	//}

	///* figures out which position keys to interpolate b/w and performs the interpolation 
	//and returns the translation matrix */
	//glm::mat4 Bone::InterpolatePosition(float animationTime)
	//{
	//	if (1 == m_positions.size())
	//		return glm::translate(glm::mat4(1.0f), m_positions[0].Position);

	//	int p0Index = GetPositionIndex(animationTime);
	//	int p1Index = p0Index + 1;
	//	float scaleFactor = GetScaleFactor(m_positions[p0Index].TimeStamp,
	//									   m_positions[p1Index].TimeStamp, animationTime);
	//	glm::vec3 finalPosition = glm::mix(m_positions[p0Index].Position, 
	//									   m_positions[p1Index].Position
	//									   , scaleFactor);
	//	return glm::translate(glm::mat4(1.0f), finalPosition);
	//}
	///* figures out which rotations keys to interpolate b/w and performs the interpolation 
	//and returns the rotation matrix */
	//glm::mat4 Bone::InterpolateRotation(float animationTime)
	//{
	//	if (1 == m_rotations.size())
	//	{
	//		auto rotation = glm::normalize(m_rotations[0].Orientation);
	//		return glm::toMat4(rotation);
	//	}

	//	int p0Index = GetRotationIndex(animationTime);
	//	int p1Index = p0Index + 1;
	//	float scaleFactor = GetScaleFactor(m_rotations[p0Index].TimeStamp,
	//									   m_rotations[p1Index].TimeStamp, animationTime);
	//	glm::quat finalRotation = glm::slerp(m_rotations[p0Index].Orientation,
	//										 m_rotations[p1Index].Orientation, scaleFactor);
	//	finalRotation = glm::normalize(finalRotation);
	//	return glm::toMat4(finalRotation);
	//}
	///* figures out which scaling keys to interpolate b/w and performs the interpolation 
	//and returns the scale matrix */
	//glm::mat4 Bone::InterpolateScaling(float animationTime)
	//{
	//	if (1 == m_scales.size())
	//		return glm::scale(glm::mat4(1.0f), m_scales[0].Scale);

	//	int p0Index = GetScaleIndex(animationTime);
	//	int p1Index = p0Index + 1;
	//	float scaleFactor = GetScaleFactor(m_scales[p0Index].TimeStamp,
	//									   m_scales[p1Index].TimeStamp, animationTime);
	//	glm::vec3 finalScale = glm::mix(m_scales[p0Index].Scale, 
	//									m_scales[p1Index].Scale, scaleFactor);
	//	return glm::scale(glm::mat4(1.0f), finalScale);
	//}

	Skeleton::Skeleton()
		: m_rootBoneId(-1)
		, m_boneCount(0)
	{ }

	Skeleton::Skeleton(u32 boneCount, glm::mat4 globalTransform)
		: m_rootBoneId(-1)
		, m_boneCount(0)
	{
		m_bones.reserve(boneCount);
		m_globalTransform = globalTransform;
	}

	void Skeleton::PopulateBoneData(const tinygltf::Model& gltfModel)
	{
		ReadHierarchyData(gltfModel.nodes.at(0), gltfModel);
	}

	void Skeleton::PopulateBoneData(const aiScene* aiScene)
	{
		ReadHierarchyData(aiScene->mRootNode);
	}

	Bone& Skeleton::GetBone(u32 index)
	{
		return m_bones.at(index);
	}

	Bone& Skeleton::GetBone(std::string name)
	{
		auto itr = m_boneNameToIndex.at(name);
		return m_bones.at(itr);
	}

	void Skeleton::AddBone(const std::string& name, glm::mat4 localBindTransform)
	{
		Bone newBone = Bone(name, m_boneCount, localBindTransform, this);
		m_boneNameToIndex[name] = m_boneCount;
		m_bones.push_back(newBone);
		++m_boneCount;
	}

	void Skeleton::ReadHierarchyData(const tinygltf::Node& node, const tinygltf::Model& gltfModel)
	{
		std::string nodeName = node.name;
		Bone* bone = nullptr;
		if (HasBone(nodeName))
		{
			bone = &GetBone(nodeName);
			if (m_rootBoneId == -1)
			{
				m_rootBoneId = bone->GetBoneId();
			}
		}

		if (bone && bone->GetBoneId() == m_rootBoneId && bone->GetChildrenCount() > 0)
		{
			auto itr = std::find_if(bone->GetChildren().begin(), bone->GetChildren().end(), [this](const u32& index) 
						 {
							 return m_rootBoneId == index;
						 });
			if (itr != bone->GetChildren().end())
			{
				bone->GetChildren().erase(itr);
			}
			// Found the root.
			return;
		}

		for (u32 i = 0; i < node.children.size(); ++i)
		{
			if (bone)
			{
				std::string childreName(gltfModel.nodes.at(node.children.at(i)).name);
				if (HasBone(childreName))
				{
					bone->AddChildBone(GetBone(childreName).GetBoneId());
					bone->SetNodeTransform(ModelLoading::GltfLoader::GetNodeMatrix(gltfModel.nodes.at(node.children.at(i))));
				}
			}
			bool isRootNode = false;
			if(bone && bone->GetBoneId() == m_rootBoneId)
			{
				isRootNode = true;
			}
			ReadHierarchyData(gltfModel.nodes.at(node.children.at(i)), gltfModel);
		}
	}

	void Skeleton::ReadHierarchyData(const aiNode* node)
	{
		std::string nodeName = node->mName.C_Str();
		Bone* bone = nullptr;
		if (HasBone(nodeName))
		{
			bone = &GetBone(nodeName);
			if (m_rootBoneId == -1)
			{
				m_rootBoneId = bone->GetBoneId();
			}
		}
		for (u32 i = 0; i < node->mNumChildren; ++i)
		{
			if (bone)
			{
				std::string childreName(node->mChildren[i]->mName.C_Str());
				if (HasBone(childreName))
				{
					bone->AddChildBone(GetBone(childreName).GetBoneId());
					bone->SetNodeTransform(ModelLoading::AssimpLoader::AssimpToGLM(node->mTransformation));
				}
			}
			ReadHierarchyData(node->mChildren[i]);
		}
	}
}