#include "VulkanDevice.h"

#include <algorithm>
#include <iostream>

VkSurfaceFormatKHR SwapchainInitialConfig::SelectSwapSurfaceFormat(Vector<VkSurfaceFormatKHR>& availableFormats)
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

VkPresentModeKHR SwapchainInitialConfig::SelectSwapPresentMode(Vector<VkPresentModeKHR>& availablePresentModes)
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

VkExtent2D SwapchainInitialConfig::SelectSwapExtent(VkSurfaceCapabilitiesKHR& capabilities)
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

void VulkanDevice::Initialize()
{
	CreateSwapChain();
	CreateSwapChainImageViews(m_Device);

	CreateRenderPass();

	CreateDescriptorSetLayout();
	CreateDescriptorPool();
	CreateDescriptorSets();

	CreateFrameBufferForRenderPass();

	// Create Command Pools
	CreateGraphicsCommandPool();
	CreateTransferCommandPool();

	CreateGraphicsCommandBuffers();
	CreateTransferCommandBuffers();

	CreateFrameSyncObjects();
}

void VulkanDevice::Shutdown()
{
	DestroySwapChain();

	DestroyDescriptorSetLayout();
	DestroyDescriptorPool();

	DestroyFrameSyncObjects();
	DestroyRenderPass();
	DestroyAllCommandPools();
	vkDestroyDevice(m_Device, nullptr);
}

void VulkanDevice::CreateSwapChain()
{
	SwapChainSupportDetails swapChainSupport = m_Backend->QuerySwapChainSupport(m_PhysicalDevice);

	VkSurfaceFormatKHR surfaceFormat = SwapchainInitialConfig::SelectSwapSurfaceFormat(swapChainSupport.m_Formats);
	VkPresentModeKHR   presentMode = SwapchainInitialConfig::SelectSwapPresentMode(swapChainSupport.m_PresentModes);
	VkExtent2D         extent = SwapchainInitialConfig::SelectSwapExtent(swapChainSupport.m_Capabilities);

	u32       imageCount = swapChainSupport.m_Capabilities.minImageCount + 1;
	u32 const maxCount = swapChainSupport.m_Capabilities.maxImageCount;
	if (maxCount > 0 && imageCount > maxCount) { imageCount = maxCount; }

	VkSwapchainCreateInfoKHR createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = m_Backend->GetSurface();

	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = extent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	QueueFamilyIndices indices = m_Backend->FindQueueFamilies(m_PhysicalDevice);
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

	if (vkCreateSwapchainKHR(m_Device, &createInfo, nullptr, &m_SwapChain.m_SwapChain) != VK_SUCCESS)
	{
		std::cout << "Error creating SwapChain" << std::endl;
	}

	vkGetSwapchainImagesKHR(m_Device, m_SwapChain.m_SwapChain, &imageCount, nullptr);
	m_SwapChain.m_Images.resize(imageCount);
	vkGetSwapchainImagesKHR(m_Device, m_SwapChain.m_SwapChain, &imageCount, m_SwapChain.m_Images.data());

	m_SwapChain.m_ImageFormat = surfaceFormat.format;
	m_SwapChain.m_Extent = extent;
}

void VulkanDevice::CreateSwapChainImageViews(VkDevice device)
{
	m_SwapChain.m_ImageViews.resize(m_SwapChain.m_Images.size());

	for (int i = 0; i < m_SwapChain.m_Images.size(); ++i)
	{
		VkImageViewCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.image = m_SwapChain.m_Images[i];
		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		createInfo.format = m_SwapChain.m_ImageFormat;
		createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

		createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1;

		if (vkCreateImageView(device, &createInfo, nullptr, &m_SwapChain.m_ImageViews[i]) != VK_SUCCESS)
		{
			std::cout << "Error creating image view" << std::endl;
		}
	}
}

void VulkanDevice::RecreateSwapChain(VkDevice device)
{
	i32 w = 0;
	i32 h = 0;
	glfwGetFramebufferSize(g_pWindow, &w, &h);

	while (w == 0 || h == 0)
	{
		glfwGetFramebufferSize(g_pWindow, &w, &h);
		glfwWaitEvents();
	}

	vkDeviceWaitIdle(device);

	DestroySwapChain();

	CreateSwapChain();
	CreateSwapChainImageViews(device);
	CreateFrameBufferForRenderPass();
}

