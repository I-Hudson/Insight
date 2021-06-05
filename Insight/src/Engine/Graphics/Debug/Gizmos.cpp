#include "ispch.h"
#include "Engine/Graphics/Debug/Gizmos.h"
#include "Engine/Graphics/RenderGraph/RenderGraph.h"
#include "Engine/Graphics/RenderGraph/RenderPass.h"
#include "Engine/Graphics/GPUDynamicBuffer.h"
#include "Engine/Graphics/Shaders/GPUShader.h"
#include "Engine/Graphics/GPUCommandBuffer.h"
#include "Engine/Scene/Scene.h"

#include "Engine/Component/TransformComponent.h"

namespace Insight::Graphics::Debug
{
	Gizmos::Gizmos()
	{
	}

	Gizmos::~Gizmos()
	{ }

	void Gizmos::DrawGizmos(CameraComponent& cameraComponent)
	{
		RenderGraph& graph = *RenderGraph::Instance();
		auto& pass = graph.AddPass("Gizmos", RenderGraphQueueFlagsBits::RENDER_GRAPH_QUEUE_GRAPHICS_BIT);
		pass.AddColorOutput("color");
		pass.SetPassQueue(Graphics::RenderPassQueue::Overlay);
		pass.SetRenderFunc([&cameraComponent](GPUCommandBuffer* cmdBuffer, FrameBufferResources& buffers, GPUDescriptorBuilder* builder, RenderPass& pass, RenderList* renderList)
		{
			IS_PROFILE_SCOPE("Gizmos");

			GPUShader* gizmosShader = nullptr;
			{
				IS_PROFILE_SCOPE("Shader");
				Utils::Hasher hash;
				hash.Hash("./data/shaders/vulkan/gizmos.vert");
				hash.Hash("./data/shaders/vulkan/gizmos.frag");
				if (GPUShaderCache::Instance()->GetItem(hash.GetHash(), gizmosShader))
				{
					gizmosShader->SetStage(ShaderStage::Vertex, "./data/shaders/vulkan/gizmos.vert", ShaderStageInput::FilePath);
					gizmosShader->SetStage(ShaderStage::Fragment, "./data/shaders/vulkan/gizmos.frag", ShaderStageInput::FilePath);
					gizmosShader->Compile();
				}
			}

			GPUPipeline* gizmosPipeline = GPUPipeline::New();
			{
				IS_PROFILE_SCOPE("Pipeline");
				pass.AddLifeTimeObject(gizmosPipeline);
				gizmosPipeline->SetShader(gizmosShader);
				gizmosPipeline->Init(pass.GetGraphPass(), GPUPipelineDesc::GPUPipelineDesc(PrimitiveTopologyType::Line_Strip, PolygonMode::Line, CullMode::None, FrontFace::Counter_Clockwise));
				cmdBuffer->BindPipeline(PipelineBindPoint::Graphics, gizmosPipeline);
			}

			IS_PROFILE_SCOPE("Transforms");
			{
				struct Line
				{
					Line(glm::vec3 pos, glm::vec4 color)
						: Pos(pos), Color(color)
					{ }
					glm::vec3 Pos;
					glm::vec4 Color;
				};
				//std::vector<Line> lines;
				//std::vector<GPUBuffer*> vertexbuffers;

				auto& transforms = Scene::ActiveScene()->GetAllComponents<TransformComponent>();
				for (auto& com : transforms)
				{
					IS_PROFILE_SCOPE("Single Transform");
					if (!com.IsValid())
					{
						break;
					}

					std::vector<Line> lines
					{
						Line(com.GetTransform()[3], glm::vec4(1, 0, 0, 1)),
						Line(com.GetTransform()[3] + com.GetTransform()[0] * 3.0f, glm::vec4(1, 0, 0, 1)),

						Line(com.GetTransform()[3], glm::vec4(0, 1, 0, 1)),
						Line(com.GetTransform()[3] + com.GetTransform()[1] * 3.0f, glm::vec4(0, 1, 0, 1)),

						Line(com.GetTransform()[3], glm::vec4(0, 0, 1, 1)),
						Line(com.GetTransform()[3] + com.GetTransform()[2] * 3.0f, glm::vec4(0, 0, 1, 1)),
					};
					GPUBuffer* vBuffer = nullptr;
					{
						IS_PROFILE_SCOPE("VBuffer");
						vBuffer = buffers.at(GPUBufferFlags::VERTEX)->Upload(lines.data(), sizeof(Line) * lines.size());
					}
					glm::mat4 projViewMatrixx = cameraComponent.GetProjMatrix() * glm::inverse(cameraComponent.GetViewMatrix());
					Graphics::GPUBuffer* uboBuffer = buffers.at(Graphics::GPUBufferFlags::UNIFORM)->Upload(&projViewMatrixx, sizeof(projViewMatrixx));

					Graphics::GPUDescriptorSet* vertexSet = nullptr;
					{
						IS_PROFILE_SCOPE("BindDescriptorSets");
						builder->BindBuffer(0, uboBuffer, DescriptorType::Unifom_Buffer, ShaderStage::Vertex)->Build(vertexSet);
						Graphics::GPUDescriptorSet* sets[] = { vertexSet };
						cmdBuffer->BindDescriptorSets(PipelineBindPoint::Graphics, gizmosPipeline, 0, ARRAY_COUNT(sets), sets, 0, nullptr);
					}
					GPUBuffer* vBuffers[] = { vBuffer };
					u32 offsets[] = { 0 };
					cmdBuffer->BindVertexBuffers(0, 1, vBuffers, offsets);

					cmdBuffer->SetLineWidth(1.0f);

					cmdBuffer->Draw(6, 1, 0, 0);
				}
			}
		});
	}
}