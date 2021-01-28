#pragma once

#include "Engine/Graphics/GPUTimerQuery.h"
#include "GPUDeviceVulkan.h"

class GPUTimerQueryVulkan : public GPUResourceVulkan<GPUTimerQuery>
{
public:
    /// <summary>
    /// Initializes a new instance of the <see cref="GPUTimerQueryVulkan"/> class.
    /// </summary>
    /// <param name="device">The graphics device.</param>
    GPUTimerQueryVulkan(GPUDeviceVulkan* device);

    /// <summary>
    /// Interrupts an in-progress query, allowing the command buffer to submitted. Interrupted queries must be resumed using Resume().
    /// </summary>
    /// <param name="cmdBuffer">The GPU commands buffer.</param>
    void Interrupt(CmdBufferVulkan* cmdBuffer);

    /// <summary>
    /// Resumes an interrupted query, restoring it back to its original in-progress state.
    /// </summary>
    /// <param name="cmdBuffer">The GPU commands buffer.</param>
    void Resume(CmdBufferVulkan* cmdBuffer);

    // [GPUTimerQuery]
    void Begin() override;
    void End() override;

    bool HasResult() override
    {
        if (!_endCalled)
            return false;
        if (_hasResult)
            return true;

        return TryGetResult();
    }

    float GetResult() override
    {
        if (_hasResult)
        {
            return _timeDelta;
        }

        TryGetResult();

        return _timeDelta;
    }

protected:  
    // [GPUResourceVulkan]
    void OnReleaseGPU() override;

private:
    struct Query
    {
        BufferedQueryPoolVulkan* Pool;
        U32 Index;
        U64 Result;
    };

    struct QueryPair
    {
        Query Begin;
        Query End;
    };

    bool GetResult(Query& query);
    void WriteTimestamp(CmdBufferVulkan* cmdBuffer, Query& query) const;
    bool TryGetResult();
    bool UseQueries();

private:
    bool _hasResult = false;
    bool _endCalled = false;
    bool _interrupted = false;
    float _timeDelta = 0.0f;
    I32 _queryIndex;
    std::vector<QueryPair> _queries;
};

