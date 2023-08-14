#pragma once

#include "CookieKat/Core/Math/Math.h"
#include "CookieKat/Core/Platform/PrimitiveTypes.h"
#include "CookieKat/Systems/RenderAPI/RenderDevice.h"
#include <GLFW/glfw3.h>

namespace CKE
{
	inline void GLFW_FramebufferSizeCallback(GLFWwindow* wnd, int width, int height)
	{
		RenderDevice* pDevice = static_cast<RenderDevice*>(glfwGetWindowUserPointer(wnd));
		pDevice->RecordBackBufferResized(Int2{ width, height });
	}

	inline void GLFW_RegisterAllCallbacks(GLFWwindow* wnd)
	{
		glfwSetFramebufferSizeCallback(wnd, GLFW_FramebufferSizeCallback);
	}

	struct GLFWWindowConfig
	{
		const i32 m_Width{ 1280 };
		const i32 m_Height{ 720 };
		const char* m_Title{ "CookieKat Render Test" };
	};
}
