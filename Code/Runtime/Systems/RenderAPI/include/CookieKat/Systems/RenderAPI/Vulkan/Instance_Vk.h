#pragma once

#include "CookieKat/Core/Containers/Containers.h"

#define NOMINMAX
#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

namespace CKE {
	// Forward Declarations
	class RenderDevice;
}

namespace CKE {
	// Vulkan Debug Function Pointers
	inline PFN_vkSetDebugUtilsObjectNameEXT pfnSetDebugUtilsObjectNameEXT;
	inline PFN_vkCmdBeginDebugUtilsLabelEXT pfnCmdBeginDebugUtilsLabelEXT;
	inline PFN_vkCmdEndDebugUtilsLabelEXT   pfnCmdEndDebugUtilsLabelEXT;
}

namespace CKE {
	class QueueFamilyIndices
	{
	public:
		inline u32 GetGraphicsIdx() { return m_GraphicsFamily; }
		inline u32 GetTransferIdx() { return m_TransferFamily; }
		inline u32 GetPresentIdx() { return m_PresentFamily; }
		inline u32 GetComputeIdx() { return m_ComputeFamily; }

	private:
		friend class VulkanInstance;
		u32 m_GraphicsFamily{};
		u32 m_PresentFamily{};
		u32 m_TransferFamily{};
		u32 m_ComputeFamily{};
	};
}

namespace CKE {
	class VulkanInstance
	{
	public:
		// Global Setup
		//-----------------------------------------------------------------------------

		void Initialize(void* pWindowData);
		void Shutdown();

		inline VkSurfaceKHR GetSurface() const { return m_Surface; }
		void                CreateLogicalDevice(RenderDevice& device);

		SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device);
		QueueFamilyIndices      FindQueueFamilies(VkPhysicalDevice physicalDevice);

	private:
		friend RenderDevice;
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
			void*                                       pUserData);

		// Surface
		//-----------------------------------------------------------------------------

		void CreateSurface(HWND window);

		// Devices
		//-----------------------------------------------------------------------------

		VkPhysicalDevice SelectPhysicalDevice(Vector<const char*> const& requiredExtensions);

		// Querying and Suitability Checks
		bool CheckDeviceSupportsNecessaryExtensions(VkPhysicalDevice           device,
		                                            Vector<const char*> const& deviceExtensions);
		bool IsPhysicalDeviceSuitable(VkPhysicalDevice device, Vector<const char*> const& requiredExtensions);

	private:
		VkInstance               m_Instance{};
		VkDebugUtilsMessengerEXT m_DebugMessenger{};
		Vector<const char*>      m_ValidationLayers;

		VkSurfaceKHR m_Surface{};
	};
}
