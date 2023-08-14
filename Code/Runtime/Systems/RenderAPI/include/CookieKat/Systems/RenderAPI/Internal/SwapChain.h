#pragma once
#include <vulkan/vulkan_core.h>

#include "CookieKat/Systems/RenderAPI/RenderHandle.h"
#include "CookieKat/Systems/RenderAPI/Texture.h"
#include "CookieKat/Core/Containers/Containers.h"
#include "CookieKat/Core/Math/Math.h"

namespace CKE {
	struct SwapchainInitialConfig
	{
		// SwapChain Config
		static VkSurfaceFormatKHR SelectSwapSurfaceFormat(Vector<VkSurfaceFormatKHR>& availableFormats);
		static VkPresentModeKHR   SelectSwapPresentMode(Vector<VkPresentModeKHR>& availablePresentModes);
		static VkExtent2D         SelectSwapExtent(VkSurfaceCapabilitiesKHR& capabilities, Int2 framebufferSize);
	};

	struct SwapChainSupportDetails
	{
		VkSurfaceCapabilitiesKHR   m_Capabilities;
		Vector<VkSurfaceFormatKHR> m_Formats;
		Vector<VkPresentModeKHR>   m_PresentModes;
	};

	// Contains all of the data regarding the SwapChain
	class SwapChain
	{
	public:
		VkSwapchainKHR m_vkSwapChain = VK_NULL_HANDLE;
		VkFormat       m_vkImageFormat{};
		VkExtent2D     m_vkExtent{};

		u32                       m_CurrentTextureIdx;
		Vector<TextureHandle>     m_Textures;
		Vector<TextureViewHandle> m_Views;
		TextureDesc               m_Desc;

		bool m_FrameBufferResized = false;
		Int2 m_NewFramebufferSize;
	};
}