Pipeline VulkanDevice::CreatePipeline()
{
	Pipeline p{};

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

	auto bindingDesc = Vertex::GetBindingDesc();
	auto attributeDesc = Vertex::GetAttributeDesc();

	VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount = 1;
	vertexInputInfo.pVertexBindingDescriptions = &bindingDesc;
	vertexInputInfo.vertexAttributeDescriptionCount = static_cast<u32>(attributeDesc.size());
	vertexInputInfo.pVertexAttributeDescriptions = attributeDesc.data();

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
	viewport.width = static_cast<f32>(m_SwapChain.m_Extent.width);
	viewport.height = static_cast<f32>(m_SwapChain.m_Extent.height);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor{};
	scissor.offset = { 0, 0 };
	scissor.extent = m_SwapChain.m_Extent;

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

	// Multi-sampling
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

	VkPushConstantRange pushConstantRange{};
	pushConstantRange.size = sizeof(Mat4);
	pushConstantRange.offset = 0;
	pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

	VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
	pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutCreateInfo.setLayoutCount = 1;
	pipelineLayoutCreateInfo.pSetLayouts = &m_DescriptorSetLayout;
	pipelineLayoutCreateInfo.pushConstantRangeCount = 1;
	pipelineLayoutCreateInfo.pPushConstantRanges = &pushConstantRange;

	if (vkCreatePipelineLayout(m_Device, &pipelineLayoutCreateInfo, nullptr, &p.m_Layout) != VK_SUCCESS)
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
		, &p.m_Pipeline) != VK_SUCCESS)
	{
		std::cout << "Error creating graphics pipeline" << std::endl;
	}

	// Cleanup
	//-----------------------------------------------------------------------------

	vkDestroyShaderModule(m_Device, vertShaderModule, nullptr);
	vkDestroyShaderModule(m_Device, fragShaderModule, nullptr);

	return p;
}

void VulkanDevice::DestroyPipeline(Pipeline pipeline)
{
	vkDestroyPipeline(m_Device, pipeline.m_Pipeline, nullptr);
	vkDestroyPipelineLayout(m_Device, pipeline.m_Layout, nullptr);
}

VkShaderModule VulkanDevice::CreateShaderModule(Vector<u8> const& code)
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

