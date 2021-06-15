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
	struct KeyPosition
	{
		glm::vec3 Position;
		float TimeStamp;
	};

	struct KeyRotation
	{
		glm::quat Orientation;
		float TimeStamp;
	};

	struct KeyScale
	{
		glm::vec3 Scale;
		float TimeStamp;
	};

	class Animation
	{
	public:
		Animation() = default;
		Animation(const aiScene* aiScene , u32 animationIndex, Model* model);

		INLINE float GetTicksPerSecond() const { return m_ticksPerSecond; }
		INLINE float GetDuration() const { return m_duration; }

		KeyPosition GetPreviousPositionKey(const std::string& boneName, const float& animationTime);
		KeyPosition GetNextPositionKey(const std::string& boneName, const float& animationTime);

		KeyRotation GetPreviousRotationKey(const std::string& boneName, const float& animationTime);
		KeyRotation GetNextRotationKey(const std::string& boneName, const float& animationTime);

		KeyScale GetPreviousScaleKey(const std::string& boneName, const float& animationTime);
		KeyScale GetNextScaleKey(const std::string& boneName, const float& animationTime);

		float CalculateProgressionThroughFrame(const float& animationTime, const float& previousFrame, const float& nextFrame);

	private:
		float m_ticksPerSecond;
		float m_duration;
		std::unordered_map<std::string, std::vector<KeyPosition>> m_keyPositions;
		std::unordered_map<std::string, std::vector<KeyRotation>> m_keyRotations;
		std::unordered_map<std::string, std::vector<KeyScale>> m_keyScale;
	};
}