#include "CookieKat/Core/Containers/Containers.h"
#include "CookieKat/Core/Containers/String.h"
#include "CookieKat/Core/Platform/Asserts.h"
#include "CookieKat/Core/FileSystem/FileSystem.h"

#include <iostream>
#include <optional>

#define NOMINMAX
#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <algorithm>
#include <GLFW/glfw3native.h>

using namespace CKE;

//-----------------------------------------------------------------------------

struct QueueFamilyIndices
{
	Optional<u32> m_GraphicsFamily;
	Optional<u32> m_PresentFamily;
};

struct SwapChainSupportDetails
{
	VkSurfaceCapabilitiesKHR   m_Capabilities{};
	Vector<VkSurfaceFormatKHR> m_Formats{};
	Vector<VkPresentModeKHR>   m_PresentModes{};
};

//-----------------------------------------------------------------------------

constexpr bool g_EnableValidationLayers = true;

GLFWwindow* g_pWindow;

class VRenderer2
{
public:

	// Global Setup
	//-----------------------------------------------------------------------------

	void Initialize();
	void Shutdown();

	// Instance
	//-----------------------------------------------------------------------------

	void CreateInstance(Vector<const char*>& validationLayers, VkDebugUtilsMessengerCreateInfoEXT& msgCreateInfo);

	// Querying and Checks
	bool CheckInstanceValidationLayersSupport(Vector<const char*> const& validationLayers);
	void GetRequiredInstanceExtensions(Vector<const char*>& requiredExtensions);
	void EnumerateExtensionsToConsole(Vector<VkExtensionProperties> const& extensions);

	// Debugging
	void CreateDebugMessenger(VkDebugUtilsMessengerCreateInfoEXT msgCreateInfo);
	void ConfigVkDebugMessenger(VkDebugUtilsMessengerCreateInfoEXT& msgCreateInfo);
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

	void SelectPhysicalDevice(Vector<const char*> const& requiredExtensions);
	void CreateLogicalDevice(Vector<const char*>& validationLayers, Vector<const char*>& requiredExtensions);

	// Querying and Suitability Checks
	bool CheckDeviceSupportsNecessaryExtensions(VkPhysicalDevice device, Vector<const char*> const& deviceExtensions);
	bool IsPhysicalDeviceSuitable(VkPhysicalDevice device, Vector<const char*> const& requiredExtensions);
	SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device);
	QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice physicalDevice);

	// SwapChain
	//-----------------------------------------------------------------------------

	void                    CreateSwapChain();
	void					CreateSwapChainImageViews();
	void RecreateSwapChain();
	void DestroySwapChain();

	// Swapchain Config
	VkSurfaceFormatKHR      SelectSwapSurfaceFormat(Vector<VkSurfaceFormatKHR>& availableFormats);
	VkPresentModeKHR        SelectSwapPresentMode(Vector<VkPresentModeKHR>& availablePresentModes);
	VkExtent2D              SelectSwapExtent(VkSurfaceCapabilitiesKHR& capabilities);

	// Pipeline
	//-----------------------------------------------------------------------------

	void CreateGraphicsPipeline();
	VkShaderModule CreateShaderModule(Vector<u8> const& code);

	// Render Pass & Framebuffer
	//-----------------------------------------------------------------------------

	void CreateRenderPass();
	void CreateFrameBuffer();

	// Commands
	//-----------------------------------------------------------------------------

	void CreateCommandPool();
	void CreateCommandBuffers();

	void RecordCommandBuffer(VkCommandBuffer commandBuffer, u32 imageIndex);

	void CreateSyncObjects();
	void DrawFrame();

	// Accessors
	inline VkDevice GetDevice() const { return m_Device; }

private:
	VkInstance               m_Instance{};
	VkDebugUtilsMessengerEXT m_DebugMessenger{};

	VkSurfaceKHR   m_Surface{};
	VkSwapchainKHR m_SwapChain{};
	VkFormat       m_SwapChainImageFormat{};
	VkExtent2D     m_SwapChainExtent{};

	Vector<VkImage>     m_SwapChainImages{};
	Vector<VkImageView> m_SwapChainImageViews{};
	Vector<VkFramebuffer> m_SwapChainFrameBuffers{};

	VkPhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;
	VkDevice         m_Device{};

	VkQueue m_GraphicsQueue{};
	VkQueue m_PresentQueue{};
	VkCommandPool m_CommandPool{};

	VkRenderPass m_RenderPass{};
	VkPipelineLayout m_PipelineLayout{};
	VkPipeline m_GraphicsPipeline{};

	Vector<VkCommandBuffer> m_CommandBuffers{};
	Vector<VkSemaphore> m_ImageAvailableSemaphores{};
	Vector<VkSemaphore> m_RenderFinishedSemaphores{};
	Vector<VkFence> m_InFlightFences{};

	const i32 MAX_FRAMES_IN_FLIGHT = 2;
	u64 m_CurrentFrame = 0;

