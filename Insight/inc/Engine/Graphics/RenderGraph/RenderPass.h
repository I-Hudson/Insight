#pragma once

#include "Engine/Core/Common.h"
#include "Engine/Graphics/PixelFormat.h"
#include "Engine/Graphics/Enums.h"
#include "Engine/Core/Maths/Rect.h"
#include "Engine/Utils/Hasher.h"

#include "Engine/Graphics/GPUBufferDesc.h"

#include <functional>
#include "glm/glm.hpp"


namespace Insight::Graphics
{
	class GPUDynamicBuffer;
	class GPUDescriptorBuilder;
	class GPUDescriptorAllocator;
	class GPUImage;
	class GPUImageView;
	class GPUCommandBuffer;

	class GPURenderGraphPass;
	class RenderPass;
	class Texture;
	class RenderGraph;

	using FrameBufferResources = std::unordered_map<GPUBufferFlags, GPUDynamicBuffer*>;

	using RenderPassBeginRenderFunc = std::function<void(RenderPass*, GPURenderGraphPass*)>;
	using RenderPassRenderFunc = std::function<void(GPUCommandBuffer*, FrameBufferResources&, GPUDescriptorBuilder*, RenderPass&)>;
	using RenderPassEndRenderFunc = std::function<void(RenderPass*)>;

	using RenderPassClearColourFunc = std::function<void(u32, glm::vec4&)>;
	using RenderPassClearDepthStencilFunc = std::function<void(glm::vec2&)>;

	enum RenderGraphQueueFlagsBits
	{
		RENDER_GRAPH_QUEUE_GRAPHICS_BIT = 1 << 0,
		RENDER_GRAPH_QUEUE_COMPUTE_BIT = 1 << 1,
		RENDER_GRAPH_QUEUE_ASYNC_COMPUTE_BIT = 1 << 2,
		RENDER_GRAPH_QUEUE_ASYNC_GRAPHICS_BIT = 1 << 3,
	};
	using RenderGraphQueueFlags = u32;

	static const RenderGraphQueueFlags ComputeQueues = RENDER_GRAPH_QUEUE_COMPUTE_BIT |
		RENDER_GRAPH_QUEUE_ASYNC_COMPUTE_BIT;

	class RenderPassLifeTimeObject
	{
	public:
		RenderPassLifeTimeObject(void* objectPtr);
		RenderPassLifeTimeObject(const RenderPassLifeTimeObject& other);
		RenderPassLifeTimeObject(RenderPassLifeTimeObject&& other);
		~RenderPassLifeTimeObject();

		RenderPassLifeTimeObject& operator= (const RenderPassLifeTimeObject& other);
		RenderPassLifeTimeObject& operator= (RenderPassLifeTimeObject&& other);
	private:
		void* m_objectPtr;
		u32* m_refCount;
	};

	enum RenderPassQueue
	{
		Invalid = 0,
		Default = 1,
		UI = 2,
		Overlay = 3,

		Count
	};

	/// <summary>
	/// Define an image attachment
	/// </summary>
	struct ImageAttachmentInfo
	{
		PixelFormat Format = PixelFormat::Unknown;
		std::string Name = "";
		u32 Width = 0;
		u32 Height = 0;
		u32 Depth = 1;
		SampleLevel Samples = SampleLevel::None;
		u32 Levels = 1;
		u32 Layers = 1;
		bool AutoSizeToWindow = true;
		bool Persistent = true;
		bool UnormSRGBAlias = false;
		bool SupportsPrerotate = false;
		bool SwapchainImage = false;
	};

	struct BufferAttachmentInfo
	{
		u64 Size;
		ShaderStage ShaderStages;
	};

	struct TextureGraphResourceInfo
	{
		const ImageAttachmentInfo& GetAttachmentInfo() const { return m_info; }
		void SetAttachmentInfo(const ImageAttachmentInfo& info) { m_info = info; }
		void AddImageUsage(const ImageUsageFlags& imageUsage) { m_usageFlags |= imageUsage; }
		const ImageUsageFlags& GetImageUsage() const { return m_usageFlags; }

		void SetTransientState(bool enable) { m_transient = enable; }
		const bool& GetTransientState() const { return m_transient; }

