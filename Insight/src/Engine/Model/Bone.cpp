#include "ispch.h"
#include "Engine/Model/Bone.h"
#include "Engine/Model/ModelLoading.h"

#include "assimp/anim.h"
#include "assimp/scene.h"
#include "glm/gtx/quaternion.hpp"


namespace Insight::Animation
{
	Bone::Bone(std::string name, u32 ID, const aiNodeAnim* channel)
		: m_name(std::move(name))
		, m_ID(ID)
		, m_localTransform(glm::mat4(1.0f))
	{
		m_numPositions = channel->mNumPositionKeys;
		m_numRotations = channel->mNumRotationKeys;
		m_numScalings = channel->mNumScalingKeys;

		for (u32 i = 0; i < m_numPositions; ++i)
		{
			KeyPosition key;
			key.Position = ModelLoading::AssimpLoader::GetGLMVec(channel->mPositionKeys[i].mValue);
			key.TimeStamp = (float)channel->mPositionKeys[i].mTime;
			m_positions.push_back(key);
		}

		for (u32 i = 0; i < m_numRotations; ++i)
		{
			KeyRotation key;
			key.Orientation = ModelLoading::AssimpLoader::GetGLMQuat(channel->mRotationKeys[i].mValue);
			key.TimeStamp = (float)channel->mRotationKeys[i].mTime;
			m_rotations.push_back(key);
		}

		for (u32 i = 0; i < m_numPositions; ++i)
		{
			KeyScale key;
			key.Scale = ModelLoading::AssimpLoader::GetGLMVec(channel->mScalingKeys[i].mValue);
			key.TimeStamp = (float)channel->mScalingKeys[i].mTime;
			m_scales.push_back(key);
		}
	}

	/* Interpolates b/w positions,rotations & scaling keys based on the curren time of the 
	animation and prepares the local transformation matrix by combining all keys tranformations */
	void Bone::Update(const float& animationTime)
	{
		glm::mat4 translation = InterpolatePosition(animationTime);
		glm::mat4 rotation = InterpolateRotation(animationTime);
		glm::mat4 scale = InterpolateScaling(animationTime);
		m_localTransform = translation * rotation * scale;
	}

	/* Gets the current index on mKeyPositions to interpolate to based on the current 
	animation time */
	u32 Bone::GetPositionIndex(const float& animationTime)
	{
		for (u32 index = 0; index < m_numPositions - 1; ++index)
		{
			if (animationTime < m_positions[index + 1].TimeStamp)
				return index;
		}
		ASSERT(false && "[]");
		return -1;
	}
	/* Gets the current index on mKeyRotations to interpolate to based on the current 
	animation time */
	u32 Bone::GetRotationIndex(const float& animationTime)
	{
		for (u32 index = 0; index < m_numRotations - 1; ++index)
		{
			if (animationTime < m_rotations[index + 1].TimeStamp)
				return index;
		}
		ASSERT(false && "[]");
		return -1;
	}
	/* Gets the current index on mKeyScalings to interpolate to based on the current 
	animation time */
	u32 Bone::GetScaleIndex(const float& animationTime)
	{
		for (u32 index = 0; index < m_numScalings - 1; ++index)
		{
			if (animationTime < m_scales[index + 1].TimeStamp)
				return index;
		}
		ASSERT(false && "[]");
		return -1;
	}
	/* Gets normalized value for Lerp & Slerp*/
	float Bone::GetScaleFactor(const float& lastTimeStamp, const float& nextTimeStamp, const float& animationTime)
	{
		float scaleFactor = 0.0f;
		float midWayLength = animationTime - lastTimeStamp;
		float framesDiff = nextTimeStamp - lastTimeStamp;
		scaleFactor = midWayLength / framesDiff;
		return scaleFactor;
	}

	/* figures out which position keys to interpolate b/w and performs the interpolation 
	and returns the translation matrix */
	glm::mat4 Bone::InterpolatePosition(float animationTime)
	{
		if (1 == m_numPositions)
			return glm::translate(glm::mat4(1.0f), m_positions[0].Position);

		int p0Index = GetPositionIndex(animationTime);
		int p1Index = p0Index + 1;
		float scaleFactor = GetScaleFactor(m_positions[p0Index].TimeStamp,
										   m_positions[p1Index].TimeStamp, animationTime);
		glm::vec3 finalPosition = glm::mix(m_positions[p0Index].Position, 
										   m_positions[p1Index].Position
										   , scaleFactor);
		return glm::translate(glm::mat4(1.0f), finalPosition);
	}
	/* figures out which rotations keys to interpolate b/w and performs the interpolation 
	and returns the rotation matrix */
	glm::mat4 Bone::InterpolateRotation(float animationTime)
	{
		if (1 == m_numRotations)
		{
			auto rotation = glm::normalize(m_rotations[0].Orientation);
			return glm::toMat4(rotation);
		}

		int p0Index = GetRotationIndex(animationTime);
		int p1Index = p0Index + 1;
		float scaleFactor = GetScaleFactor(m_rotations[p0Index].TimeStamp,
										   m_rotations[p1Index].TimeStamp, animationTime);
		glm::quat finalRotation = glm::slerp(m_rotations[p0Index].Orientation,
											 m_rotations[p1Index].Orientation, scaleFactor);
		finalRotation = glm::normalize(finalRotation);
		return glm::toMat4(finalRotation);
	}
	/* figures out which scaling keys to interpolate b/w and performs the interpolation 
	and returns the scale matrix */
	glm::mat4 Bone::InterpolateScaling(float animationTime)
	{
		if (1 == m_numScalings)
			return glm::scale(glm::mat4(1.0f), m_scales[0].Scale);

		int p0Index = GetScaleIndex(animationTime);
		int p1Index = p0Index + 1;
		float scaleFactor = GetScaleFactor(m_scales[p0Index].TimeStamp,
										   m_scales[p1Index].TimeStamp, animationTime);
		glm::vec3 finalScale = glm::mix(m_scales[p0Index].Scale, 
										m_scales[p1Index].Scale, scaleFactor);
		return glm::scale(glm::mat4(1.0f), finalScale);
	}
}