#include "ispch.h"
#include "Engine/Model/Animation.h"
#include "Engine/Model/Model.h"
#include "Engine/Model/ModelLoading.h"

#include "assimp/scene.h"

namespace Insight::Animation
{
	/// <summary>
	/// Animation
	/// </summary>
	/// <param name="aiScene"></param>
	/// <param name="model"></param>
	Animation::Animation(const aiScene* aiScene, Model* model)
	{

	}

	Bone* Animation::FindBone(const std::string & name)
	{
		auto iter = std::find_if(m_bones.begin(), m_bones.end(), [&](const Bone& Bone)
								 {
									 return Bone.GetBoneName() == name;
								 });
		if (iter == m_bones.end())
		{
			return nullptr;
		}
		else
		{
			return &(*iter);
		}
	}

	void Animation::ReadMissingBones(const aiAnimation* animation, Model* model)
	{
		u32 size = animation->mNumChannels;

		auto& boneInfoMap = model->GetMesh().GetBoneInfoMap();//getting m_BoneInfoMap from Model class
		u32& boneCount = model->GetMesh().GetBoneCount(); //getting the m_BoneCounter from Model class

											   //reading channels(bones engaged in an animation and their keyframes)
		for (u32 i = 0; i < size; i++)
		{
			auto channel = animation->mChannels[i];
			std::string boneName = channel->mNodeName.data;

			if (boneInfoMap.find(boneName) == boneInfoMap.end())
			{
				boneInfoMap[boneName].Id = boneCount;
				boneCount++;
			}
			m_bones.push_back(Bone(channel->mNodeName.data,
							  boneInfoMap[channel->mNodeName.data].Id, channel));
		}
		m_boneInfoMap = boneInfoMap;
	}

	void Animation::ReadHeirarchyData(NodeData& dest, const aiNode* src)
	{
		ASSERT(src);

		dest.Name = src->mName.data;
		dest.Transform = ModelLoading::AssimpLoader::AssimpToGLM(src->mTransformation);
		dest.ChildrenCount = src->mNumChildren;

		for (u32 i = 0; i < src->mNumChildren; i++)
		{
			NodeData newData;
			ReadHeirarchyData(newData, src->mChildren[i]);
			dest.Children.push_back(newData);
		}
	}
}