public:
	bool m_FrameBufferResized = false;
};

bool VRenderer2::CheckInstanceValidationLayersSupport(Vector<const char*> const& validationLayers)
{
	u32 layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
	Vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	for (const char* validationLayer : validationLayers)
	{
		bool layerFound = false;

		for (auto const& availableLayer : availableLayers)
		{
			if (strcmp(validationLayer, availableLayer.layerName) == 0)
			{
				layerFound = true;
				break;
			}
		}

		if (!layerFound) { return false; }
	}
	return true;
}

void VRenderer2::GetRequiredInstanceExtensions(Vector<const char*>& requiredExtensions)
{
	u32          glfwExtensionCount = 0;
	const char** glfwExtensionNames = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	requiredExtensions.clear();
	requiredExtensions.reserve(glfwExtensionCount);
	for (u32 i = 0; i < glfwExtensionCount; ++i) { requiredExtensions.push_back(glfwExtensionNames[i]); }

	if constexpr (g_EnableValidationLayers) { requiredExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME); }
}

VkResult CreateDebugUtilsMessengerEXT(
	VkInstance                                instance,
	VkDebugUtilsMessengerCreateInfoEXT const* pCreateInfo,
	VkAllocationCallbacks* pAllocator,
	VkDebugUtilsMessengerEXT* pDebugMessenger)
{
	auto func = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
		vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT"));
	if (func == nullptr) { return VK_ERROR_EXTENSION_NOT_PRESENT; }

	return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
}

void DestroyDebugUtilsMessengerEXT(
	VkInstance                   instance,
	VkDebugUtilsMessengerEXT     debugMessenger,
	const VkAllocationCallbacks* pAllocator)
{
	auto func = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
		vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT"));
	if (func == nullptr)
	{
		std::cout << "Function to destroy debug messenger not found" << std::endl;
		return;
	}

	func(instance, debugMessenger, pAllocator);
}

void VRenderer2::EnumerateExtensionsToConsole(Vector<VkExtensionProperties> const& extensions)
{
	std::cout << "-----------------------------------------------------------------------------" << std::endl;
	std::cout << "Available Extensions:" << std::endl;
	std::cout << "-----------------------------------------------------------------------------" << std::endl;
	for (VkExtensionProperties const& extension : extensions)
	{
		std::cout << extension.extensionName << " - " << extension.specVersion << std::endl;
	}
}

