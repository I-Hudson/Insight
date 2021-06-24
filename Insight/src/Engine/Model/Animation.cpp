#include "ispch.h"
#include "Engine/Model/Animation.h"
#include "Engine/Model/Model.h"
#include "Engine/Model/ModelLoading.h"

#include "assimp/scene.h"
#include "assimp/anim.h"
#include "tiny_gltf.h"

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

	Animation::Animation(const tinygltf::Model& gltfModel, u32 animationIndex, Model* model)
	{
		const tinygltf::Animation& gltfAnimation = gltfModel.animations.at(animationIndex);
		for (u32 i = 0; i < gltfAnimation.channels.size(); ++i)
		{
			const tinygltf::AnimationChannel& channel = gltfAnimation.channels.at(i);
			std::string name = gltfModel.nodes.at(channel.target_node).name;
			const tinygltf::AnimationSampler& glTFSampler = gltfAnimation.samplers.at(channel.sampler);

			const tinygltf::Accessor &  keyFrameAccessor   = gltfModel.accessors[glTFSampler.input];
			const tinygltf::BufferView& keyFrameBufferView = gltfModel.bufferViews[keyFrameAccessor.bufferView];
			const tinygltf::Buffer&     keyFrameBuffer     = gltfModel.buffers[keyFrameBufferView.buffer];
			const void*					keyFrameDataPtr    = &keyFrameBuffer.data[keyFrameAccessor.byteOffset + keyFrameBufferView.byteOffset];
			const float*				keyFrameBufferPtr  = static_cast<const float*>(keyFrameDataPtr);

			const tinygltf::Accessor&	accessor   = gltfModel.accessors[glTFSampler.output];
			const tinygltf::BufferView& bufferView = gltfModel.bufferViews[accessor.bufferView];
			const tinygltf::Buffer&		buffer     = gltfModel.buffers[bufferView.buffer];
			const void*					dataPtr    = &buffer.data[accessor.byteOffset + bufferView.byteOffset];

			for (u32 index = 0; index < keyFrameAccessor.count; ++index)
			{
				if (channel.target_path == "translation")
				{
					KeyPosition position;
					position.Position = accessor.type == TINYGLTF_TYPE_VEC3 ? glm::vec4(glm::vec4(static_cast<const glm::vec3*>(dataPtr)[index], 1.0f)) : static_cast<const glm::vec4*>(dataPtr)[index];
					position.TimeStamp = keyFrameBufferPtr[index];
					m_keyPositions[name].push_back(position);
				}
				else if (channel.target_path == "rotation")
				{
					KeyRotation rotation;
					rotation.Orientation = ModelLoading::GltfLoader::GetGlmQuat(static_cast<const glm::vec4*>(dataPtr)[index]);
					rotation.TimeStamp = keyFrameBufferPtr[index];
					m_keyRotations[name].push_back(rotation);
				}
				else if (channel.target_path == "scale")
				{
					KeyScale scale;
					scale.Scale = accessor.type == TINYGLTF_TYPE_VEC3 ? glm::vec4(glm::vec4(static_cast<const glm::vec3*>(dataPtr)[index], 1.0f)) : static_cast<const glm::vec4*>(dataPtr)[index];
					scale.TimeStamp = keyFrameBufferPtr[index];
					m_keyScale[name].push_back(scale);
				}
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