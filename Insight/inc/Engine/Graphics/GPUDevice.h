#pragma once
#include "Engine/Core/Common.h"
#include "Engine/Core/NonCopyable.h"
#include "Engine/Templates/TSingleton.h"
#include "GPUResourceCollection.h"
#include "GPULimits.h"
#include "GPUAdapter.h"
#include "Enums.h"
#include "Config.h"

namespace Insight::Graphics
{
    class GPUBuffer;
    class GPUCommandBuffer;
    class GPUCommandPool;
    class GPUImageView;
    class GPUImage;
    class GPUSampler;
    class GPUShader;
    class GPUPipeline;
    class GPUFenceManager;
    class GPUSemaphoreManager;

    using GPUDeviceSyncFuncs = std::queue<std::function<void()>>;
    using GPUSamplerCache = Insight::Graphics::GPUItemCache<Insight::Graphics::GPUSampler*>;
    using GPUShaderCache = Insight::Graphics::GPUItemCache<Insight::Graphics::GPUShader*>;
    using GPUImageCache = Insight::Graphics::GPUItemCache<Insight::Graphics::GPUImage*>;
    using GPUPipelineCache = Insight::Graphics::GPUItemCache<Insight::Graphics::GPUPipeline*>;
}

class GPUContext;
class GPUTexture;
class GPUSwapChain;

class GPUDevice : public TSingleton<GPUDevice>
{
public:
    /// <summary>
    /// Graphics Device states that describe its lifetime.
    /// </summary>
    DECLARE_ENUM_6(DeviceState, Missing, Created, Ready, Removed, Disposing, Disposed);
    struct VideoOutputMode
    {
        /// <summary>
        /// The resolution width (in pixel).
        /// </summary>
        u32 Width;

        /// <summary>
        /// The resolution height (in pixel).
        /// </summary>
        u32 Height;

        /// <summary>
        /// The screen refresh rate (in hertz).
        /// </summary>
        u32 RefreshRate;
    };

    GPUDevice(RendererType rendererType, ShaderProfile shaderProfile);
    virtual ~GPUDevice();

    void Lock() { m_locker.Lock(); }
    void Unlock() { m_locker.Unlock(); }

    virtual void BeginFrame() { }
    virtual void EndFrame() { }

    const DeviceState& GetState() const { return m_state; }
    const bool& IsRendering() const { return m_isRendering; }
    const RendererType& GetRendererType() const { return m_rendererType; }
    const ShaderProfile& GetShaderProfile() const { return m_shaderProfile; }
    const FeatureLevel& GetFeatureLevel() const { return m_featureLevel; }

    const GPULimits& GetLimits() const { return m_gpuLimits; }
    FormatFeatures GetFormatFeatures(const PixelFormat& format) const { return m_featuresPerFormat[(I32)format]; }
    
    Insight::Graphics::GPUCommandPool* GetDefaultCommandPool() const { return m_defaultCommandPool; }
    virtual Insight::Graphics::GPUFenceManager* GetDefaultFenceManager() = 0;
    virtual Insight::Graphics::GPUSemaphoreManager* GetDefaultSignalManager() = 0;

    FORCE_INLINE u64 GetMemoryusage() const { return Resources.GetMemoryUsage(); }
    
    virtual GPUContext* GetMainContext() = 0;
    virtual GPUAdapter* GetAdapter() = 0;
    virtual bool Init();
    virtual bool LoadContent();
    virtual bool CanDraw() { return true; }
    virtual void Dispose();
    virtual void WaitForGPU() = 0;
    virtual u32 GetQueueIndex(GPUQueue queue) = 0;

public:
    Insight::Graphics::GPUResourceCollection Resources;

protected:
    CriticalSection m_locker;

    u64 m_totalGraphicsMemory;
    GPULimits m_gpuLimits;
    std::vector<VideoOutputMode> m_videoOutputModes;

    FormatFeatures m_featuresPerFormat[static_cast<I32>(PixelFormat::MAX)];

    DeviceState m_state;
    bool m_isRendering;
    RendererType m_rendererType;
    ShaderProfile m_shaderProfile;
    FeatureLevel m_featureLevel;

    Insight::Graphics::GPUCommandPool* m_defaultCommandPool;

    Insight::Graphics::GPUSamplerCache m_samplerCache;
    Insight::Graphics::GPUShaderCache m_shaderCache;
    Insight::Graphics::GPUImageCache m_imageCache;
    Insight::Graphics::GPUPipelineCache m_pipelineCache;

    struct DefaultData;
    DefaultData* m_defaultData;
};

/// <summary>
/// Utility structure to safety graphics device locking.
/// </summary>
struct GPUDeviceLock : NonCopyable
{
    GPUDevice* Device;

    GPUDeviceLock(GPUDevice* device)
        : Device(device)
    {
        Device->Lock();
    }

    ~GPUDeviceLock()
    {
        Device->Unlock();
    }
};
