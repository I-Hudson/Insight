#include "ispch.h"
#include "Engine/Model/Animation.h"
#include "Engine/Model/Model.h"
#include "Engine/Model/ModelLoading.h"

#include "assimp/scene.h"
#include "assimp/anim.h"

namespace Insight::Animation
{
	/// <summary>
	/// Animation
	/// </summary>
	/// <param name="aiScene"></param>
	/// <param name="model"></param>
	Animation::Animation(const aiScene* aiScene, u32 animationIndex, Model* model)
	{
		const aiAnimation* aiAnimation = aiScene->mAnimations[animationIndex];
		m_duration = (float)aiAnimation->mDuration;
		m_ticksPerSecond = (float)aiAnimation->mTicksPerSecond;

		for (u32 i = 0; i < aiAnimation->mNumChannels; ++i)
		{
			const aiNodeAnim* channel = aiAnimation->mChannels[i];
			std::string boneName = channel->mNodeName.C_Str();

			for (u32 positionKeys = 0; positionKeys < channel->mNumPositionKeys; ++positionKeys)
			{
				const aiVectorKey& aiKey = channel->mPositionKeys[positionKeys];
				KeyPosition key;
				key.Position = ModelLoading::AssimpLoader::GetGLMVec(aiKey.mValue);
				key.TimeStamp = static_cast<float>(aiKey.mTime);
				m_keyPositions[boneName].push_back(key);
			}

			for (u32 rotationKeys = 0; rotationKeys < channel->mNumRotationKeys; ++rotationKeys)
			{
				const aiQuatKey& aiKey = channel->mRotationKeys[rotationKeys];
				KeyRotation key;
				key.Orientation = ModelLoading::AssimpLoader::GetGLMQuat(aiKey.mValue);
				key.TimeStamp = static_cast<float>(aiKey.mTime);
				m_keyRotations[boneName].push_back(key);
			}

			for (u32 scaleKeys = 0; scaleKeys < channel->mNumScalingKeys; ++scaleKeys)
			{
				const aiVectorKey& aiKey = channel->mScalingKeys[scaleKeys];
				KeyScale key;
				key.Scale = ModelLoading::AssimpLoader::GetGLMVec(aiKey.mValue);
				key.TimeStamp = static_cast<float>(aiKey.mTime);
				m_keyScale[boneName].push_back(key);
			}

		}
	}

	void Animation::SetPlayBackSpeed(float speedInSeconds)
	{
		m_playBackSpeed = speedInSeconds;
	}

	KeyPosition Animation::GetPreviousPositionKey(const std::string& boneName, const float& animationTime)
	{
		const std::vector<KeyPosition>& positions = m_keyPositions.at(boneName);

		if (positions.size() == 1)
		{
			return positions.at(0);
		}

		for (u32 i = 0; i < positions.size() - 1; ++i)
		{
			if (animationTime < positions.at(i + 1).TimeStamp)
			{
				return positions.at(i);
			}
		}

		ASSERT(false);
		return KeyPosition();
	}
	KeyPosition Animation::GetNextPositionKey(const std::string& boneName, const float& animationTime)
	{
		const std::vector<KeyPosition>& positions = m_keyPositions.at(boneName);

		if (positions.size() == 1)
		{
			return positions.at(0);
		}

		for (u32 i = 0; i < positions.size() - 1; ++i)
		{
			if (animationTime < positions.at(i + 1).TimeStamp)
			{
				return positions.at(i + 1);
			}
		}
		ASSERT(false);
		return KeyPosition();

	}

	KeyRotation Animation::GetPreviousRotationKey(const std::string& boneName, const float& animationTime)
	{
		const std::vector<KeyRotation>& rotations = m_keyRotations.at(boneName);

		if (rotations.size() == 1)
		{
			return rotations.at(0);
		}

		for (u32 i = 0; i < rotations.size() - 1; ++i)
		{
			if (animationTime < rotations.at(i + 1).TimeStamp)
			{
				return rotations.at(i);
			}
		}
		ASSERT(false);
		return KeyRotation();
	}
	KeyRotation Animation::GetNextRotationKey(const std::string& boneName, const float& animationTime)
	{
		const std::vector<KeyRotation>& rotations = m_keyRotations.at(boneName);

		if (rotations.size() == 1)
		{
			return rotations.at(0);
		}

		for (u32 i = 0; i < rotations.size() - 1; ++i)
		{
			if (animationTime < rotations.at(i + 1).TimeStamp)
			{
				return rotations.at(i + 1);
			}
		}
		ASSERT(false);
		return KeyRotation();

	}

	KeyScale Animation::GetPreviousScaleKey(const std::string& boneName, const float& animationTime)
	{
		const std::vector<KeyScale>& scales = m_keyScale.at(boneName);

		if (scales.size() == 1)
		{
			return scales.at(0);
		}

		for (u32 i = 0; i < scales.size() - 1; ++i)
		{
			if (animationTime < scales.at(i + 1).TimeStamp)
			{
				return scales.at(i);
			}
		}
		ASSERT(false);
		return KeyScale();
	}
	KeyScale Animation::GetNextScaleKey(const std::string& boneName, const float& animationTime)
	{
		const std::vector<KeyScale>& scales = m_keyScale.at(boneName);

		if (scales.size() == 1)
		{
			return scales.at(0);
		}

		for (u32 i = 0; i < scales.size() - 1; ++i)
		{
			if (animationTime < scales.at(i + 1).TimeStamp)
			{
				return scales.at(i + 1);
			}
		}
		ASSERT(false);
		return KeyScale();
	}

	float Animation::CalculateProgressionThroughFrame(const float& animationTime, const float& previousFrame, const float& nextFrame)
	{
		if (nextFrame - previousFrame < FLOAT_EPSILON)
		{
			return 0;
		}

		float totalTime = nextFrame - previousFrame;
		float currentTime = animationTime - previousFrame;
		return currentTime / totalTime;
	}
}