		ImageAttachmentInfo m_info = {};
		ImageUsageFlags m_usageFlags = 0;
		ImageLayout ImageLayout;
		bool m_transient = false;
	};

	struct BufferGraphResourceInfo
	{
		u64 Size;
		GPUBufferFlags Usage;
	};

	struct AccessedResouce
	{
		PipelineStageFlags Stages = 0;
		AccessFlags Access = 0;
		ImageLayoutFlags Layout = 0;
	};

	class RenderGraphResource
	{
	public:
		enum class Type
		{
			Buffer,
			Texture
		};

		enum { Unused = -1 };

		RenderGraphResource(const Type& type, u32 index, const std::string& name)
			: m_type(type)
			, m_index(index)
			, m_name(name)
			, m_physicalIndex(Unused)
		{ }

		const Type& GetType() const { return m_type; }
		const u32& GetIndex() const { return m_index; }
		const u32& GetPhysicalIndex() const { return m_physicalIndex; }
		void SetPhysicalIndex(const u32& physicalIndex) { m_physicalIndex = physicalIndex; }
		void SetPassName(const std::string& passName) { m_passName = passName; }

		void AddQueue(const RenderGraphQueueFlags& flags) { m_usedQueues |= flags; }
		void AddWrittenInPass(const u32& pass) { m_writtemInPasses.insert(pass); }
		void AddReadInPass(const u32& pass) { m_readInPasses.insert(pass); }

		const std::unordered_set<u32>& GetWritePasses() const { return m_writtemInPasses; }
		const std::unordered_set<u32>& GetReadPasses() const { return m_readInPasses; }

		const std::string& GetName() const { return m_name; }
		const std::string& GetPassName() const { return m_passName; }
		std::string GetFullName() const { return m_passName + ": " + m_name; }
		const RenderGraphQueueFlags& GetUsedQueues() const { return m_usedQueues; }

	private:
		Type m_type;
		u32 m_index;
		u32 m_physicalIndex;
		std::unordered_set<u32> m_writtemInPasses;
		std::unordered_set<u32> m_readInPasses;
		std::string m_name;
		std::string m_passName;
		RenderGraphQueueFlags m_usedQueues = 0;

	public:
		AccessedResouce AccessedResource;
		BufferGraphResourceInfo BufferInfo;
		TextureGraphResourceInfo TextureInfo;
	};

	class RenderPass
	{
	public:
		explicit RenderPass(RenderGraph* graph, const u32& index, const std::string name,
							RenderGraphQueueFlags queue);
		~RenderPass();

		RenderGraph* GetGraph() const { return m_graph; }
		const u32& GetPassIndex() const { return m_passIndex; }
		const u32& GetOrderedPassIndex() const { return m_orderedPassIndex; }
		void SetPhysicalPassIndex(const u32& index) { m_orderedPassIndex = index; }
		
		void SetGPUGraphPass(GPURenderGraphPass* pass) { m_graphPass = pass; }
		GPURenderGraphPass* GetGraphPass() const { return m_graphPass; }

		const RenderGraphQueueFlags& GetQueue()const { return m_queue; }
		const std::string& GetPassName() const { return m_name; }

		RenderGraphResource& AddColorOutput(const std::string& name, ImageAttachmentInfo& attachment);
		RenderGraphResource& AddColorOutput(const std::string& name);
		RenderGraphResource& AddColorInput(const std::string& name);
		RenderGraphResource& SetDepthStencilOutput(const std::string& name, ImageAttachmentInfo& attachment);

		//TODO: Used for tile rendering. Currently not supported.
		RenderGraphResource& AddAttachmentInput(const std::string& name);

		RenderGraphResource& SetDepthStencilInput(const std::string& name);

		void SetPassQueue(RenderPassQueue passQueue) { m_passQueue = passQueue; }
		void SetRenderFunc(RenderPassRenderFunc func) { m_renderFunc = func; }
		void OnBeginRender(RenderPassBeginRenderFunc func) { m_beginRenderFunc = func; }
		void OnEndRender(RenderPassEndRenderFunc func) { m_endRenderFunc = func; }

		void SetClearColour(const glm::vec4& clearColour) { m_clearColour = clearColour; }
		void SetClearDepthStencil(const glm::vec2& clearDepthStencil) { m_clearDepthStencil = clearDepthStencil; }

