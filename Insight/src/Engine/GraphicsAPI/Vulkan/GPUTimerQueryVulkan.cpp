#include "ispch.h"
#include "GPUTimerQueryVulkan.h"
#include "CmdBufferVulkan.h"

GPUTimerQueryVulkan::GPUTimerQueryVulkan(GPUDeviceVulkan* device)
	: GPUResourceVulkan<GPUTimerQuery>(device, "")
{
}

void GPUTimerQueryVulkan::Interrupt(CmdBufferVulkan* cmdBuffer)
{
	if (!_interrupted)
	{
		_interrupted = true;
		WriteTimestamp(cmdBuffer, _queries[_queryIndex].End);
	}
}

void GPUTimerQueryVulkan::Resume(CmdBufferVulkan* cmdBuffer)
{
	ASSERT(_interrupted);

	QueryPair e;
	e.End.Pool = nullptr;

	_interrupted = false;
	WriteTimestamp(cmdBuffer, e.Begin);

	_queries.push_back(e);
	_queryIndex++;
}

void GPUTimerQueryVulkan::Begin()
{
    if (UseQueries())
    {
        const auto context = (GPUContextVulkan*)m_device->GetMainContext();
        const auto cmdBuffer = context->GetCmdBufferManager()->GetCmdBuffer();

        QueryPair e;
        e.End.Pool = nullptr;

        _queryIndex = 0;
        _interrupted = false;
        WriteTimestamp(cmdBuffer, e.Begin);
        context->GetCmdBufferManager()->OnQueryBegin(this);

        ASSERT(_queries.empty());
        _queries.push_back(e);
    }

    _hasResult = false;
    _endCalled = false;
}

void GPUTimerQueryVulkan::End()
{
    if (_endCalled)
        return;

    if (UseQueries())
    {
        const auto context = (GPUContextVulkan*)m_device->GetMainContext();
        const auto cmdBuffer = context->GetCmdBufferManager()->GetCmdBuffer();

        if (!_interrupted)
        {
            WriteTimestamp(cmdBuffer, _queries[_queryIndex].End);
        }
        context->GetCmdBufferManager()->OnQueryEnd(this);
    }

    _endCalled = true;
}

void GPUTimerQueryVulkan::OnReleaseGPU()
{
    _hasResult = false;
    _endCalled = false;
    _timeDelta = 0.0f;

    for (I32 i = 0; i < _queries.size(); i++)
    {
        auto& e = _queries[i];

        if (e.Begin.Pool)
        {
            e.Begin.Pool->ReleaseQuery(e.Begin.Index);
        }

        if (e.End.Pool)
        {
            e.End.Pool->ReleaseQuery(e.End.Index);
        }
    }
    _queries.clear();
}

bool GPUTimerQueryVulkan::GetResult(Query& query)
{
    if (query.Pool)
    {
        const auto context = (GPUContextVulkan*)m_device->GetMainContext();
        if (query.Pool->GetResults(context, query.Index, query.Result))
        {
            // Release query
            query.Pool->ReleaseQuery(query.Index);
            query.Pool = nullptr;
        }
        else
        {
            // No results
            return true;
        }
    }

    // Has result
    return false;
}

void GPUTimerQueryVulkan::WriteTimestamp(CmdBufferVulkan* cmdBuffer, Query& query) const
{
    auto pool = m_device->FindAvailableTimestampQueryPool();
    U32 index;
    pool->AcquireQuery(index);

    vkCmdWriteTimestamp(cmdBuffer->GetHandle(), VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, pool->GetHandle(), index);
    pool->MarkQueryAsStarted(index);

    query.Pool = pool;
    query.Index = index;
}

bool GPUTimerQueryVulkan::TryGetResult()
{
    // Try get queries value (if not already)
    for (I32 i = 0; i < _queries.size(); i++)
    {
        auto& e = _queries[i];

        if (GetResult(e.Begin))
            return false;
        if (GetResult(e.End))
            return false;
    }

    // Calculate delta time (accumulated)
    U64 delta = 0;
    for (I32 i = 0; i < _queries.size(); i++)
    {
        auto& e = _queries[i];

        if (e.End.Result > e.Begin.Result)
        {
            delta += e.End.Result - e.Begin.Result;
        }
    }

    // Calculate event duration in milliseconds
    const double frequency = double(m_device->PhysicalDeviceLimits.timestampPeriod) / 1e6;
    _timeDelta = static_cast<float>((delta * frequency));

    // Clear data for next usage
    _hasResult = true;
    for (I32 i = 0; i < _queries.size(); i++)
    {
        auto& e = _queries[i];

        if (e.Begin.Pool)
        {
            e.Begin.Pool->ReleaseQuery(e.Begin.Index);
        }

        if (e.End.Pool)
        {
            e.End.Pool->ReleaseQuery(e.End.Index);
        }
    }
    _queries.clear();

    return true;
}

bool GPUTimerQueryVulkan::UseQueries()
{
    return m_device->PhysicalDeviceLimits.timestampComputeAndGraphics == VK_TRUE;
}
