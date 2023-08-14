#pragma once

#include "Auxiliary.h"

#include "CookieKat/Core/Containers/Containers.h"
#include "CookieKat/Core/Containers/String.h"
#include "CookieKat/Core/Platform/Asserts.h"
#include "CookieKat/Core/FileSystem/FileSystem.h"
#include "CookieKat/Core/Math/Math.h"

#define NOMINMAX
#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

using namespace CKE;
class VulkanDevice;

inline GLFWwindow* g_pWindow;
constexpr bool g_EnableValidationLayers = true;

class VulkanBackend
{
public:

	// Global Setup
	//-----------------------------------------------------------------------------

	void Initialize();
	void Shutdown();

	inline VkSurfaceKHR GetSurface() const { return m_Surface; }
	VulkanDevice CreateLogicalDevice();

	SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device);
	QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice physicalDevice);

private:
	// Instance
	//-----------------------------------------------------------------------------

	void CreateInstance(VkDebugUtilsMessengerCreateInfoEXT& msgCreateInfo);

	// Querying and Checks
	static bool CheckInstanceValidationLayersSupport(Vector<const char*> const& validationLayers);
	static void GetRequiredInstanceExtensions(Vector<const char*>& requiredExtensions);
	static void EnumerateExtensionsToConsole(Vector<VkExtensionProperties> const& extensions);

	// Debugging
	void        CreateDebugMessenger(VkDebugUtilsMessengerCreateInfoEXT msgCreateInfo);
	void        ShutdownDebugMessenger();
	static void ConfigVkDebugMessenger(VkDebugUtilsMessengerCreateInfoEXT& msgCreateInfo);

	static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT      messageSeverity,
		VkDebugUtilsMessageSeverityFlagsEXT         messageType,
		VkDebugUtilsMessengerCallbackDataEXT const* pCallbackData,
		void* pUserData);

	// Surface
	//-----------------------------------------------------------------------------

	void CreateSurface();

	// Devices
	//-----------------------------------------------------------------------------

	VkPhysicalDevice SelectPhysicalDevice(Vector<const char*> const& requiredExtensions);

	// Querying and Suitability Checks
	bool CheckDeviceSupportsNecessaryExtensions(VkPhysicalDevice device, Vector<const char*> const& deviceExtensions);
	bool IsPhysicalDeviceSuitable(VkPhysicalDevice device, Vector<const char*> const& requiredExtensions);

private:

	VkInstance               m_Instance{};
	VkDebugUtilsMessengerEXT m_DebugMessenger{};
	VkSurfaceKHR m_Surface{};

	Vector<const char*> validationLayers;
};
