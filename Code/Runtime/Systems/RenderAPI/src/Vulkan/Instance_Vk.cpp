#include "CookieKat/Core/Platform/PrimitiveTypes.h"
#include "CookieKat/Core/Containers/String.h"

#include "CookieKat/Systems/RenderAPI/Internal/SwapChain.h"
#include "CookieKat/Systems/RenderAPI/Vulkan/Instance_Vk.h"
#include "CookieKat/Systems/RenderAPI/Vulkan/RenderDevice_Vk.h"
#include "CookieKat/Systems/RenderAPI/RenderHandle.h"
#include "CookieKat/Systems/RenderAPI/CommandQueue.h"

#include <vulkan/vulkan_core.h>
#include <iostream>

namespace CKE {
	bool RenderInstance::CheckInstanceValidationLayersSupport(Vector<const char*> const& validationLayers) {
		u32 layerCount;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
		Vector<VkLayerProperties> availableLayers(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

		for (const char* validationLayer : validationLayers) {
			bool layerFound = false;

			for (auto const& availableLayer : availableLayers) {
				if (strcmp(validationLayer, availableLayer.layerName) == 0) {
					layerFound = true;
					break;
				}
			}

			if (!layerFound) { return false; }
		}
		return true;
	}

	void RenderInstance::GetRequiredInstanceExtensions(Vector<const char*>& requiredExtensions) {
		u32          glfwExtensionCount = 0;
		const char** glfwExtensionNames = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

		requiredExtensions.clear();
		requiredExtensions.reserve(glfwExtensionCount);
		for (u32 i = 0; i < glfwExtensionCount; ++i) { requiredExtensions.push_back(glfwExtensionNames[i]); }

		if constexpr (RenderSettings::ENABLE_DEBUG) { requiredExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME); }
		requiredExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}

	VkResult CreateDebugUtilsMessengerEXT(
		VkInstance                                instance,
		VkDebugUtilsMessengerCreateInfoEXT const* pCreateInfo,
		VkAllocationCallbacks*                    pAllocator,
		VkDebugUtilsMessengerEXT*                 pDebugMessenger) {
		auto func = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
			vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT"));
		if (func == nullptr) { return VK_ERROR_EXTENSION_NOT_PRESENT; }

		return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
	}

	void DestroyDebugUtilsMessengerEXT(
		VkInstance                   instance,
		VkDebugUtilsMessengerEXT     debugMessenger,
		const VkAllocationCallbacks* pAllocator) {
		auto func = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
			vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT"));
		if (func == nullptr) {
			std::cout << "Function to destroy debug messenger not found" << std::endl;
			return;
		}

		func(instance, debugMessenger, pAllocator);
	}

	void RenderInstance::EnumerateExtensionsToConsole(Vector<VkExtensionProperties> const& extensions) {
		std::cout << "-----------------------------------------------------------------------------" << std::endl;
		std::cout << "Available Extensions:" << std::endl;
		std::cout << "-----------------------------------------------------------------------------" << std::endl;
		for (VkExtensionProperties const& extension : extensions) {
			std::cout << extension.extensionName << " - " << extension.specVersion << std::endl;
		}
	}

