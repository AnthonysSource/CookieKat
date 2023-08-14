#pragma once

#include "CookieKat/Core/Containers/String.h"

#include "CookieKat/Systems/FrameGraph/FrameGraphResources.h"

namespace CKE {
	// Forward Declarations
	class ComputeCommandList;
	class TransferCommandList;
	class GraphicsCommandList;
	class FrameGraph;
	class RenderDevice;
	class ExecuteResourcesCtx;
	class FrameGraphSetupContext;
}

namespace CKE {
	class FGRenderPass
	{
	public:
		FGRenderPass() : m_ID{"Unnamed"} { }
		FGRenderPass(FGRenderPassID id) : m_ID(id) { }
		virtual ~FGRenderPass() = default;

		virtual void Setup(FrameGraphSetupContext& setup) = 0;

	protected:
		friend FrameGraph;

		FGRenderPassID m_ID{};
	};

	class FGGraphicsRenderPass : public FGRenderPass
	{
	public:
		FGGraphicsRenderPass() : FGRenderPass() {};
		FGGraphicsRenderPass(FGRenderPassID id) : FGRenderPass(id) { }

		virtual void Execute(ExecuteResourcesCtx& ctx, GraphicsCommandList& cmdList, RenderDevice& rd) = 0;
	};

	class FGTransferRenderPass : public FGRenderPass
	{
	public:
		FGTransferRenderPass() : FGRenderPass() {};
		FGTransferRenderPass(FGRenderPassID id) : FGRenderPass(id) { }

		virtual void Execute(ExecuteResourcesCtx& ctx, TransferCommandList& cmdList, RenderDevice& rd) = 0;
	};

	class FGComputeRenderPass : public FGRenderPass
	{
	public:
		FGComputeRenderPass() : FGRenderPass() {};
		FGComputeRenderPass(FGRenderPassID id) : FGRenderPass(id) { }

		virtual void Execute(ExecuteResourcesCtx& ctx, ComputeCommandList& cmdList, RenderDevice& rd) = 0;
	};
}
