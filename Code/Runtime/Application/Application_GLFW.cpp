#include "Shared/GameEngine.h"

#define NOMINMAX
#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

namespace {
	using namespace CKE;

	// GLFW Callbacks
	//-----------------------------------------------------------------------------

	void FramebufferSizeCallback(GLFWwindow* wnd, int width, int height) {
		GameEngine* pGameEngine = static_cast<GameEngine*>(glfwGetWindowUserPointer(wnd));
		pGameEngine->GetEngine()->GetRenderingSystem()->RecordRenderTargetResizeEvent(Int2(width, height));
	}

	void MouseScrollCallback(GLFWwindow* wnd, double xoffset, double yoffset) { }

	void MouseButtonCallback(GLFWwindow* wnd, int button, int action, int mods) { }

	void MousePositionCallback(GLFWwindow* wnd, double xpos, double ypos) {
		GameEngine*          pGameEngine = static_cast<GameEngine*>(glfwGetWindowUserPointer(wnd));
		PlatformInputMessage msg{};
		msg.m_Data0 = static_cast<i64>(1);
		msg.m_Data1 = static_cast<i64>(0);
		msg.m_Data2 = static_cast<i64>(xpos);
		msg.m_Data3 = static_cast<i64>(ypos);
		pGameEngine->GetEngine()->GetInputSystem()->ProcessPlatformInputMessage(msg);
	}

	void KeyboardKeyCallback(GLFWwindow* wnd, int key, int scancode, int action, int mods) {
		GameEngine*          pGameEngine = static_cast<GameEngine*>(glfwGetWindowUserPointer(wnd));
		PlatformInputMessage msg{};
		msg.m_Data0 = static_cast<i64>(0);
		msg.m_Data1 = static_cast<i64>(key);
		msg.m_Data2 = static_cast<i64>(scancode);
		msg.m_Data3 = static_cast<i64>(action);
		msg.m_Data4 = static_cast<i64>(mods);
		pGameEngine->GetEngine()->GetInputSystem()->ProcessPlatformInputMessage(msg);
	}

	void KeyboardCharacterCallback(GLFWwindow* wnd, unsigned int codepoint) { }

	void RegisterGLFWCallbacks(GLFWwindow* wnd) {
		// Window Events
		glfwSetFramebufferSizeCallback(wnd, FramebufferSizeCallback);

		// Mouse Input
		glfwSetCursorPosCallback(wnd, MousePositionCallback);
		glfwSetScrollCallback(wnd, MouseScrollCallback);
		glfwSetMouseButtonCallback(wnd, MouseButtonCallback);

		// Keyboard Input
		glfwSetKeyCallback(wnd, KeyboardKeyCallback);
		glfwSetCharCallback(wnd, KeyboardCharacterCallback);
	}

	struct GLFWWindowConfig
	{
		const i32   m_Width{1280};
		const i32   m_Height{720};
		const char* m_Title{"CookieKat GLFW Engine"};
	};
} // namespace CKE

// Entry Point
//-----------------------------------------------------------------------------

int main() {
	using namespace CKE;

	// Init the platform
	glfwInit();

	// Create and configure window
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	constexpr GLFWWindowConfig s_WindowConfig{};
	GLFWwindow* pWindowHandle = glfwCreateWindow(s_WindowConfig.m_Width, s_WindowConfig.m_Height, s_WindowConfig.m_Title, NULL, NULL);
	glfwMakeContextCurrent(pWindowHandle);
	glfwSwapInterval(0);
	glfwSetInputMode(pWindowHandle, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetInputMode(pWindowHandle, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
	glfwSetWindowAspectRatio(pWindowHandle, 16, 9);

	RegisterGLFWCallbacks(pWindowHandle);

	// Init the engine
	GameEngine gameEngine{};
	glfwSetWindowUserPointer(pWindowHandle, &gameEngine);

	gameEngine.GetEngine()->GetRenderingSystem()->InitializeRenderTarget(glfwGetWin32Window(pWindowHandle));
	gameEngine.Initialize();

	// Set initial render target size because the callback
	// only gets called when its updated
	// gameEngine.GetEngine()->GetRenderingSystem()->UpdateRenderTargetSize(Int2(s_WindowConfig.m_Width, s_WindowConfig.m_Height));

	while (!glfwWindowShouldClose(pWindowHandle)) {
		glfwPollEvents();
		gameEngine.Update();
		glfwSwapBuffers(pWindowHandle);
	}

	gameEngine.Shutdown();
	glfwTerminate();
	return 0;
}