	VkBool32 RenderInstance::DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT      messageSeverity,
	                                       VkDebugUtilsMessageSeverityFlagsEXT         messageType,
	                                       VkDebugUtilsMessengerCallbackDataEXT const* pCallbackData,
	                                       void*                                       pUserData) {
		if (messageType == VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT) {
			return VK_FALSE;
		}

		std::cout << "-----------------------------------------------------------------------------" << std::endl;
		std::cout << "Message Type: ";
		switch (messageType) {
		case VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT: std::cout << "General";
			break;
		case VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT: std::cout << "Validation";
			break;
		case VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT: std::cout << "Performance";
			break;
		default: std::cout << "Unknown";
		}
		std::cout << std::endl;
		std::cout << pCallbackData->pMessage << std::endl;
		std::cout << "-----------------------------------------------------------------------------" << std::endl;

		return VK_FALSE;
	}

	VkPhysicalDevice RenderInstance::SelectPhysicalDevice(Vector<const char*> const& requiredExtensions) {
		// Enumerate all physical devices
		u32 deviceCount = 0;
		vkEnumeratePhysicalDevices(m_Instance, &deviceCount, nullptr);
		if (deviceCount == 0) { std::cout << "Didn't find a physical device with Vulkan support" << std::endl; }
		Vector<VkPhysicalDevice> physicalDevices{deviceCount};
		vkEnumeratePhysicalDevices(m_Instance, &deviceCount, physicalDevices.data());

		VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;

		// Iterate over all of them to find a suitable one
		for (VkPhysicalDevice& device : physicalDevices) {
			if (IsPhysicalDeviceSuitable(device, requiredExtensions)) {
				physicalDevice = device;
				return physicalDevice;
			}
		}

		if (physicalDevice == VK_NULL_HANDLE) {
			std::cout << "No suitable physical device found, NOTE: Only discrete devices are supported" << std::endl;
		}
		return physicalDevice;
	}

	bool RenderInstance::CheckDeviceSupportsNecessaryExtensions(VkPhysicalDevice           device,
	                                                            Vector<const char*> const& deviceExtensions) {
		u32 extensionCount;
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);
		std::vector<VkExtensionProperties> availableExtensions(extensionCount);
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

		Set<String> requiredExtensions{deviceExtensions.begin(), deviceExtensions.end()};
		for (VkExtensionProperties const& extension : availableExtensions) {
			requiredExtensions.erase(extension.extensionName);
		}

		return requiredExtensions.empty();
	}

	bool RenderInstance::IsPhysicalDeviceSuitable(VkPhysicalDevice           device,
	                                              Vector<const char*> const& requiredExtensions) {
		// VulkanDevice properties & features
		VkPhysicalDeviceProperties properties{};
		vkGetPhysicalDeviceProperties(device, &properties);
		VkPhysicalDeviceFeatures features{};
		vkGetPhysicalDeviceFeatures(device, &features);

		// Queue Families Support
		CommandQueueFamilyIndices queueFamilyIndices = FindQueueFamilies(device);

		// Extensions Support
		bool extensionsSupported = CheckDeviceSupportsNecessaryExtensions(device, requiredExtensions);

		// SwapChain Support
		bool swapChainAdequate = false;
		if (extensionsSupported) {
			SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(device);
			swapChainAdequate = !swapChainSupport.m_Formats.empty() && !swapChainSupport.m_PresentModes.empty();
		}

		return
				properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU &&
				features.geometryShader &&
				extensionsSupported &&
				swapChainAdequate;
	}

	CommandQueueFamilyIndices RenderInstance::FindQueueFamilies(VkPhysicalDevice physicalDevice) {
		CommandQueueFamilyIndices indices{};

		u32 queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);
		Vector<VkQueueFamilyProperties> queueFamilies{queueFamilyCount};
		vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());

		VkBool32 presentSupport = false;
		bool     hasTransferQueue = false;
		bool     hasComputeQueue = false;

		for (u32 i = 0; i < queueFamilies.size(); ++i) {
			VkQueueFamilyProperties const& family = queueFamilies[i];

			// Graphics Family
			if (family.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
				indices.m_GraphicsFamily = i;
			}

			// Present Family
			vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, m_Surface, &presentSupport);
			if (presentSupport) {
				indices.m_PresentFamily = i;
			}

			// Transfer/Copy Family
			if (family.queueFlags & VK_QUEUE_TRANSFER_BIT &&
				!(family.queueFlags & VK_QUEUE_GRAPHICS_BIT) &&
				!presentSupport) {
				hasTransferQueue = true;
				indices.m_TransferFamily = i;
			}

			// Compute Family
			if (family.queueFlags & VK_QUEUE_COMPUTE_BIT &&
				!(family.queueFlags & VK_QUEUE_GRAPHICS_BIT)) {
				indices.m_ComputeFamily = i;
				hasComputeQueue = true;
			}
		}

		// If the device doesn't have a dedicated transfer queue we create a second graphics
		// queue and use it as a transfer one
		if (!hasTransferQueue) {
			indices.m_TransferFamily = indices.m_GraphicsFamily;
		}

		if (!hasComputeQueue) {
			indices.m_ComputeFamily = indices.m_GraphicsFamily;
		}

		return indices;
	}

	RenderDevice RenderInstance::CreateDevice(RenderDeviceCreateInfo const& deviceCreateInfo) {
		RenderDevice device{};

		Vector<char const*> deviceRequiredExt = {
			VK_KHR_SWAPCHAIN_EXTENSION_NAME
		};
		device.m_PhysicalDevice = SelectPhysicalDevice(deviceRequiredExt);
		device.m_QueueFamilyIndices = FindQueueFamilies(device.m_PhysicalDevice);

		// Queues
		//-----------------------------------------------------------------------------

		Vector<VkDeviceQueueCreateInfo> queueCreateInfos{};
		for (i32 i = 0; i < deviceCreateInfo.m_QueueDescCount; ++i) {
			CommandQueueDesc desc = deviceCreateInfo.m_QueueDesc[i];

			VkDeviceQueueCreateInfo queueCreateInfo{};
			queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueCreateInfo.queueFamilyIndex = device.m_QueueFamilyIndices.GetIndex(desc.m_QueueType);
			queueCreateInfo.queueCount = 1;
			queueCreateInfo.pQueuePriorities = &desc.m_Priority.m_Value;
			queueCreateInfos.emplace_back(queueCreateInfo);
		}

		// Vulkan Device
		//-----------------------------------------------------------------------------

		VkPhysicalDeviceVulkan11Features vk11features{
			.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES
		};
		VkPhysicalDeviceVulkan13Features vk13features{
			.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES
		};
		VkPhysicalDeviceFeatures2 deviceFeatures2{
			.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
			.pNext = &vk13features,
		};
		vkGetPhysicalDeviceFeatures2(device.m_PhysicalDevice, &deviceFeatures2);
		deviceFeatures2.features.samplerAnisotropy = VK_TRUE;
		vk13features.dynamicRendering = VK_TRUE;
		vk13features.pNext = &vk11features;
		vk11features.shaderDrawParameters = VK_TRUE;

		VkDeviceCreateInfo deviceCreateInfo{};
		deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		deviceCreateInfo.queueCreateInfoCount = static_cast<u32>(queueCreateInfos.size());
		deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
		deviceCreateInfo.pNext = &deviceFeatures2;

		// NOTE: No longer needed in newer versions of Vulkan
		deviceCreateInfo.enabledExtensionCount = static_cast<u32>(deviceRequiredExt.size());
		deviceCreateInfo.ppEnabledExtensionNames = deviceRequiredExt.data();

		// Layers
		//-----------------------------------------------------------------------------

		if constexpr (RenderSettings::ENABLE_DEBUG) {
			deviceCreateInfo.enabledLayerCount = static_cast<u32>(m_ValidationLayers.size());
			deviceCreateInfo.ppEnabledLayerNames = m_ValidationLayers.data();
		}
		else {
			deviceCreateInfo.enabledLayerCount = 0;
		}

		VK_CHECK_CALL(vkCreateDevice(device.m_PhysicalDevice, &deviceCreateInfo, nullptr, &device.m_Device));
	}

	SwapChainSupportDetails RenderInstance::QuerySwapChainSupport(VkPhysicalDevice device) {
		SwapChainSupportDetails details{};

		// Capabilities
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, m_Surface, &details.m_Capabilities);

		// Formats
		u32 formatCount = 0;
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_Surface, &formatCount, nullptr);
		if (formatCount > 0) {
			details.m_Formats.resize(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_Surface, &formatCount, details.m_Formats.data());
		}

		// Present Modes
		u32 presentModeCount = 0;
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_Surface, &presentModeCount, nullptr);
		if (presentModeCount > 0) {
			details.m_PresentModes.resize(presentModeCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_Surface, &presentModeCount,
			                                          details.m_PresentModes.data());
		}

		return details;
	}

	void RenderInstance::ConfigVkDebugMessenger(VkDebugUtilsMessengerCreateInfoEXT& msgCreateInfo) {
		msgCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		msgCreateInfo.messageSeverity =
				VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
				VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		msgCreateInfo.messageType =
				VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
				VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
				VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		msgCreateInfo.pfnUserCallback = DebugCallback;
		msgCreateInfo.pUserData = nullptr;
	}

	void RenderInstance::CreateInstance(VkDebugUtilsMessengerCreateInfoEXT& msgCreateInfo) {
		// Application Info
		//-----------------------------------------------------------------------------

		VkApplicationInfo appInfo{};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = "Vulkanik CookieKat";
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = "CKE";
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion = VK_API_VERSION_1_3;

		// Instance Creation Info
		//-----------------------------------------------------------------------------

		VkInstanceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;

		// Extensions
		//-----------------------------------------------------------------------------

		// Get required extensions
		Vector<const char*> requiredExtensions{};
		GetRequiredInstanceExtensions(requiredExtensions);
		createInfo.enabledExtensionCount = static_cast<u32>(requiredExtensions.size());
		createInfo.ppEnabledExtensionNames = requiredExtensions.data();

		// Validation Layers
		//-----------------------------------------------------------------------------

		m_ValidationLayers = {"VK_LAYER_KHRONOS_validation"};
		msgCreateInfo = {};

		if constexpr (RenderSettings::ENABLE_DEBUG) {
			// Check layer support
			if (CheckInstanceValidationLayersSupport(m_ValidationLayers)) {
				createInfo.enabledLayerCount = static_cast<u32>(m_ValidationLayers.size());
				createInfo.ppEnabledLayerNames = m_ValidationLayers.data();
			}
			else {
				std::cout << "Error, validation layers are not supported" << std::endl;
			}

			// Debug messenger
			ConfigVkDebugMessenger(msgCreateInfo);
			createInfo.pNext = &msgCreateInfo;
		}
		else {
			createInfo.enabledLayerCount = 0;
		}


		VkResult result = vkCreateInstance(&createInfo, nullptr, &m_Instance);
		if (result != VK_SUCCESS) {
			std::cout << "Error creating vulkan instance" << std::endl;
			if (result == VK_ERROR_EXTENSION_NOT_PRESENT) { std::cout << "Extensions not present" << std::endl; }
			else if (result == VK_ERROR_LAYER_NOT_PRESENT) { std::cout << "Layer not present" << std::endl; }
			else { std::cout << "Unhandled error" << std::endl; }
		}
	}

	void RenderInstance::CreateDebugMessenger(VkDebugUtilsMessengerCreateInfoEXT msgCreateInfo) {
		if (RenderSettings::ENABLE_DEBUG) {
			VK_CHECK_CALL(CreateDebugUtilsMessengerEXT(m_Instance, &msgCreateInfo, nullptr, &m_DebugMessenger));
		}
	}

	void RenderInstance::RetrieveDebugFunctionPtrs() {
		pfnSetDebugUtilsObjectNameEXT = (PFN_vkSetDebugUtilsObjectNameEXT)vkGetInstanceProcAddr(
			m_Instance, "vkSetDebugUtilsObjectNameEXT");
		pfnCmdBeginDebugUtilsLabelEXT = (PFN_vkCmdBeginDebugUtilsLabelEXT)vkGetInstanceProcAddr(
			m_Instance, "vkCmdBeginDebugUtilsLabelEXT");
		pfnCmdEndDebugUtilsLabelEXT = (PFN_vkCmdEndDebugUtilsLabelEXT)vkGetInstanceProcAddr(
			m_Instance, "vkCmdEndDebugUtilsLabelEXT");
	}

	void RenderInstance::Initialize() {
		VkDebugUtilsMessengerCreateInfoEXT msgCreateInfo;

		CreateInstance(msgCreateInfo);
		CreateDebugMessenger(msgCreateInfo);
		CreateSurface((HWND)pWindowData);
		RetrieveDebugFunctionPtrs();
	}

	void RenderInstance::ShutdownDebugMessenger() {
		if (RenderSettings::ENABLE_DEBUG) { DestroyDebugUtilsMessengerEXT(m_Instance, m_DebugMessenger, nullptr); }
	}

	void RenderInstance::Shutdown() {
		vkDestroySurfaceKHR(m_Instance, m_Surface, nullptr);
		ShutdownDebugMessenger();
		vkDestroyInstance(m_Instance, nullptr);
	}

	void FrameData::Initialize(RenderDevice& device) {
		m_InFlightFence = device.CreateFence(true);
		m_ImageAvailableSemaphore = device.CreateSemaphoreGPU();
		m_RenderFinishedSemaphore = device.CreateSemaphoreGPU();
	}

	void FrameData::ResetForNewFrame() {
		for (auto& [handle, frameData] : m_da) {
			frameData.m_DescriptorSets.clear();
		}

		m_LastCmdIdxGraphics = 0;
		m_LastCmdIdxTransfer = 0;
		m_LastCmdIdxCompute = 0;
	}

	void FrameData::Destroy(RenderDevice& device) { }

	VkCommandBuffer FrameData::GetNextCmdBuffer(QueueType type) {
		switch (type) {
		case QueueType::Graphics: return GetNextGraphicsCmdBuffer();
		case QueueType::Transfer: return GetNextTransferCmdBuffer();
		case QueueType::Compute: return GetNextComputeCmdBuffer();
		default: CKE_UNREACHABLE_CODE();
		}
		return (VkCommandBuffer)0;
	}

	VkCommandBuffer FrameData::GetNextGraphicsCmdBuffer() {
		CKE_ASSERT(m_LastCmdIdxGraphics < m_GraphicsCommandBuffer.size());
		VkCommandBuffer cmdBuff = m_GraphicsCommandBuffer[m_LastCmdIdxGraphics];
		m_LastCmdIdxGraphics++;
		return cmdBuff;
	}

	VkCommandBuffer FrameData::GetNextTransferCmdBuffer() {
		CKE_ASSERT(m_LastCmdIdxTransfer < m_TransferCommandBuffer.size());
		VkCommandBuffer cmdBuff = m_TransferCommandBuffer[m_LastCmdIdxTransfer];
		m_LastCmdIdxTransfer++;
		return cmdBuff;
	}

	VkCommandBuffer FrameData::GetNextComputeCmdBuffer() {
		CKE_ASSERT(m_LastCmdIdxCompute < m_ComputeCommandBuffer.size());
		VkCommandBuffer cmdBuff = m_ComputeCommandBuffer[m_LastCmdIdxCompute];
		m_LastCmdIdxCompute++;
		return cmdBuff;
	}
}
