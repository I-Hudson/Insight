#pragma once

#include "Engine/Core/Common.h"
#include "Engine/Graphics/GPUBuffer.h"
#include "Engine/Component/CameraComponent.h"

namespace Insight::Graphics::Debug
{
	class Gizmos : public Core::TSingleton<Gizmos>
	{
	public:
		Gizmos();
		~Gizmos();

		void DrawGizmos(CameraComponent& cameraComponent);
	private:

		GPUBuffer* m_boxVBuffer;
		GPUBuffer* m_boxIBuffer;
	};
}