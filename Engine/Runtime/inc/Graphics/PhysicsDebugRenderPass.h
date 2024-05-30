#pragma once

#include "Graphics/RenderContext.h"
#include "Physics/DebugRendererData.h"

namespace Insight
{
    namespace Graphics
    {
        class RHI_Buffer;
        class RHI_Texture;

        class PhysicsDebugRenderPass
        {
        public:

            void Create();
            void Render();
            void Destroy();

        private:
            FrameResource<RHI_Buffer*> m_vertex_buffer;
            FrameResource<RHI_Buffer*> m_index_buffer;
        };
    }
}