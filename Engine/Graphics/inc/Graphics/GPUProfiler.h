#pragma once

#include "Graphics/Defines.h"

#include "Core/TypeAlias.h"
#include "Core/Collections/DoubleBufferVector.h"
#include "Core/Singleton.h"

#include <string>
#include <vector>
#include <stack>
#include <mutex>

namespace Insight
{
    namespace Graphics
    {
        class RenderContext;
        class RHI_CommandList;
    }

    struct GPUProfilerNode
    {
        enum class State
        {
            None,
            Started,
            Ended
        };

        std::string Name;
        State State = State::None;

        u64 StartIndex = 0;
        u64 EndIndex = 0;
        u64 GPUSample = 0;
        double GPUSampleMS = 0;
    };

    class IS_GRAPHICS GPUProfileFrame
    {
    public:
        GPUProfileFrame();
        GPUProfileFrame(const GPUProfileFrame& other);
        GPUProfileFrame(GPUProfileFrame&& other);
        ~GPUProfileFrame();

        GPUProfileFrame& operator=(const GPUProfileFrame& other);
        GPUProfileFrame& operator=(GPUProfileFrame&& other);

        void Initialise(Graphics::RenderContext* renderContext);

        void Resolve(Graphics::RHI_CommandList* cmdList);
        void Clear();

        void StartProfile(Graphics::RHI_CommandList* cmdList, std::string name);
        void EndProfile(Graphics::RHI_CommandList* cmdList);

        void Draw() const;

    private:
        std::vector<GPUProfilerNode> m_nodes;
        std::stack<u32> m_activeNodeIndexes;
        mutable std::mutex m_mutex;
        u64 m_currentIndex = 0;
        Graphics::RenderContext* m_renderContext = nullptr;
    };

    class IS_GRAPHICS GPUProfiler : public Core::Singleton<GPUProfiler>
    {
    public:
        GPUProfiler();
        ~GPUProfiler();

        void Initialise(Graphics::RenderContext* renderContext);
        void Shutdown();

        void StartProfile(Graphics::RHI_CommandList* cmdList, std::string name);
        void EndProfile(Graphics::RHI_CommandList* cmdList);

        void BeginFrame(Graphics::RHI_CommandList* cmdList);
        void EndFrame(Graphics::RHI_CommandList* cmdList);

        /// @brief Gets the previous frames data.
        GPUProfileFrame GetFrameData() const;

    private:
        DoubleBufferVector<GPUProfileFrame> m_profileFrames;
        GPUProfileFrame m_resolveGPUFrame;
        Graphics::RenderContext* m_renderContext = nullptr;
    };
}