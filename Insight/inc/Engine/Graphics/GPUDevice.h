#pragma once
#include "Engine/Core/Common.h"
#include "Engine/Core/NonCopyable.h"
#include "Engine/Templates/TSingleton.h"
#include "GPUResourceCollection.h"
#include "GPULimits.h"
#include "GPUAdapter.h"
#include "Enums.h"
#include "Config.h"
#include <queue>

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
    using GPUSamplerCache = GPUItemCache<GPUSampler*>;
    using GPUShaderCache = GPUItemCache<GPUShader*>;
    using GPUImageCache = GPUItemCache<GPUImage*>;
    using GPUImageViewCache = GPUItemCache<GPUImageView*>;
    using GPUPipelineCache = GPUItemCache<GPUPipeline*>;

    class GPUContext;
    class GPUTexture;
    class GPUSwapChain;

    class GPUDevice : public Core::TSingleton<GPUDevice>
    {
    public:
        /// <summary>
        /// Graphics Device states that describe its lifetime.
        /// </summary>
        DECLARE_ENUM_6(DeviceState, Missing, Created, Ready, Removed, Disposing, Disposed);

        static GPUDevice* New();

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
        virtual bool HasExtension(GPUDeviceExtension ext) = 0;

        GPUCommandPool* GetDefaultCommandPool() const { return m_defaultCommandPool; }
        virtual GPUFenceManager* GetDefaultFenceManager() = 0;
        virtual GPUSemaphoreManager* GetDefaultSignalManager() = 0;

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
        GPUResourceCollection Resources;

    protected:
        CriticalSection m_locker;

        u64 m_totalGraphicsMemory;
        GPULimits m_gpuLimits;

        FormatFeatures m_featuresPerFormat[static_cast<I32>(PixelFormat::MAX)];

        DeviceState m_state;
        bool m_isRendering;
        RendererType m_rendererType;
        ShaderProfile m_shaderProfile;
        FeatureLevel m_featureLevel;

        GPUCommandPool* m_defaultCommandPool;

        GPUSamplerCache m_samplerCache;
        GPUShaderCache m_shaderCache;
        GPUImageCache m_imageCache;
        GPUImageViewCache m_imageViewCache;
        GPUPipelineCache m_pipelineCache;
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
}