		const glm::vec4& GetClearColour() const { return m_clearColour; }
		const glm::vec2& GetClearDepthStencil() const { return m_clearDepthStencil; }

		//RenderGraphResource& AddTextureInput(const std::string& name, PipelineStageFlags stages);
		//void AddStorageInput(const std::string& name, BufferAttachmentInfo& attachment);
		//RenderGraphResource& AddUniformInput(const std::string& name, BufferAttachmentInfo& attachment);

		const std::vector<u32>& GetColorInputs() const { return m_colorInputs; }
		const std::vector<u32>& GetColorOutputs() const { return m_colorOutputs; }
		const std::vector<u32>& GetAttachmentInputs() const { return m_attachmentsInputs; }
		const std::vector<RenderGraphResource>& GetGenericTextureInputs() const { return m_genericTextures; }

		RenderGraphResource& GetDepthStencilInput() const;
		RenderGraphResource& GetDepthStencilOutput() const;

		bool IsDepthSencilInputValid() const { return m_depthStencilInput != -1; }
		bool IsDepthSencilOuputValid() const { return m_depthStencilOutput != -1; }

		void AddDependentPass(u32 passIndex) { m_dependentPasses.insert(passIndex); }
		const std::unordered_set<u32> GetDependentPasses() { return m_dependentPasses; }

		RenderGraphResource& GetTextureResource(u32 index) const;
		const GPUImage* GetPhysicalImage(u32 index) const;
		const GPUImageView* GetPhysicalImageView(u32 index) const;

		u64 GetColorOutputHash() { return m_colorOutputHasher.GetHash(); }
		const Maths::Rect& GetWindowRect() const { return m_windowRect; }
		const glm::vec4& GetClearColor() { return m_clearColour; }
		const glm::vec2& GetClearDepthStencil() { return m_clearDepthStencil; }

		template<typename T>
		void AddLifeTimeObject(T* lifeTimeObejct);

	private:
		void CallBeginRenderFunc(GPURenderGraphPass* pass)
		{
			if (m_beginRenderFunc)
			{
				m_beginRenderFunc(this, pass);
			}
		}

		void CallRenderFunc(GPUCommandBuffer* cmdBuffer, FrameBufferResources& frameBuffers, GPUDescriptorBuilder* builder)
		{
			if (m_renderFunc)
			{
				m_renderFunc(cmdBuffer, frameBuffers, builder, *this);
			}
		}

		void CallEndRenderFunc()
		{
			if (m_endRenderFunc)
			{
				m_endRenderFunc(this);
			}
		}

	private:
		/// <summary>
		/// Owner graph for this render pass.
		/// </summary>
		RenderGraph* m_graph;
		/// <summary>
		/// Render pass index in the unordered passes vector.
		/// </summary>
		u32 m_passIndex;
		/// <summary>
		/// Pass index which has been ordered by the render graph. TODO: Do this.
		/// </summary>
		u32 m_orderedPassIndex;
		/// <summary>
		/// Queue for this pass.
		/// </summary>
		RenderGraphQueueFlags m_queue;
		/// <summary>
		/// Render pass name. Used for debug.
		/// </summary>
		std::string m_name;

		GPURenderGraphPass* m_graphPass;
		std::vector<RenderPassLifeTimeObject> m_lifeTimeObjects;

		RenderPassQueue m_passQueue;
		Maths::Rect m_windowRect;

		std::unordered_set<u32> m_dependentPasses;

		std::vector<u32> m_colorInputs;
		std::vector<u32> m_colorOutputs;
		std::vector<u32> m_attachmentsInputs;
		std::vector<RenderGraphResource> m_genericTextures;

		RenderPassRenderFunc m_renderFunc;
		RenderPassBeginRenderFunc m_beginRenderFunc;
		RenderPassEndRenderFunc m_endRenderFunc;
		glm::vec4 m_clearColour;
		glm::vec2 m_clearDepthStencil;

		Utils::Hasher m_colorOutputHasher;
		u32 m_depthStencilInput = -1;
		u32 m_depthStencilOutput = -1;

		friend RenderGraph;
	};

	template<typename T>
	inline void RenderPass::AddLifeTimeObject(T* lifeTimeObejct)
	{
		m_lifeTimeObjects.push_back(RenderPassLifeTimeObject(lifeTimeObejct));
	}
}