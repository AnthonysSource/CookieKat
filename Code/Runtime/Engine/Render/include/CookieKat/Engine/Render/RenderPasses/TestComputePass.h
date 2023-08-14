#pragma once

#include "CookieKat/Engine/Render/RenderPasses/RenderPassInitCtx.h"
#include "CookieKat/Systems/ECS/EntityDatabase.h"
#include "CookieKat/Systems/FrameGraph/FrameGraphPass.h"
#include "CookieKat/Systems/Resources/ResourceSystem.h"

namespace CKE {

	class TestComputePass : public FGComputeRenderPass
	{
	public:
		void Setup(FrameGraphSetupContext& setup) override;
		void Execute(ExecuteResourcesCtx& ctx, ComputeCommandList& cmdList, RenderDevice& rd) override;
	};
}