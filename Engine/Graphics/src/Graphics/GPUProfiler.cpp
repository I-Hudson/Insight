#include "Graphics/GPUProfiler.h"
#include "Graphics/RenderContext.h"

#include <imgui.h>

namespace Insight
{
    //-------------------------------
    // GPUProfileFrame
    //-------------------------------
    GPUProfileFrame::GPUProfileFrame()
    {
    }

    GPUProfileFrame::GPUProfileFrame(const GPUProfileFrame& other)
    {
        *this = other;
    }

    GPUProfileFrame::GPUProfileFrame(GPUProfileFrame&& other)
    {
        *this = std::move(other);
    }

    GPUProfileFrame::~GPUProfileFrame()
    {
        Clear();
    }

    GPUProfileFrame& GPUProfileFrame::operator=(const GPUProfileFrame& other)
    {
        m_nodes = other.m_nodes;
        m_activeNodeIndexes = other.m_activeNodeIndexes;
        m_renderContext = other.m_renderContext;
        return *this;
    }

    GPUProfileFrame& GPUProfileFrame::operator=(GPUProfileFrame&& other)
    {
        m_nodes = std::move(other.m_nodes);
        m_activeNodeIndexes = std::move(other.m_activeNodeIndexes);
        m_renderContext = std::move(other.m_renderContext);

        other.m_nodes = {};
        other.m_activeNodeIndexes = {};
        other.m_renderContext = nullptr;
        return *this;
    }

    void GPUProfileFrame::Initialise(Graphics::RenderContext* renderContext)
    {
        m_renderContext = renderContext;
    }

    void GPUProfileFrame::Resolve(Graphics::RHI_CommandList* cmdList)
    {
        std::lock_guard lock(m_mutex);
        std::vector<u64> queryTimestamps = m_renderContext->ResolveTimeStamps(cmdList);

        if (queryTimestamps.size() == 0)
        {
            return;
        }

        const u64 timeStampFrequency = m_renderContext->GetTimeStampFrequency();
        const u64 totalQueryTimeStamps = queryTimestamps.size();
        const u64 nodeQueryTimeStamps = totalQueryTimeStamps / 2;
        if (nodeQueryTimeStamps != m_nodes.size())
        {
            return;
        }

        for (size_t i = 0; i < m_nodes.size(); ++i)
        {
            GPUProfilerNode& node = m_nodes.at(i);

            const u64 startTimeStamp = queryTimestamps.at(node.StartIndex);
            const u64 endTimeStamp = queryTimestamps.at(node.EndIndex);

            node.GPUSample = endTimeStamp - startTimeStamp;
            double dFrequency = (double)timeStampFrequency;
            node.GPUSampleMS = (node.GPUSample / dFrequency) * 1000.0;
        }
    }

    void GPUProfileFrame::Clear()
    {
        std::lock_guard lock(m_mutex);
        m_currentIndex = 0;
        m_nodes.clear();
    }

    void GPUProfileFrame::StartProfile(Graphics::RHI_CommandList* cmdList, std::string name)
    {
        std::lock_guard lock(m_mutex);
        m_renderContext->MarkTimeStamp(cmdList);

        u32 topIndex = static_cast<u32>(m_nodes.size());
        m_activeNodeIndexes.push(topIndex);

        GPUProfilerNode node;
        node.Name = std::move(name);
        node.State = GPUProfilerNode::State::Started;
        node.StartIndex = m_currentIndex++;
        m_nodes.push_back(node);
    }

    void GPUProfileFrame::EndProfile(Graphics::RHI_CommandList* cmdList)
    {
        std::lock_guard lock(m_mutex);
        m_renderContext->MarkTimeStamp(cmdList);

        u32 topIndex = m_activeNodeIndexes.top();
        m_activeNodeIndexes.pop();

        GPUProfilerNode& node = m_nodes.at(topIndex);
        node.State = GPUProfilerNode::State::Ended;
        node.EndIndex = m_currentIndex++;
    }

    void GPUProfileFrame::Draw() const
    {
        std::lock_guard lock(m_mutex);
        if (ImGui::Begin("GPUProfiler"))
        {
            for (const GPUProfilerNode& node : m_nodes)
            {
                ImGui::Text("%s", node.Name.c_str());
                ImGui::Indent();
                ImGui::Text("%lf", node.GPUSampleMS);
                ImGui::Unindent();
                ImGui::Spacing();
                ImGui::Spacing();
            }
        }
        ImGui::End();
    }

    //-------------------------------
    // GPUProfiler
    //-------------------------------
    GPUProfiler::GPUProfiler()
    {
    }

    GPUProfiler::~GPUProfiler()
    {
    }

    void GPUProfiler::Initialise(Graphics::RenderContext* renderContext)
    {
        m_renderContext = renderContext;
        m_profileFrames.GetCurrent().Initialise(m_renderContext);
        m_profileFrames.Swap();
        m_profileFrames.GetCurrent().Initialise(m_renderContext);
        m_profileFrames.Swap();
    }

    void GPUProfiler::Shutdown()
    {
        m_profileFrames.GetCurrent().Clear();
        m_profileFrames.Swap();
        m_profileFrames.GetCurrent().Clear();
    }

    void GPUProfiler::StartProfile(Graphics::RHI_CommandList* cmdList, std::string name)
    {
        m_profileFrames.GetCurrent().StartProfile(cmdList, std::move(name));
    }

    void GPUProfiler::EndProfile(Graphics::RHI_CommandList* cmdList)
    {
        m_profileFrames.GetCurrent().EndProfile(cmdList);
    }

    void GPUProfiler::BeginFrame(Graphics::RHI_CommandList* cmdList)
    {
        StartProfile(cmdList, "GPUFrame");
    }

    void GPUProfiler::EndFrame(Graphics::RHI_CommandList* cmdList)
    {
        EndProfile(cmdList);
        m_profileFrames.GetCurrent().Resolve(cmdList);
        m_resolveGPUFrame = m_profileFrames.GetCurrent();
        m_profileFrames.Swap();
        m_profileFrames.GetCurrent().Clear();
    }

    GPUProfileFrame GPUProfiler::GetFrameData() const
    {
        return m_resolveGPUFrame;
    }
}