VkBool32 VRenderer2::DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT      messageSeverity,
	VkDebugUtilsMessageSeverityFlagsEXT         messageType,
	VkDebugUtilsMessengerCallbackDataEXT const* pCallbackData,
	void* pUserData)
{
	std::cout << "-----------------------------------------------------------------------------" << std::endl;
	std::cout << "Message Type: ";
	switch (messageType)
	{
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

void VRenderer2::CreateSurface()
{
	VkWin32SurfaceCreateInfoKHR createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	createInfo.hwnd = glfwGetWin32Window(g_pWindow);
	createInfo.hinstance = GetModuleHandle(nullptr);

	if (vkCreateWin32SurfaceKHR(m_Instance, &createInfo, nullptr, &m_Surface) != VK_SUCCESS)
	{
		std::cout << "Error creating Win32 surface" << std::endl;
	}
}

void VRenderer2::SelectPhysicalDevice(Vector<const char*> const& requiredExtensions)
{
	// Enumerate all physical devices
	u32 deviceCount = 0;
	vkEnumeratePhysicalDevices(m_Instance, &deviceCount, nullptr);
	if (deviceCount == 0) { std::cout << "Didn't find a physical device with Vulkan support" << std::endl; }
	Vector<VkPhysicalDevice> physicalDevices{ deviceCount };
	vkEnumeratePhysicalDevices(m_Instance, &deviceCount, physicalDevices.data());

	// Iterate over all of them to find a suitable one
	for (VkPhysicalDevice& device : physicalDevices)
	{
		if (IsPhysicalDeviceSuitable(device, requiredExtensions))
		{
			m_PhysicalDevice = device;
			break;
		}
	}

	if (m_PhysicalDevice == VK_NULL_HANDLE) { std::cout << "No suitable physical device found" << std::endl; }
}

bool VRenderer2::CheckDeviceSupportsNecessaryExtensions(VkPhysicalDevice           device,
	Vector<const char*> const& deviceExtensions)
{
	u32 extensionCount;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);
	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

	Set<String> requiredExtensions{ deviceExtensions.begin(), deviceExtensions.end() };
	for (VkExtensionProperties const& extension : availableExtensions)
	{
		requiredExtensions.erase(extension.extensionName);
	}

	return requiredExtensions.empty();
}

bool VRenderer2::IsPhysicalDeviceSuitable(VkPhysicalDevice device, Vector<const char*> const& requiredExtensions)
{
	// Device properties & features
	VkPhysicalDeviceProperties properties{};
	vkGetPhysicalDeviceProperties(device, &properties);
	VkPhysicalDeviceFeatures features{};
	vkGetPhysicalDeviceFeatures(device, &features);

	// Queue Families Support
	QueueFamilyIndices queueFamilyIndices = FindQueueFamilies(device);

	// Extensions Support
	bool extensionsSupported = CheckDeviceSupportsNecessaryExtensions(device, requiredExtensions);

	// SwapChain Support
	bool swapChainAdequate = false;
	if (extensionsSupported)
	{
		SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(device);
		swapChainAdequate = !swapChainSupport.m_Formats.empty() && !swapChainSupport.m_PresentModes.empty();
	}

	return
		properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU &&
		features.geometryShader &&
		queueFamilyIndices.m_GraphicsFamily.has_value() &&
		queueFamilyIndices.m_PresentFamily.has_value() &&
		extensionsSupported &&
		swapChainAdequate;
}

QueueFamilyIndices VRenderer2::FindQueueFamilies(VkPhysicalDevice physicalDevice)
{
	QueueFamilyIndices indices{};

	u32 queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);
	Vector<VkQueueFamilyProperties> queueFamilies{ queueFamilyCount };
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());

	VkBool32 presentSupport = false;

	i32 i = 0;
	for (VkQueueFamilyProperties const& family : queueFamilies)
	{
		// Graphics Family
		if (family.queueFlags & VK_QUEUE_GRAPHICS_BIT) { indices.m_GraphicsFamily = i; }

		// Present Family
		vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, m_Surface, &presentSupport);
		if (presentSupport) { indices.m_PresentFamily = i; }

		i++;
	}

	return indices;
}

void VRenderer2::CreateLogicalDevice(Vector<const char*>& validationLayers, Vector<const char*>& requiredExtensions)
{
	// Queues
	//-----------------------------------------------------------------------------

	QueueFamilyIndices indices = FindQueueFamilies(m_PhysicalDevice);
	Vector<u32>        familiesIndices{ {indices.m_GraphicsFamily.value(), indices.m_PresentFamily.value()} };

	Vector<VkDeviceQueueCreateInfo> queueCreateInfos{};
	f32                             queuePriority = 1.0f;
	for (u32 familyIndex : familiesIndices)
	{
		VkDeviceQueueCreateInfo queueCreateInfo{};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = familyIndex;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &queuePriority;
		queueCreateInfos.emplace_back(queueCreateInfo);
	}

	// Device
	//-----------------------------------------------------------------------------

	VkPhysicalDeviceFeatures deviceFeatures{};
	vkGetPhysicalDeviceFeatures(m_PhysicalDevice, &deviceFeatures);

	VkDeviceCreateInfo deviceCreateInfo{};
	deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceCreateInfo.queueCreateInfoCount = static_cast<u32>(familiesIndices.size());
	deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
	deviceCreateInfo.pEnabledFeatures = &deviceFeatures;


	// NOTE: No longer needed in newer versions of Vulkan

	deviceCreateInfo.enabledExtensionCount = static_cast<u32>(requiredExtensions.size());
	deviceCreateInfo.ppEnabledExtensionNames = requiredExtensions.data();

	// Layers
	//-----------------------------------------------------------------------------

	if constexpr (g_EnableValidationLayers)
	{
		deviceCreateInfo.enabledLayerCount = static_cast<u32>(validationLayers.size());
		deviceCreateInfo.ppEnabledLayerNames = validationLayers.data();
	}
	else { deviceCreateInfo.enabledLayerCount = 0; }

	if (vkCreateDevice(m_PhysicalDevice, &deviceCreateInfo, nullptr, &m_Device) != VK_SUCCESS)
	{
		std::cout << "Error creating logical device" << std::endl;
	}

	// Get Queues
	//-----------------------------------------------------------------------------

	vkGetDeviceQueue(m_Device, indices.m_GraphicsFamily.value(), 0, &m_GraphicsQueue);
	vkGetDeviceQueue(m_Device, indices.m_PresentFamily.value(), 0, &m_PresentQueue);
}

