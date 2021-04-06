#pragma once
#include "Engine/Core/Common.h"
#include "Engine/Core/NonCopyable.h"
#include "Engine/Templates/TSingleton.h"
#include "GPUResourceCollection.h"
#include "GPULimits.h"
#include "GPUAdapter.h"
#include "Enums.h"
#include "Config.h"

class GPUContext;
class GPUTexture;
class GPUBuffer;
class GPUSwapChain;
class GPUShader;
class GPUImageView;
class GPUImage;

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
        U32 Width;

        /// <summary>
        /// The resolution height (in pixel).
        /// </summary>
        U32 Height;

        /// <summary>
        /// The screen refresh rate (in hertz).
        /// </summary>
        U32 RefreshRate;
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
    const 

    FORCE_INLINE U64 GetMemoryusage() const { return Resources.GetMemoryUsage(); }
    
    virtual GPUContext* GetMainContext() = 0;
    virtual GPUAdapter* GetAdapter() = 0;
    virtual bool Init();
    virtual bool LoadContent();
    virtual bool CanDraw() { return true; }
    virtual void Dispose();
    virtual void WaitForGPU() = 0;

    virtual GPUImageView* GetTransientAttachment(U32 width, U32 height, PixelFormat format, U32 index, U32 samples, U32 layers) = 0;

public:
    GPUResourceCollection Resources;

protected:
    CriticalSection m_locker;

    U64 m_totalGraphicsMemory;
    GPULimits m_gpuLimits;
    std::vector<VideoOutputMode> m_videoOutputModes;

    FormatFeatures m_featuresPerFormat[static_cast<I32>(PixelFormat::MAX)];

    DeviceState m_state;
    bool m_isRendering;
    RendererType m_rendererType;
    ShaderProfile m_shaderProfile;
    FeatureLevel m_featureLevel;

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
