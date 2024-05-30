#pragma once

#include "Runtime/Defines.h"
#include "Graphics/RenderContext.h"
#include "Physics/DebugRendererData.h"

#include "Maths/Matrix4.h"

namespace Insight
{
    namespace Graphics
    {
        class RHI_Buffer;
        class RHI_Texture;

        class IS_RUNTIME PhysicsDebugRenderPass
        {
        public:

            struct IS_RUNTIME ConstantBuffer
            {
                Maths::Matrix4 View;
                Maths::Matrix4 Project;
                Maths::Matrix4 LightView;
                Maths::Matrix4 LightProjection;
            };

            void Create();
            void Render(ConstantBuffer constantBuffer, std::string_view colourTextureName, std::string_view depthTextureName);
            void Destroy();

        private:
            FrameResource<RHI_Buffer*> m_vertex_buffer;
            FrameResource<RHI_Buffer*> m_index_buffer;
        };
    }
}