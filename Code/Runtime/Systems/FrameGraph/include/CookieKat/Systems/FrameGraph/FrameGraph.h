#pragma once

#include "CookieKat/Core/Containers/Containers.h"
#include "CookieKat/Core/Memory/Memory.h"

#include "CookieKat/Systems/FrameGraph/FrameGraphRegistry.h"
#include "CookieKat/Systems/FrameGraph/FrameGraphBuilder.h"
#include "CookieKat/Systems/FrameGraph/FrameGraphPass.h"
#include "CookieKat/Systems/FrameGraph/FrameGraphResources.h"
#include "CookieKat/Systems/FrameGraph/FrameGraphDB.h"
#include "CookieKat/Systems/RenderAPI/CommandList.h"

namespace CKE {
	// Forward Declarations
	class RenderDevice;
}

namespace CKE {
	// FrameGraph/RenderGraph implementation to aid in the creation of modular rendering code split
	// into render passes.
	class FrameGraph
	{
	public:
		//-----------------------------------------------------------------------------

		void Initialize(RenderDevice* pDevice);
		void Shutdown();

		//-----------------------------------------------------------------------------

		// Add a new Graphics Pass to the end of the graph.
		//
		// The pointer must remain valid through all of the FrameGraph's lifetime.
		void AddGraphicsPass(FGGraphicsRenderPass* pRenderPass);

		// Add a new Transfer Pass to the end of the graph.
		//
		// The pointer must remain valid through all of the FrameGraph's lifetime.
		void AddTransferPass(FGTransferRenderPass* pRenderPass);

		// Add a new Compute Pass to the end of the graph
		//
		// The pointer must remain valid through all of the FrameGraph's lifetime.
		void AddComputePass(FGComputeRenderPass* pRenderPass);

		//-----------------------------------------------------------------------------

		// Calculates the required information about the graph
		// and prepares it for execution
		void Compile(UInt2 renderTargetSize);

		// Executes the configured graph, syncing using the provided objects
		void Execute(CmdListWaitSemaphoreInfo waitInfoAtStart,
		             SemaphoreHandle          signalSemaphoreOnFinish,
		             FenceHandle              signalFenceOnFinish);

		//-----------------------------------------------------------------------------

		// Imports a new texture or updates an existing one.
		// Makes it available for all of the passes.
		//
		// The texture layout is automatically transitioned in the graph execution, returning
		// it to its initial layout when finished.
		void ImportTexture(FGImportedTextureDesc desc);

		// Imports a new buffer and makes it available to all of the passes.
		void ImportBuffer(FGResourceID fgID, BufferHandle bufferHandle);

		// Updates the data of an existing imported texture.
		void UpdateImportedTexture(FGImportedTextureDesc desc);

		//-----------------------------------------------------------------------------

		// Updates the sizes of all textures that are dependent on the render target size.
		// This method destroys and recreates all of these textures.
		void UpdateRenderTargetSize(UInt2 newSize);

		//-----------------------------------------------------------------------------

		// Lambda-based variation of the base AddGraphicsPass(...) method,
		// allowing inline definition of the setup and execute functions of the pass
		// TODO: Specify function requirements with concepts
		template <typename SetupFunc, typename ExecuteFunc>
		void AddGraphicsPass(SetupFunc&& setupFunc, ExecuteFunc&& executeFunc);

		//-----------------------------------------------------------------------------

	private:
		friend class GfxQueueRecordingContext;

		// Data associated with a render pass
		struct RenderPassData
		{
			// Data defined when adding the pass to the graph
			RenderPassType      m_Type;           // Indicates the queue to which the pass will be submitted
			FGRenderPass*       m_pPass;          // Ptr to the pass defining object
			ExecuteResourcesCtx m_ExecuteContext; // All of the data accessible by the pass when executing

