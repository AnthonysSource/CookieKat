#define NOMINMAX
#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include <gtest/gtest.h>

#include "CookieKat/Systems/RenderAPI/RenderDevice.h"

using namespace CKE;

namespace
{
	using namespace CKE;

	// GLFW Callbacks
	//-----------------------------------------------------------------------------

	void FramebufferSizeCallback(GLFWwindow* wnd, int width, int height)
	{
		RenderDevice* pDevice = static_cast<RenderDevice*>(glfwGetWindowUserPointer(wnd));
		pDevice->RecordBackBufferResized(Int2{ width, height });
	}

	void RegisterGLFWCallbacks(GLFWwindow* wnd)
	{
		// Window Events
		glfwSetFramebufferSizeCallback(wnd, FramebufferSizeCallback);
	}

	struct GLFWWindowConfig
	{
		const i32 m_Width{ 1280 };
		const i32 m_Height{ 720 };
		const char* m_Title{ "CookieKat Render Test" };
	};

} // namespace CKE

// Entry Point
//-----------------------------------------------------------------------------

void Render(RenderDevice& device) {
	device.AcquireNextBackBuffer();

	auto cmdList = device.GetGraphicsCmdList();

	cmdList.Begin();
	//cmdList.BeginRendering(RenderingInfo{
	//	device.GetBackBufferSize(),
	//	{
	//		RenderingAttachment{
	//			device.GetBackBufferView(),
	//			TextureLayout::Color_Attachment,
	//			LoadOp::DontCare, StoreOp::Store
	//		}
	//	},
	//	true,
	//	RenderingAttachment{

	//	}
	//});
	cmdList.End();

	device.Present();
}

TEST(Rendering, Open_Window)
{
	using namespace CKE;

	// Init the platform
	glfwInit();

	// Create and configure window
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	constexpr GLFWWindowConfig s_WindowConfig{};
	GLFWwindow* pWindowHandle = glfwCreateWindow(s_WindowConfig.m_Width, s_WindowConfig.m_Height, s_WindowConfig.m_Title, NULL, NULL);
	glfwMakeContextCurrent(pWindowHandle);
	glfwSwapInterval(0);
	glfwSetWindowAspectRatio(pWindowHandle, 16, 9);

	RegisterGLFWCallbacks(pWindowHandle);

	RenderDevice device{};
	glfwSetWindowUserPointer(pWindowHandle, &device);

	device.PassRenderTargetData(glfwGetWin32Window(pWindowHandle));
	device.Initialize({ s_WindowConfig.m_Width, s_WindowConfig.m_Height });

	while (!glfwWindowShouldClose(pWindowHandle))
	{
		glfwPollEvents();
		Render(device);
		glfwSwapBuffers(pWindowHandle);
	}

	glfwTerminate();
	EXPECT_TRUE(true);
}