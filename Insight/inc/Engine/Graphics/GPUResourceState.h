#pragma once

#include "Engine/Platform/Platform.h"

/// <summary>
/// Tracking of per-resource or per-subresource state for GPU resources that require to issue resource access barriers during rendering.
/// </summary>
template<typename StateType, StateType InvalidState>
class GPUResourceState
{
private:

    /// <summary>
    /// The whole resource state (used only if m_allSubresourcesSame is 1).
    /// </summary>
    StateType m_resourceState : 31;

    /// <summary>
    /// Set to 1 if m_resourceState is valid. In this case, all subresources have the same state.
    /// Set to 0 if m_subresourceState is valid. In this case, each subresources may have a different state (or may be unknown).
    /// </summary>
    U32 m_allSubresourcesSame : 1;

    /// <summary>
    /// The per subresource state (used only if m_allSubresourcesSame is 0).
    /// </summary>
    std::vector<StateType> m_subresourceState;

public:

    /// <summary>
    /// Initializes a new instance of the <see cref="GPUResourceState"/> class.
    /// </summary>
    GPUResourceState()
        : m_resourceState(InvalidState)
        , m_allSubresourcesSame(true)
    {
    }

public:

    void Initialize(U32 subresourceCount, StateType initialState, bool usePerSubresourceTracking)
    {
        ASSERT(m_subresourceState.empty() && subresourceCount > 0);

        // Initialize state
        m_allSubresourcesSame = true;
        m_resourceState = initialState;

        // Allocate space for per-subresource state tracking
        if (usePerSubresourceTracking && subresourceCount > 1)
            m_subresourceState.clear();
            m_subresourceState.resize(subresourceCount);
#if IS_DEBUG
        std::fill(m_subresourceState.begin(), m_subresourceState.end(), InvalidState);
#endif
    }

    bool IsInitializated() const
    {
        return m_resourceState != InvalidState || m_subresourceState.empty();
    }

    void Release()
    {
        m_resourceState = InvalidState;
        m_subresourceState.resize(0);
    }

    bool AreAllSubresourcesSame() const
    {
        return m_allSubresourcesSame;
    }

    I32 GetSubresourcesCount() const
    {
        return m_subresourceState.size();
    }

    bool CheckResourceState(StateType state) const
    {
        if (m_allSubresourcesSame)
        {
            return state == m_resourceState;
        }

        // Check all subresources
        for (I32 i = 0; i < m_subresourceState.size(); ++i)
        {
            if (m_subresourceState[i] != state)
            {
                return false;
            }
        }

        return true;
    }

    StateType GetSubresourceState(U32 subresourceIndex) const
    {
        if (m_allSubresourcesSame)
        {
            return m_resourceState;
        }

        ASSERT(subresourceIndex >= 0 && subresourceIndex < static_cast<U32>(m_subresourceState.size()));
        return m_subresourceState[subresourceIndex];
    }

    void SetResourceState(StateType state)
    {
        m_allSubresourcesSame = 1;
        m_resourceState = state;

#if IS_DEBUG
        for (I32 i = 0; i < m_subresourceState.size(); ++i)
        {
            m_subresourceState[i] = InvalidState;
        }
#endif
    }

    void SetSubresourceState(I32 subresourceIndex, StateType state)
    {
        // Check if use single state for the whole resource
        if (subresourceIndex == -1 || m_subresourceState.size() <= 1)
        {
            SetResourceState(state);
        }
        else
        {
            ASSERT(subresourceIndex < static_cast<I32>(m_subresourceState.size()));

            // Transition for all sub-resources
            if (m_allSubresourcesSame)
            {
                for (I32 i = 0; i < m_subresourceState.size(); ++i)
                {
                    m_subresourceState[i] = m_resourceState;
                }
                m_allSubresourcesSame = 0;
#if IS_DEBUG
                m_resourceState = InvalidState;
#endif
            }

            m_subresourceState[subresourceIndex] = state;
        }
    }
};