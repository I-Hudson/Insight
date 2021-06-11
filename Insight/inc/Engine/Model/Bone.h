#pragma once

#include "Engine/Core/Common.h"
#include "glm/glm.hpp"
#include "glm/gtc/quaternion.hpp"

struct aiNodeAnim;

namespace Insight::Animation
{
	struct BoneInfo
	{
		/*id is index in finalBoneMatrices*/
		u32 Id;
		/*offset matrix transforms vertex from model space to bone space*/
		glm::mat4 Offset;
	};

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

	class Bone
	{
	public:
		Bone(std::string name, u32 ID, const aiNodeAnim* channel);

		void Update(const float& animationTime);
		glm::mat4 GetLocalTransform() const { return m_localTransform; }
		std::string GetBoneName() const { return m_name; }
		u32 GetBoneId() const { return m_ID; }

		u32 GetPositionIndex(const float& animationTime);
		u32 GetRotationIndex(const float& animationTime);
		u32 GetScaleIndex(const float& animationTime);

	private:
		float GetScaleFactor(const float& lastTimeStamp, const float& nextTimeStamp, const float& animationTime);

		glm::mat4 InterpolatePosition(float animationTime);
		glm::mat4 InterpolateRotation(float animationTime);
		glm::mat4 InterpolateScaling(float animationTime);

	private:
		std::vector<KeyPosition> m_positions;
		std::vector<KeyRotation> m_rotations;
		std::vector<KeyScale> m_scales;
		u32 m_numPositions;
		u32 m_numRotations;
		u32 m_numScalings;

		glm::mat4 m_localTransform;
		std::string m_name;
		u32 m_ID;
	};
}