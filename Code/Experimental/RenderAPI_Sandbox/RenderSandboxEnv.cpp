#include "RenderSandboxEnv.h"

#include "CookieKat/Core/Memory/Memory.h"
#include "GLFWUtils.h"

namespace CKE {
	void RenderSandboxEnv::Run(IRenderSandboxTest* pTestDef) {
		// Init the platform
		glfwInit();

		// Create and configure window
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		constexpr GLFWWindowConfig s_WindowConfig{};
		GLFWwindow* pWindow = glfwCreateWindow(s_WindowConfig.m_Width, s_WindowConfig.m_Height, s_WindowConfig.m_Title, NULL, NULL);
		glfwMakeContextCurrent(pWindow);
		glfwSwapInterval(0);
		glfwSetWindowAspectRatio(pWindow, 16, 9);

		GLFW_RegisterAllCallbacks(pWindow);

		RenderDevice device{};
		glfwSetWindowUserPointer(pWindow, &device);

		device.PassRenderTargetData(glfwGetWin32Window(pWindow));
		device.Initialize({s_WindowConfig.m_Width, s_WindowConfig.m_Height});
		pTestDef->Setup(&device);

		while (!glfwWindowShouldClose(pWindow)) {
			glfwPollEvents();
			pTestDef->Render(&device);
			glfwSwapBuffers(pWindow);
		}

		device.WaitForDevice();
		pTestDef->TearDown(&device);
		glfwTerminate();
	}
}
