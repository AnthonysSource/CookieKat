#pragma once
#include "CookieKat/Systems/RenderAPI/RenderDevice.h"

namespace CKE {
	// Base interface for al rendering tests
	class IRenderSandboxTest
	{
	public:
		virtual void Setup(RenderDevice* pDevice) = 0;
		virtual void Render(RenderDevice* pDevice) = 0;
		virtual void TearDown(RenderDevice* pDevice) = 0;
	};

	// Rendering Test Runner
	class RenderSandboxEnv
	{
	public:
		void Run(IRenderSandboxTest* pTestDef);
	};
}
