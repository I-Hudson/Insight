#pragma once

#include "Insight/Core.h"
#include "Insight/Object.h"

namespace Insight
{
	class LightComponent;
	class MeshComponent;

	class RenderPass : public Object
	{
		virtual ~RenderPass() { }

		virtual void Setup() = 0;
		virtual void Render() = 0;

		virtual void SetLights(const std::vector<LightComponent*>& lights) = 0;
		virtual void SetMeshes(const std::vector<MeshComponent*>& meshes) = 0;

		static TSafePtr<RenderPass> Create();
	};
}