			// Data defined when compiling the graph
			Vector<TextureBarrierDescription> m_TransitionsBefore; // Required texture barriers BEFORE executing the pass commands
			Vector<TextureBarrierDescription> m_TransitionsAfter; // Required texture barriers AFTER executing the pass commands
			Vector<CmdListWaitSemaphoreInfo> m_WaitSemaphores; // Semaphores that the pass must wait on if any
			Vector<SemaphoreHandle> m_SignalSemaphores; // Semaphores that the pass signal on finish if any
			FenceHandle m_SignalFences; // Fence to signal on pass finish
			bool m_SubmitAfterExecuting = false; // Should this be the end of a command buffer and trigger a submit

			// Clears the references to resources when compiling the pass
			// IT DOESN'T RELEASE THE ACTUAL GPU RESOURCES
			inline void ClearCompilation(RenderDevice* pDevice) {
				m_TransitionsBefore.clear();
				m_TransitionsAfter.clear();
				m_WaitSemaphores.clear();
				m_SignalSemaphores.clear();
				m_SignalFences = FenceHandle{0};
			}

			inline CmdListSubmitInfo GetSubmitInfo() {
				CmdListSubmitInfo submitInfo{};
				submitInfo.m_WaitSemaphores = m_WaitSemaphores;
				submitInfo.m_SignalSemaphores = m_SignalSemaphores;
				submitInfo.m_SignalFence = m_SignalFences;
				return submitInfo;
			}
		};

		// Tracks the deferred destruction of Semaphores that are still in use
		struct DeletionEntry
		{
			u32             m_FramesTillDeletion;
			SemaphoreHandle m_Handle;
		};

	private:
		void CreateTransientResources(FrameGraphSetupContext setupCtx);
		void RecordResouceTransitions(GraphicsCommandList& cmdList, RenderPassData& renderPass);

		void ClearCurrentCompilation();

		void AddPass(FGRenderPassID id, FGRenderPass* pPass, RenderPassType type);

		void Compile_TransientTexUsageFlags();
		void Compile_BarriersAndQueueSync();
		void Execute_ReturnTexturesToOriginal(SemaphoreHandle signalSemaphoreOnFinish, FenceHandle signalFenceOnFinish);

	private:
		RenderDevice* m_pDevice = nullptr;

		// Returns the index in the passes vector
		Map<FGRenderPassID, u64> m_RenderPassIDToIndex;
		Vector<RenderPassData>   m_Passes;                  // Passes in submission order
		FrameGraphDB             m_DB{};                    // Graph resources
		UInt2                    m_RenderTargetSize{0, 0};  // Current backbuffer size used to calculate relative texture sizes
		Vector<DeletionEntry>    m_SemaphoreDeletionList{}; // Info to deffer the destruction of in-use data
	};
}

namespace CKE {
	template <typename SetupFunc, typename ExecuteFunc>
	class LambdaGfxPass : public FGGraphicsRenderPass
	{
	public:
		inline void Initialize(SetupFunc&& setupFunc, ExecuteFunc&& executeFunc) {
			m_SetupFunc = setupFunc;
			m_ExecuteFunc = executeFunc;
		}

		inline void Setup(FrameGraphSetupContext& setup) override {
			m_SetupFunc(setup);
		}

		inline void Execute(ExecuteResourcesCtx& ctx, GraphicsCommandList& cmdList, RenderDevice& rd) override {
			m_ExecuteFunc(ctx, cmdList, rd);
		}

	private:
		Func<void(FrameGraphSetupContext& setup)>                                            m_SetupFunc;
		Func<void(ExecuteResourcesCtx& ctx, GraphicsCommandList& cmdList, RenderDevice& rd)> m_ExecuteFunc;
	};

	template <typename SetupFunc, typename ExecuteFunc>
	void FrameGraph::AddGraphicsPass(SetupFunc&& setupFunc, ExecuteFunc&& executeFunc) {
		LambdaGfxPass<SetupFunc, ExecuteFunc>* pPass = new LambdaGfxPass<SetupFunc, ExecuteFunc>();
		pPass->Initialize(std::forward<SetupFunc>(setupFunc), std::forward<ExecuteFunc>(executeFunc));
		AddGraphicsPass(pPass);
	}
}