void VulkanDevice::CreateRenderPass()
{
	VkAttachmentDescription colorAttachment{};
	colorAttachment.format = m_SwapChain.m_ImageFormat;
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

void VulkanDevice::CreateFrameBufferForRenderPass()
{
	m_SwapChain.m_FrameBuffers.resize(m_SwapChain.m_ImageViews.size());

	for (usize i = 0; i < m_SwapChain.m_ImageViews.size(); ++i)
	{
		VkImageView attachments[] = { m_SwapChain.m_ImageViews[i] };

		VkFramebufferCreateInfo framebufferInfo{};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = m_RenderPass;
		framebufferInfo.attachmentCount = 1;
		framebufferInfo.pAttachments = attachments;
		framebufferInfo.width = m_SwapChain.m_Extent.width;
		framebufferInfo.height = m_SwapChain.m_Extent.height;
		framebufferInfo.layers = 1;

		if (vkCreateFramebuffer(m_Device, &framebufferInfo, nullptr, &m_SwapChain.m_FrameBuffers[i]) != VK_SUCCESS)
		{
			std::cout << "Error creating framebuffer" << std::endl;
		}
	}
}

void VulkanDevice::CreateGraphicsCommandBuffers()
{
	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = m_GraphicsCommandPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = m_Frame.MAX_FRAMES_IN_FLIGHT;

	if (vkAllocateCommandBuffers(m_Device, &allocInfo, m_Frame.m_GraphicsCommandBuffers.data()) != VK_SUCCESS)
	{
		std::cout << "Error creating command buffer" << std::endl;
	}
}

Buffer VulkanDevice::CreateBuffer(VkDeviceSize bufferSize, VkBufferUsageFlags bufferUsage, int memoryProperties)
{
	VkBuffer buffer;
	VkDeviceMemory bufferMemory;

	// Create Buffer
	VkBufferCreateInfo bufferInfo{};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = bufferSize;
	bufferInfo.usage = bufferUsage;
	bufferInfo.sharingMode = VK_SHARING_MODE_CONCURRENT;
	bufferInfo.flags = 0;

	u32 sharedFamilies[] = {
		m_FamilyIndices.m_GraphicsFamily.value(),
		m_FamilyIndices.m_TransferFamily.value()
	};
	bufferInfo.queueFamilyIndexCount = 2;
	bufferInfo.pQueueFamilyIndices = sharedFamilies;

	buffer = {};
	if (vkCreateBuffer(m_Device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS)
	{
		std::cout << "Oh no" << std::endl;
	}

	// Get Memory requirements
	VkMemoryRequirements memoryRequirements;
	vkGetBufferMemoryRequirements(m_Device, buffer, &memoryRequirements);

	bufferMemory = {};
	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memoryRequirements.size;
	allocInfo.memoryTypeIndex = FindMemoryType(memoryRequirements.memoryTypeBits, memoryProperties);
	if (vkAllocateMemory(m_Device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS)
	{
		CKE_UNREACHABLE_CODE();
	}

	// Bind memory to buffer
	vkBindBufferMemory(m_Device, buffer, bufferMemory, 0);

	Buffer b{};
	b.m_Buffer = buffer;
	b.m_Memory = bufferMemory;
	b.m_Size = bufferSize;
	return b;
}

void VulkanDevice::DestroyBuffer(Buffer& buffer)
{
	vkDestroyBuffer(m_Device, buffer.m_Buffer, nullptr);
	vkFreeMemory(m_Device, buffer.m_Memory, nullptr);
}

void VulkanDevice::CreateUniformBuffer(u64 uboSize)
{
	VkBufferUsageFlags bufferUsage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	VkMemoryPropertyFlags memoryProperties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	Vector<Buffer> uniformBuffers;
	uniformBuffers.push_back(CreateBuffer(uboSize, bufferUsage, memoryProperties));
	uniformBuffers.push_back(CreateBuffer(uboSize, bufferUsage, memoryProperties));

	Vector<void*> uniformBuffersMapped{ 2 };
	MapMemory(uniformBuffers[0].m_Memory, 0, uboSize, &uniformBuffersMapped[0]);
	MapMemory(uniformBuffers[1].m_Memory, 0, uboSize, &uniformBuffersMapped[1]);

	for (int i = 0; i < m_Frame.MAX_FRAMES_IN_FLIGHT; ++i)
	{
		VkDescriptorBufferInfo bufferInfo{};
		bufferInfo.buffer = uniformBuffers[i].m_Buffer;
		bufferInfo.offset = 0;
		bufferInfo.range = uboSize;

		VkWriteDescriptorSet descriptorWrite{};
		descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrite.dstSet = m_Frame.m_DescriptorSets[i];
		descriptorWrite.dstBinding = 0;
		descriptorWrite.dstArrayElement = 0;
		descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrite.descriptorCount = 1;

		descriptorWrite.pBufferInfo = &bufferInfo;

		vkUpdateDescriptorSets(m_Device, 1, &descriptorWrite, 0, nullptr);
	}
}

VkImage VulkanDevice::CreateTexture(u8* textureData, u32 width, u32 height)
{
	u64 texSize = width * height * 4;

	VkBufferUsageFlags bufferUsage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
	VkMemoryPropertyFlags memoryProperties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	Buffer stagingBuffer = CreateBuffer(texSize, bufferUsage, memoryProperties);

	void* texMappedMem;
	MapMemory(stagingBuffer.m_Memory, 0, texSize, &texMappedMem);
	memcpy(texMappedMem, textureData, texSize);
	UnmapMemory(stagingBuffer.m_Memory);

	// Create a final device texture and transfer it
	VkImageCreateInfo imageInfo{};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.extent.width = width;
	imageInfo.extent.height = height;
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = 1;
	imageInfo.arrayLayers = 1;
	imageInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
	imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	// We use concurrent because we have a transfer queue from a different family
	imageInfo.sharingMode = VK_SHARING_MODE_CONCURRENT;
	u32 familyIndices[] = {
		m_FamilyIndices.m_GraphicsFamily.value(),
		m_FamilyIndices.m_TransferFamily.value()
	};
	imageInfo.queueFamilyIndexCount = 2;
	imageInfo.pQueueFamilyIndices = familyIndices;
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageInfo.flags = 0;
	VkImage image;
	if (vkCreateImage(m_Device, &imageInfo, nullptr, &image) != VK_SUCCESS)
	{
		CKE_UNREACHABLE_CODE();
	}

	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements(m_Device, image, &memRequirements);

	VkDeviceMemory imageMemory;
	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = texSize;
	allocInfo.memoryTypeIndex = memRequirements.memoryTypeBits;

	TransferContext transferCtx = GetTransferContext();

	transferCtx.Begin();
	transferCtx.End();

	if (vkAllocateMemory(m_Device, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS)
	{
		CKE_UNREACHABLE_CODE();
	}

	vkBindImageMemory(m_Device, image, imageMemory, 0);

	return image;
}

void VulkanDevice::MapMemory(VkDeviceMemory memory, VkDeviceSize offset, VkDeviceSize size, void** data)
{
	vkMapMemory(m_Device, memory, offset, size, 0, data);
}

void VulkanDevice::UnmapMemory(VkDeviceMemory memory)
{
	vkUnmapMemory(m_Device, memory);
}

void VulkanDevice::CreateGraphicsCommandPool()
{
	VkCommandPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	poolInfo.queueFamilyIndex = m_FamilyIndices.m_GraphicsFamily.value();

	if (vkCreateCommandPool(m_Device, &poolInfo, nullptr, &m_GraphicsCommandPool) != VK_SUCCESS)
	{
		std::cout << "Error creating command pool" << std::endl;
	}
}

void VulkanDevice::CreateTransferCommandPool()
{
	VkCommandPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	poolInfo.queueFamilyIndex = m_FamilyIndices.m_TransferFamily.value();

	if (vkCreateCommandPool(m_Device, &poolInfo, nullptr, &m_TransferCommandPool) != VK_SUCCESS)
	{
		std::cout << "Error creating command pool" << std::endl;
	}
}

u32 VulkanDevice::FindMemoryType(u32 typeFilter, VkMemoryPropertyFlags properties)
{
	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(m_PhysicalDevice, &memProperties);

	for (int i = 0; i < memProperties.memoryTypeCount; ++i)
	{
		if (typeFilter & (1 << i) &&
			(memProperties.memoryTypes[i].propertyFlags & properties) == properties)
		{
			return i;
		}
	}

	CKE_UNREACHABLE_CODE();
}

void VulkanDevice::CreateFrameSyncObjects()
{
	m_Frame.m_ImageAvailableSemaphores.resize(m_Frame.MAX_FRAMES_IN_FLIGHT);
	m_Frame.m_RenderFinishedSemaphores.resize(m_Frame.MAX_FRAMES_IN_FLIGHT);
	m_Frame.m_InFlightFences.resize(m_Frame.MAX_FRAMES_IN_FLIGHT);

	VkSemaphoreCreateInfo semaphoreInfo{};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fenceInfo{};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	for (int i = 0; i < m_Frame.MAX_FRAMES_IN_FLIGHT; ++i)
	{
		if (vkCreateSemaphore(m_Device, &semaphoreInfo, nullptr, &m_Frame.m_ImageAvailableSemaphores[i]) != VK_SUCCESS
			||
			vkCreateSemaphore(m_Device, &semaphoreInfo, nullptr, &m_Frame.m_RenderFinishedSemaphores[i]) != VK_SUCCESS
			||
			vkCreateFence(m_Device, &fenceInfo, nullptr, &m_Frame.m_InFlightFences[i]) != VK_SUCCESS)
		{
			std::cout << "Error creating sync primitives" << std::endl;
		}
	}
}

u32 VulkanDevice::AcquireNextImage()
{
	VkFence inFlightFence = m_Frame.GetInFlightFence();
	VkSemaphore imageAvailableSemaphore = m_Frame.GetImageAvailableSemaphore();

	vkWaitForFences(m_Device, 1, &inFlightFence, VK_TRUE, UINT64_MAX);

	u32 newImageIdx = 0;
	VkResult result = vkAcquireNextImageKHR(m_Device, m_SwapChain.m_SwapChain, UINT64_MAX,
		imageAvailableSemaphore, VK_NULL_HANDLE, &newImageIdx);

	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_SwapChain.m_FrameBufferResized)
	{
		m_SwapChain.m_FrameBufferResized = false;
		RecreateSwapChain(m_Device);
		return AcquireNextImage();
	}
	else if (result != VK_SUCCESS)
	{
		std::cout << "Error acquiring SwapChain image" << std::endl;
	}

	vkResetFences(m_Device, 1, &inFlightFence);

	m_SwapchainIdx = newImageIdx;
	return newImageIdx;
}

void VulkanDevice::Present()
{
	VkFence& inFlightFence = m_Frame.m_InFlightFences[m_Frame.m_Idx];
	VkCommandBuffer& commandBuffer = m_Frame.m_GraphicsCommandBuffers[m_Frame.m_Idx];
	VkSemaphore& imageAvailableSemaphore = m_Frame.m_ImageAvailableSemaphores[m_Frame.m_Idx];
	VkSemaphore& finishedSemaphore = m_Frame.m_RenderFinishedSemaphores[m_Frame.m_Idx];

	// Build & Submit Present Commands
	//-----------------------------------------------------------------------------

	VkSemaphore signalSemaphores[] = { m_Frame.m_RenderFinishedSemaphores[m_Frame.m_Idx] };

	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;

	VkSwapchainKHR swapChains[] = { m_SwapChain.m_SwapChain };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;
	presentInfo.pImageIndices = &m_SwapchainIdx;

	presentInfo.pResults = nullptr;

	vkQueuePresentKHR(m_PresentQueue, &presentInfo);

	m_Frame.m_Idx = (m_Frame.m_Idx + 1) % m_Frame.MAX_FRAMES_IN_FLIGHT;
}

void VulkanDevice::DestroySwapChain()
{
	for (VkFramebuffer_T* framebuffer : m_SwapChain.m_FrameBuffers)
	{
		vkDestroyFramebuffer(m_Device, framebuffer, nullptr);
	}

	for (VkImageView_T* swapChainImageView : m_SwapChain.m_ImageViews)
	{
		vkDestroyImageView(m_Device, swapChainImageView, nullptr);
	}

	vkDestroySwapchainKHR(m_Device, m_SwapChain.m_SwapChain, nullptr);
}



void VulkanDevice::DestroyFrameSyncObjects()
{
	for (int i = 0; i < m_Frame.MAX_FRAMES_IN_FLIGHT; ++i)
	{
		vkDestroySemaphore(m_Device, m_Frame.m_ImageAvailableSemaphores[i], nullptr);
		vkDestroySemaphore(m_Device, m_Frame.m_RenderFinishedSemaphores[i], nullptr);
		vkDestroyFence(m_Device, m_Frame.m_InFlightFences[i], nullptr);
	}
}

void VulkanDevice::TriggerFrameBufferResized()
{
	m_SwapChain.m_FrameBufferResized = true;
}

void VulkanDevice::DestroyRenderPass()
{
	vkDestroyRenderPass(m_Device, m_RenderPass, nullptr);
}

void VulkanDevice::DestroyAllCommandPools()
{
	vkDestroyCommandPool(m_Device, m_GraphicsCommandPool, nullptr);
	vkDestroyCommandPool(m_Device, m_TransferCommandPool, nullptr);
}

void VulkanDevice::CreateDescriptorSetLayout()
{
	VkDescriptorSetLayoutBinding defaultLayoutBinding{};
	defaultLayoutBinding.binding = 0;
	defaultLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	defaultLayoutBinding.descriptorCount = 1;
	defaultLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

	VkDescriptorSetLayoutCreateInfo layoutInfo{};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = 1;
	layoutInfo.pBindings = &defaultLayoutBinding;
	layoutInfo.flags = 0;

	if (vkCreateDescriptorSetLayout(m_Device, &layoutInfo, nullptr
		, &m_DescriptorSetLayout) != VK_SUCCESS) {
		CKE_UNREACHABLE_CODE();
	}
}

void VulkanDevice::DestroyDescriptorSetLayout()
{
	vkDestroyDescriptorSetLayout(m_Device, m_DescriptorSetLayout, nullptr);
}

void VulkanDevice::CreateDescriptorPool()
{
	VkDescriptorPoolSize poolSize{};
	poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSize.descriptorCount = static_cast<u32>(m_Frame.MAX_FRAMES_IN_FLIGHT);

	VkDescriptorPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = 1;
	poolInfo.pPoolSizes = &poolSize;
	poolInfo.maxSets = static_cast<u32>(m_Frame.MAX_FRAMES_IN_FLIGHT);
	poolInfo.flags = 0;

	if (vkCreateDescriptorPool(m_Device, &poolInfo, nullptr, &m_DescriptorPool) != VK_SUCCESS) {
		CKE_UNREACHABLE_CODE();
	}
}

void VulkanDevice::DestroyDescriptorPool()
{
	vkDestroyDescriptorPool(m_Device, m_DescriptorPool, nullptr);
}

void VulkanDevice::CreateDescriptorSets()
{
	Vector<VkDescriptorSetLayout> layouts(m_Frame.MAX_FRAMES_IN_FLIGHT, m_DescriptorSetLayout);
	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = m_DescriptorPool;
	allocInfo.descriptorSetCount = static_cast<u32>(m_Frame.MAX_FRAMES_IN_FLIGHT);
	allocInfo.pSetLayouts = layouts.data();

	if (vkAllocateDescriptorSets(m_Device, &allocInfo, m_Frame.m_DescriptorSets.data()) != VK_SUCCESS) { CKE_UNREACHABLE_CODE(); }
}

void VulkanDevice::CreateTransferCommandBuffers()
{
	m_Frame.m_TransferCommandBuffers.resize(m_Frame.MAX_FRAMES_IN_FLIGHT);

	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = m_TransferCommandPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = m_Frame.MAX_FRAMES_IN_FLIGHT;

	if (vkAllocateCommandBuffers(m_Device, &allocInfo, m_Frame.m_TransferCommandBuffers.data()) != VK_SUCCESS)
	{
		std::cout << "Error creating command buffer" << std::endl;
	}
}

GraphicsContext VulkanDevice::GetGraphicsContext()
{
	GraphicsContext ctx;
	ctx.m_SwapChain = &m_SwapChain;
	ctx.m_CommandBuffer = m_Frame.GetGraphicsCommandBuffer();
	return ctx;
}

TransferContext VulkanDevice::GetTransferContext()
{
	TransferContext ctx;
	ctx.m_CommandBuffer = m_Frame.GetTransferCommandBuffer();
	return ctx;
}

void VulkanDevice::SubmitGraphicsWork(GraphicsContext& context)
{
	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	VkSemaphore          waitSemaphores[] = { m_Frame.m_ImageAvailableSemaphores[m_Frame.m_Idx] };
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;

	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &context.m_CommandBuffer;

	VkSemaphore signalSemaphores[] = { m_Frame.m_RenderFinishedSemaphores[m_Frame.m_Idx] };
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	if (vkQueueSubmit(m_GraphicsQueue, 1, &submitInfo, m_Frame.m_InFlightFences[m_Frame.m_Idx]) != VK_SUCCESS)
	{
		std::cout << "Error submiting graphics queue command buffer" << std::endl;
	}
}

void VulkanDevice::SubmitTransferWork(TransferContext& context)
{
	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &context.m_CommandBuffer;

	vkQueueSubmit(m_TransferQueue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(m_TransferQueue);
}

void VulkanDevice::WaitIdle()
{
	vkDeviceWaitIdle(m_Device);
}
