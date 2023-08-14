#pragma once

#include "TestPrimitives.h"

#include "CookieKat/Core/Containers/Containers.h"
#include "CookieKat/Core/Platform/PrimitiveTypes.h"

#define NOMINMAX
#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#include <optional>

//-----------------------------------------------------------------------------

using namespace CKE;

struct QueueFamilyIndices
{
	Optional<u32> m_GraphicsFamily;
	Optional<u32> m_PresentFamily;
	Optional<u32> m_TransferFamily;
};

struct SwapChainSupportDetails
{
	VkSurfaceCapabilitiesKHR   m_Capabilities;
	Vector<VkSurfaceFormatKHR> m_Formats;
	Vector<VkPresentModeKHR>   m_PresentModes;
};

struct SwapChainData
{
	VkSwapchainKHR m_SwapChain{};
	VkFormat       m_ImageFormat{};
	VkExtent2D     m_Extent{};

	Vector<VkImage>       m_Images{};
	Vector<VkImageView>   m_ImageViews{};
	Vector<VkFramebuffer> m_FrameBuffers{};
	bool                  m_FrameBufferResized = false;
};

class FrameData
{
	friend class VulkanDevice;
public:
	FrameData()
	{
		m_ImageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
		m_RenderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
		m_InFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

		m_GraphicsCommandBuffers.resize(MAX_FRAMES_IN_FLIGHT);
		m_TransferCommandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

		m_DescriptorSets.resize(MAX_FRAMES_IN_FLIGHT);
	}

	inline VkSemaphore GetImageAvailableSemaphore() { return m_ImageAvailableSemaphores[m_Idx]; }
	inline VkSemaphore GetRenderFinishedSemaphore() { return m_RenderFinishedSemaphores[m_Idx]; }
	inline VkFence GetInFlightFence() { return m_InFlightFences[m_Idx]; }

	inline VkCommandBuffer GetGraphicsCommandBuffer() { return m_GraphicsCommandBuffers[m_Idx]; }
	inline VkCommandBuffer GetTransferCommandBuffer() { return m_TransferCommandBuffers[m_Idx]; }

	inline VkDescriptorSet GetDescriptorSet() { return m_DescriptorSets[m_Idx]; }

	const i32 MAX_FRAMES_IN_FLIGHT = 2;

private:
	Vector<VkSemaphore> m_ImageAvailableSemaphores{};
	Vector<VkSemaphore> m_RenderFinishedSemaphores{};
	Vector<VkFence>     m_InFlightFences{};

	Vector<VkCommandBuffer> m_GraphicsCommandBuffers{};
	Vector<VkCommandBuffer> m_TransferCommandBuffers{};

	Vector<VkDescriptorSet> m_DescriptorSets{};

	u64       m_Idx = 0;
};

struct PushConstants
{
	Mat4 MVP;
};