SwapChainSupportDetails VRenderer2::QuerySwapChainSupport(VkPhysicalDevice device)
{
	SwapChainSupportDetails details;

	// Capabilities
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, m_Surface, &details.m_Capabilities);

	// Formats
	u32 formatCount = 0;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_Surface, &formatCount, nullptr);
	if (formatCount > 0)
	{
		details.m_Formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_Surface, &formatCount, details.m_Formats.data());
	}

	// Present Modes
	u32 presentModeCount = 0;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_Surface, &presentModeCount, nullptr);
	if (presentModeCount > 0)
	{
		details.m_PresentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_Surface, &presentModeCount, details.m_PresentModes.data());
	}

	return details;
}

VkSurfaceFormatKHR VRenderer2::SelectSwapSurfaceFormat(Vector<VkSurfaceFormatKHR>& availableFormats)
{
	CKE_ASSERT(availableFormats.size() > 0);

	for (VkSurfaceFormatKHR& format : availableFormats)
	{
		if (format.format == VK_FORMAT_B8G8R8A8_SRGB &&
			format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
		{
			return format;
		}
	}

	return availableFormats[0];
}

VkPresentModeKHR VRenderer2::SelectSwapPresentMode(Vector<VkPresentModeKHR>& availablePresentModes)
{
	for (VkPresentModeKHR& presentMode : availablePresentModes)
	{
		if (presentMode == VK_PRESENT_MODE_MAILBOX_KHR)
		{
			return presentMode;
		}
	}

	return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D VRenderer2::SelectSwapExtent(VkSurfaceCapabilitiesKHR& capabilities)
{
	if (capabilities.currentExtent.width != std::numeric_limits<u32>::max())
	{
		return capabilities.currentExtent;
	}
	else
	{
		i32 width, height;
		glfwGetFramebufferSize(g_pWindow, &width, &height);

		VkExtent2D actualExtent = {
			static_cast<u32>(width),
			static_cast<u32>(height)
		};

		actualExtent.width = std::clamp(actualExtent.width,
			capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
		actualExtent.height = std::clamp(actualExtent.height,
			capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

		return actualExtent;
	}
}

void VRenderer2::CreateSwapChain()
{
	SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(m_PhysicalDevice);

	VkSurfaceFormatKHR surfaceFormat = SelectSwapSurfaceFormat(swapChainSupport.m_Formats);
	VkPresentModeKHR   presentMode = SelectSwapPresentMode(swapChainSupport.m_PresentModes);
	VkExtent2D         extent = SelectSwapExtent(swapChainSupport.m_Capabilities);

	u32       imageCount = swapChainSupport.m_Capabilities.minImageCount + 1;
	u32 const maxCount = swapChainSupport.m_Capabilities.maxImageCount;
	if (maxCount > 0 && imageCount > maxCount) { imageCount = maxCount; }

	VkSwapchainCreateInfoKHR createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = m_Surface;

	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = extent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	QueueFamilyIndices indices = FindQueueFamilies(m_PhysicalDevice);
	u32                queueFamilyIndices[] = { indices.m_GraphicsFamily.value(), indices.m_PresentFamily.value() };

	if (indices.m_GraphicsFamily != indices.m_PresentFamily)
	{
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
	}
	else
	{
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	}

	createInfo.preTransform = swapChainSupport.m_Capabilities.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

	createInfo.presentMode = presentMode;
	createInfo.clipped = true;

	createInfo.oldSwapchain = VK_NULL_HANDLE;

	if (vkCreateSwapchainKHR(m_Device, &createInfo, nullptr, &m_SwapChain) != VK_SUCCESS)
	{
		std::cout << "Error creating SwapChain" << std::endl;
	}

	vkGetSwapchainImagesKHR(m_Device, m_SwapChain, &imageCount, nullptr);
	m_SwapChainImages.resize(imageCount);
	vkGetSwapchainImagesKHR(m_Device, m_SwapChain, &imageCount, m_SwapChainImages.data());

	m_SwapChainImageFormat = surfaceFormat.format;
	m_SwapChainExtent = extent;
}

void VRenderer2::CreateSwapChainImageViews()
{
	m_SwapChainImageViews.resize(m_SwapChainImages.size());

	for (int i = 0; i < m_SwapChainImages.size(); ++i)
	{
		VkImageViewCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.image = m_SwapChainImages[i];
		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		createInfo.format = m_SwapChainImageFormat;
		createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

		createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1;

		if (vkCreateImageView(m_Device, &createInfo, nullptr, &m_SwapChainImageViews[i]) != VK_SUCCESS)
		{
			std::cout << "Error creating image view" << std::endl;
		}
	}
}

void VRenderer2::RecreateSwapChain()
{
	i32 w = 0;
	i32 h = 0;
	glfwGetFramebufferSize(g_pWindow, &w, &h);

	while (w == 0 || h == 0)
	{
		glfwGetFramebufferSize(g_pWindow, &w, &h);
		glfwWaitEvents();
	}

	vkDeviceWaitIdle(m_Device);

	DestroySwapChain();

	CreateSwapChain();
	CreateSwapChainImageViews();
	CreateFrameBuffer();
}

void VRenderer2::CreateGraphicsPipeline()
{
	// Shader Stages
	//-----------------------------------------------------------------------------

	auto vertShaderCode = g_FileSystem.ReadBinaryFile("Shaders/vert.spv");
	auto fragShaderCode = g_FileSystem.ReadBinaryFile("Shaders/frag.spv");

	auto vertShaderModule = CreateShaderModule(vertShaderCode);
	auto fragShaderModule = CreateShaderModule(fragShaderCode);

	VkPipelineShaderStageCreateInfo vertShaderStageCreateInfo{};
	vertShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertShaderStageCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertShaderStageCreateInfo.module = vertShaderModule;
	vertShaderStageCreateInfo.pName = "main";

	VkPipelineShaderStageCreateInfo fragShaderStageCreateInfo{};
	fragShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragShaderStageCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragShaderStageCreateInfo.module = fragShaderModule;
	fragShaderStageCreateInfo.pName = "main";

	VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageCreateInfo, fragShaderStageCreateInfo };

	// Dynamic State
	//-----------------------------------------------------------------------------

	Vector<VkDynamicState> dynamicStates = {
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR,
	};

	VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo{};
	dynamicStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicStateCreateInfo.dynamicStateCount = static_cast<u32>(dynamicStates.size());
	dynamicStateCreateInfo.pDynamicStates = dynamicStates.data();

	// Vertex Input
	//-----------------------------------------------------------------------------

	VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount = 0;
	vertexInputInfo.pVertexBindingDescriptions = nullptr;
	vertexInputInfo.vertexAttributeDescriptionCount = 0;
	vertexInputInfo.pVertexAttributeDescriptions = nullptr;

	// Input Assembly
	//-----------------------------------------------------------------------------

	VkPipelineInputAssemblyStateCreateInfo inputAssemblyCreateInfo{};
	inputAssemblyCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssemblyCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssemblyCreateInfo.primitiveRestartEnable = VK_FALSE;

	// Viewports & Scissors
	//-----------------------------------------------------------------------------

	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = static_cast<f32>(m_SwapChainExtent.width);
	viewport.height = static_cast<f32>(m_SwapChainExtent.height);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor{};
	scissor.offset = { 0, 0 };
	scissor.extent = m_SwapChainExtent;

	VkPipelineViewportStateCreateInfo viewportCreateInfo{};
	viewportCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportCreateInfo.viewportCount = 1;
	viewportCreateInfo.scissorCount = 1;

	// Rasterizer
	//-----------------------------------------------------------------------------

	VkPipelineRasterizationStateCreateInfo rasterizer{};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
	rasterizer.depthBiasEnable = VK_FALSE;

	// Multisampling
	//-----------------------------------------------------------------------------

	VkPipelineMultisampleStateCreateInfo multisampling{};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

	// Color Blending
	//-----------------------------------------------------------------------------

	VkPipelineColorBlendAttachmentState colorBlendAttachment{};
	colorBlendAttachment.colorWriteMask =
		VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_TRUE;

	colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;

	colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

	VkPipelineColorBlendStateCreateInfo colorBlending{};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = VK_LOGIC_OP_COPY;
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &colorBlendAttachment;
	colorBlending.blendConstants[0] = 0.0f;
	colorBlending.blendConstants[1] = 0.0f;
	colorBlending.blendConstants[2] = 0.0f;
	colorBlending.blendConstants[3] = 0.0f;

	// Pipeline Layout
	//-----------------------------------------------------------------------------

	VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
	pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutCreateInfo.setLayoutCount = 0;
	pipelineLayoutCreateInfo.pSetLayouts = nullptr;
	pipelineLayoutCreateInfo.pushConstantRangeCount = 0;
	pipelineLayoutCreateInfo.pPushConstantRanges = nullptr;

	if (vkCreatePipelineLayout(m_Device, &pipelineLayoutCreateInfo, nullptr, &m_PipelineLayout) != VK_SUCCESS)
	{
		std::cout << "Error creating pipeline layout" << std::endl;
	}

	// Pipeline
	//-----------------------------------------------------------------------------

	VkGraphicsPipelineCreateInfo pipelineCreateInfo{};
	pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineCreateInfo.stageCount = 2;
	pipelineCreateInfo.pStages = shaderStages;

	pipelineCreateInfo.pVertexInputState = &vertexInputInfo;
	pipelineCreateInfo.pInputAssemblyState = &inputAssemblyCreateInfo;
	pipelineCreateInfo.pViewportState = &viewportCreateInfo;
	pipelineCreateInfo.pRasterizationState = &rasterizer;
	pipelineCreateInfo.pMultisampleState = &multisampling;
	pipelineCreateInfo.pDepthStencilState = nullptr;
	pipelineCreateInfo.pColorBlendState = &colorBlending;
	pipelineCreateInfo.pDynamicState = &dynamicStateCreateInfo;

	pipelineCreateInfo.layout = m_PipelineLayout;

	pipelineCreateInfo.renderPass = m_RenderPass;
	pipelineCreateInfo.subpass = 0;

	pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
	pipelineCreateInfo.basePipelineIndex = -1;

	if (vkCreateGraphicsPipelines(m_Device, VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr
		, &m_GraphicsPipeline) != VK_SUCCESS)
	{
		std::cout << "Error creating graphics pipeline" << std::endl;
	}

	// Cleanup
	//-----------------------------------------------------------------------------

	vkDestroyShaderModule(m_Device, vertShaderModule, nullptr);
	vkDestroyShaderModule(m_Device, fragShaderModule, nullptr);
}

VkShaderModule VRenderer2::CreateShaderModule(Vector<u8> const& code)
{
	VkShaderModuleCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = code.size();
	createInfo.pCode = reinterpret_cast<u32 const*>(code.data());

	VkShaderModule shaderModule{};
	if (vkCreateShaderModule(m_Device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
	{
		std::cout << "Error creating shader module" << std::endl;
	}
	return shaderModule;
}

void VRenderer2::CreateRenderPass()
{
	VkAttachmentDescription colorAttachment{};
	colorAttachment.format = m_SwapChainImageFormat;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;

	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference colorAttachmentRef{};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass{};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;

	VkSubpassDependency dependency{};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;

	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = 0;

	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	VkRenderPassCreateInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = 1;
	renderPassInfo.pAttachments = &colorAttachment;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &dependency;

	if (vkCreateRenderPass(m_Device, &renderPassInfo, nullptr, &m_RenderPass) != VK_SUCCESS)
	{
		std::cout << "Error creating render pass" << std::endl;
	}
}

void VRenderer2::CreateFrameBuffer()
{
	m_SwapChainFrameBuffers.resize(m_SwapChainImageViews.size());

	for (usize i = 0; i < m_SwapChainImageViews.size(); ++i)
	{
		VkImageView attachments[] = { m_SwapChainImageViews[i] };

		VkFramebufferCreateInfo framebufferInfo{};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = m_RenderPass;
		framebufferInfo.attachmentCount = 1;
		framebufferInfo.pAttachments = attachments;
		framebufferInfo.width = m_SwapChainExtent.width;
		framebufferInfo.height = m_SwapChainExtent.height;
		framebufferInfo.layers = 1;

		if (vkCreateFramebuffer(m_Device, &framebufferInfo, nullptr, &m_SwapChainFrameBuffers[i]) != VK_SUCCESS)
		{
			std::cout << "Error creating framebuffer" << std::endl;
		}
	}
}

void VRenderer2::CreateCommandPool()
{
	QueueFamilyIndices queueFamilyIndices = FindQueueFamilies(m_PhysicalDevice);

	VkCommandPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	poolInfo.queueFamilyIndex = queueFamilyIndices.m_GraphicsFamily.value();

	if (vkCreateCommandPool(m_Device, &poolInfo, nullptr, &m_CommandPool) != VK_SUCCESS)
	{
		std::cout << "Error creating command pool" << std::endl;
	}
}

void VRenderer2::CreateCommandBuffers()
{
	m_CommandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = m_CommandPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = 2;

	if (vkAllocateCommandBuffers(m_Device, &allocInfo, m_CommandBuffers.data()) != VK_SUCCESS)
	{
		std::cout << "Error creating command buffer" << std::endl;
	}
}

void VRenderer2::RecordCommandBuffer(VkCommandBuffer commandBuffer, u32 imageIndex)
{
	// Prepare Command buffer
	//-----------------------------------------------------------------------------

	VkCommandBufferBeginInfo commandBufferBeginInfo{};
	commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	commandBufferBeginInfo.flags = 0;
	commandBufferBeginInfo.pInheritanceInfo = nullptr;

	if (vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo) != VK_SUCCESS)
	{
		std::cout << "Error beginning to record command buffer" << std::endl;
	}

	VkRenderPassBeginInfo renderPassBeginInfo{};
	renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassBeginInfo.renderPass = m_RenderPass;
	renderPassBeginInfo.framebuffer = m_SwapChainFrameBuffers[imageIndex];

	renderPassBeginInfo.renderArea.offset = { 0,0 };
	renderPassBeginInfo.renderArea.extent = m_SwapChainExtent;

	VkClearValue clearColor = { {{1.0f, 0.7f, 0.0f, 1.0f}} };
	renderPassBeginInfo.clearValueCount = 1;
	renderPassBeginInfo.pClearValues = &clearColor;

	// Commands
	//-----------------------------------------------------------------------------

	vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_GraphicsPipeline);

	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = static_cast<f32>(m_SwapChainExtent.width);
	viewport.height = static_cast<f32>(m_SwapChainExtent.height);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

	VkRect2D scissor{};
	scissor.offset = { 0, 0 };
	scissor.extent = m_SwapChainExtent;
	vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
	vkCmdDraw(commandBuffer, 3, 1, 0, 0);

	vkCmdEndRenderPass(commandBuffer);

	if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
	{
		std::cout << "Error ending command buffer" << std::endl;
	}

}

void VRenderer2::CreateSyncObjects()
{
	m_ImageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	m_RenderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	m_InFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

	VkSemaphoreCreateInfo semaphoreInfo{};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fenceInfo{};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
	{
		if (vkCreateSemaphore(m_Device, &semaphoreInfo, nullptr, &m_ImageAvailableSemaphores[i]) != VK_SUCCESS ||
			vkCreateSemaphore(m_Device, &semaphoreInfo, nullptr, &m_RenderFinishedSemaphores[i]) != VK_SUCCESS ||
			vkCreateFence(m_Device, &fenceInfo, nullptr, &m_InFlightFences[i]) != VK_SUCCESS)
		{
			std::cout << "Error creating sync primitives" << std::endl;
		}
	}
}

void VRenderer2::DrawFrame()
{
	vkWaitForFences(m_Device, 1, &m_InFlightFences[m_CurrentFrame], VK_TRUE, UINT64_MAX);

	u32 imageIndex = 0;
	VkResult result = vkAcquireNextImageKHR(m_Device, m_SwapChain, UINT64_MAX, m_ImageAvailableSemaphores[m_CurrentFrame], VK_NULL_HANDLE, &imageIndex);

	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_FrameBufferResized)
	{
		m_FrameBufferResized = false;
		RecreateSwapChain();
		return;
	}
	else if (result != VK_SUCCESS)
	{
		std::cout << "Error acquiring SwapChain image" << std::endl;
	}

	vkResetFences(m_Device, 1, &m_InFlightFences[m_CurrentFrame]);

	vkResetCommandBuffer(m_CommandBuffers[m_CurrentFrame], 0);
	RecordCommandBuffer(m_CommandBuffers[m_CurrentFrame], imageIndex);

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	VkSemaphore waitSemaphores[] = { m_ImageAvailableSemaphores[m_CurrentFrame] };
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;

	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &m_CommandBuffers[m_CurrentFrame];

	VkSemaphore signalSemaphores[] = { m_RenderFinishedSemaphores[m_CurrentFrame] };
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	if (vkQueueSubmit(m_GraphicsQueue, 1, &submitInfo, m_InFlightFences[m_CurrentFrame]) != VK_SUCCESS)
	{
		std::cout << "Error submiting graphics queue command buffer" << std::endl;
	}

	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;

	VkSwapchainKHR swapChains[] = { m_SwapChain };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;
	presentInfo.pImageIndices = &imageIndex;

	presentInfo.pResults = nullptr;

	vkQueuePresentKHR(m_PresentQueue, &presentInfo);

	m_CurrentFrame = (m_CurrentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void VRenderer2::ConfigVkDebugMessenger(VkDebugUtilsMessengerCreateInfoEXT& msgCreateInfo)
{
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

void VRenderer2::CreateInstance(Vector<const char*>& validationLayers, VkDebugUtilsMessengerCreateInfoEXT& msgCreateInfo)
{
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

	validationLayers = { "VK_LAYER_KHRONOS_validation" };
	msgCreateInfo = {};

	if constexpr (g_EnableValidationLayers)
	{
		// Check layer support
		if (CheckInstanceValidationLayersSupport(validationLayers))
		{
			createInfo.enabledLayerCount = static_cast<u32>(validationLayers.size());
			createInfo.ppEnabledLayerNames = validationLayers.data();
		}
		else
		{
			std::cout << "Error, validation layers are not supported" << std::endl;
		}

		// Debug messenger
		ConfigVkDebugMessenger(msgCreateInfo);

		createInfo.pNext = static_cast<VkDebugUtilsMessengerCreateInfoEXT*>(&msgCreateInfo);
	}
	else
	{
		createInfo.enabledLayerCount = 0;
	}



	VkResult result = vkCreateInstance(&createInfo, nullptr, &m_Instance);
	if (result != VK_SUCCESS)
	{
		std::cout << "Error creating vulkan instance" << std::endl;
		if (result == VK_ERROR_EXTENSION_NOT_PRESENT) { std::cout << "Extensions not present" << std::endl; }
		else if (result == VK_ERROR_LAYER_NOT_PRESENT) { std::cout << "Layer not present" << std::endl; }
		else { std::cout << "Unhandled error" << std::endl; }
	}
}

void VRenderer2::CreateDebugMessenger(VkDebugUtilsMessengerCreateInfoEXT msgCreateInfo)
{
	if (g_EnableValidationLayers)
	{
		VkResult msgResult = CreateDebugUtilsMessengerEXT(m_Instance, &msgCreateInfo, nullptr, &m_DebugMessenger);
		if (msgResult != VK_SUCCESS) { std::cout << "Error creating debug messenger" << std::endl; }
	}
}

void VRenderer2::Initialize()
{
	Vector<const char*>                validationLayers;
	VkDebugUtilsMessengerCreateInfoEXT msgCreateInfo;

	CreateInstance(validationLayers, msgCreateInfo);
	CreateDebugMessenger(msgCreateInfo);
	CreateSurface();

	Vector<const char*> requiredDeviceExtensions = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

	SelectPhysicalDevice(requiredDeviceExtensions);
	CreateLogicalDevice(validationLayers, requiredDeviceExtensions);

	CreateSwapChain();
	CreateSwapChainImageViews();

	CreateRenderPass();
	CreateGraphicsPipeline();
	CreateFrameBuffer();
	CreateCommandPool();
	CreateCommandBuffers();
	CreateSyncObjects();
}

void VRenderer2::DestroySwapChain()
{
	for (VkFramebuffer_T* framebuffer : m_SwapChainFrameBuffers)
	{
		vkDestroyFramebuffer(m_Device, framebuffer, nullptr);
	}

	for (VkImageView_T* swapChainImageView : m_SwapChainImageViews)
	{
		vkDestroyImageView(m_Device, swapChainImageView, nullptr);
	}

	vkDestroySwapchainKHR(m_Device, m_SwapChain, nullptr);
}

void VRenderer2::Shutdown()
{
	DestroySwapChain();

	// Destroy Sync Primitives
	for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
	{
		vkDestroySemaphore(m_Device, m_ImageAvailableSemaphores[i], nullptr);
		vkDestroySemaphore(m_Device, m_RenderFinishedSemaphores[i], nullptr);
		vkDestroyFence(m_Device, m_InFlightFences[i], nullptr);
	}

	vkDestroyRenderPass(m_Device, m_RenderPass, nullptr);

	vkDestroyCommandPool(m_Device, m_CommandPool, nullptr);

	vkDestroyPipeline(m_Device, m_GraphicsPipeline, nullptr);
	vkDestroyPipelineLayout(m_Device, m_PipelineLayout, nullptr);

	vkDestroyDevice(m_Device, nullptr);
	vkDestroySurfaceKHR(m_Instance, m_Surface, nullptr);

	if (g_EnableValidationLayers) { DestroyDebugUtilsMessengerEXT(m_Instance, m_DebugMessenger, nullptr); }
	vkDestroyInstance(m_Instance, nullptr);
}

//-----------------------------------------------------------------------------

static void GLFW_FrameBufferResizeCallback(GLFWwindow* pWindow, i32 width, i32 height)
{
	auto i = reinterpret_cast<VRenderer2*>(glfwGetWindowUserPointer(g_pWindow));
	i->m_FrameBufferResized = true;
}

int main()
{
	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	g_pWindow = glfwCreateWindow(1280, 720, "Vulkanik CookieKat", nullptr, nullptr);
	glfwSetFramebufferSizeCallback(g_pWindow, GLFW_FrameBufferResizeCallback);

	VRenderer2 vulkanInstance{};
	glfwSetWindowUserPointer(g_pWindow, &vulkanInstance);

	vulkanInstance.Initialize();

	while (!glfwWindowShouldClose(g_pWindow))
	{
		glfwPollEvents();
		vulkanInstance.DrawFrame();
	}

	vkDeviceWaitIdle(vulkanInstance.GetDevice());

	vulkanInstance.Shutdown();

	glfwDestroyWindow(g_pWindow);
	glfwTerminate();

	return 0;
}
