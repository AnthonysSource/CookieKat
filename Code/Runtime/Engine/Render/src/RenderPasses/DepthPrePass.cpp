#include "CookieKat/Engine/Render/RenderPasses/DepthPrePass.h"

#include "CookieKat/Engine/Entities/Components/LocalToWorldComponent.h"
#include "CookieKat/Engine/Entities/Components/MeshComponent.h"
#include "CookieKat/Engine/Render/RenderPasses/SharedIDs.h"
#include "CookieKat/Engine/Render/RenderingSystem.h"

#include "glm/gtc/type_ptr.hpp"

namespace CKE {
	void DepthPrePass::Initialize(RenderPassInitCtx* pInitCtx) {
		m_pEntityDb = pInitCtx->GetEntityDatabase();
		m_pResources = pInitCtx->GetResourceSystem();
		m_pRenderingSettings = pInitCtx->GetRenderingSettings();
		m_Pipeline = pInitCtx->GetPipelineManager()->GetPipeline(PipelineIDS::DepthPrePass);
	}

	void DepthPrePass::Setup(FrameGraphSetupContext& setup) {
		setup.UseBuffer(SceneGlobal::ObjectData);
		setup.UseBuffer(SceneGlobal::View);

		TextureDesc depthStencilDesc{};
		depthStencilDesc.m_Name = "DepthStencil";
		depthStencilDesc.m_Format = TextureFormat::D24_UNORM_S8_UINT;
		depthStencilDesc.m_AspectMask = TextureAspectMask::Depth | TextureAspectMask::Stencil;
		depthStencilDesc.m_TextureType = TextureType::Tex2D;
		setup.CreateTransientTexture(GBuffer::DepthStencil, depthStencilDesc
		                             , TextureExtraSettings{true, {1.0f, 1.0f}});
		setup.UseTexture(GBuffer::DepthStencil, FGPipelineAccessInfo::DepthStencil());
	}

	struct RenderBatch
	{
		BufferHandle m_VertexBuffer;
		BufferHandle m_IndexBuffer;
		u32 m_IndexCount;
	};

	void DepthPrePass::Execute(ExecuteResourcesCtx& ctx, GraphicsCommandList& cmdList, RenderDevice& rd) {
		TextureViewHandle const depthStencil = ctx.GetTextureView(General::DepthStencil);
		BufferHandle const      viewBuffer = ctx.GetBuffer(SceneGlobal::View);
		BufferHandle const      objectBuffer = ctx.GetBuffer(SceneGlobal::ObjectData);

		// Rendering Setup
		cmdList.BeginRendering(RenderingInfo{
			.m_RenderArea = m_pRenderingSettings->m_RenderArea,
			.m_ColorAttachments = {},
			.m_UseDepthAttachment = true,
			.m_DepthAttachment = RenderingAttachment{
				depthStencil,
				TextureLayout::DepthStencil_Attachment,
				LoadOp::Clear, StoreOp::Store,
			},
		});

		cmdList.SetPipeline(m_Pipeline);
		cmdList.SetDefaultViewportScissor(m_pRenderingSettings->m_Viewport.m_Extent);

		DescriptorSetBuilder      descriptorBuilder = rd.CreateDescriptorSetBuilder(m_Pipeline, 0);
		DescriptorSetHandle const descriptor = descriptorBuilder
		                                       .BindUniformBuffer(0, viewBuffer)
		                                       .BindStorageBuffer(1, objectBuffer)
		                                       .Build();
		cmdList.BindDescriptor(m_Pipeline, descriptor);

		// Record draw calls
		for (auto& [l2w, mesh] : m_pEntityDb->GetMultiCompTupleIter<
			     LocalToWorldComponent, MeshComponent>()) {
			MeshResource const* m = m_pResources->GetResource<MeshResource>(mesh->m_MeshID);
			cmdList.SetVertexBuffer(m->GetVertexBuffer());
			cmdList.SetIndexBuffer(m->GetIndexBuffer(), 0);
			cmdList.DrawIndexed(m->GetIndices().size(), mesh->m_ObjectIdx - 1);
		}

		cmdList.EndRendering();
	}
}
