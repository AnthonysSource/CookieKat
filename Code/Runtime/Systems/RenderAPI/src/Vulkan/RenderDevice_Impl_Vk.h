#pragma once

#include "CookieKat/Systems/RenderAPI/Vulkan/RenderResources_Vk.h"
#include "CookieKat/Systems/RenderAPI/Vulkan/RenderDevice_Vk.h"
#include "CookieKat/Systems/RenderAPI/Vulkan/Instance_Vk.h"
#include "CookieKat/Systems/RenderAPI/Internal/SwapChain.h"
#include "Vulkan/Conversions_Vk.h"

#include <algorithm>
#include <iostream>

#include "Buffer.h"

namespace CKE {
	void RenderDevice::Initialize(Int2 backBufferSize) {
		CKE_ASSERT(m_pRenderTargetData != nullptr);

		// Create vulkan instance
		m_VulkanInstance.Initialize(m_pRenderTargetData);

		// Initialize Debug Function Pointers
		pfnSetDebugUtilsObjectNameEXT = (PFN_vkSetDebugUtilsObjectNameEXT)vkGetInstanceProcAddr(
			m_VulkanInstance.m_Instance, "vkSetDebugUtilsObjectNameEXT");
		pfnCmdBeginDebugUtilsLabelEXT = (PFN_vkCmdBeginDebugUtilsLabelEXT)vkGetInstanceProcAddr(
			m_VulkanInstance.m_Instance, "vkCmdBeginDebugUtilsLabelEXT");
		pfnCmdEndDebugUtilsLabelEXT = (PFN_vkCmdEndDebugUtilsLabelEXT)vkGetInstanceProcAddr(
			m_VulkanInstance.m_Instance, "vkCmdEndDebugUtilsLabelEXT");

		m_VulkanInstance.CreateLogicalDevice(*this);
		CreateSwapChain(backBufferSize);

		// Initialize frame data like frame begin/end semaphores
		for (FrameData_Vk& frame : m_Frame) {
			frame.Initialize(*this);
		}

		// We create a fixed amount of available command lists
		// that can be queried every frame
		CreateDefaultCommandPools();
		CreateDefaultCommandLists();
	}

	void RenderDevice::Shutdown() {
		DestroySwapChainViews();

		for (FrameData_Vk& frame : m_Frame) {
			frame.Destroy(*this);
		}

		DestroyDefaultCommandPools();
		vkDestroyDevice(m_Device, nullptr);

		m_VulkanInstance.Shutdown();
	}

	void RenderDevice::CreateSwapChain(Int2 frameBufferSize) {
		SwapChainSupportDetails swapChainSupport = m_VulkanInstance.QuerySwapChainSupport(
			m_PhysicalDevice);

		VkSurfaceFormatKHR surfaceFormat = SwapchainInitialConfig::SelectSwapSurfaceFormat(
			swapChainSupport.m_Formats);
		VkPresentModeKHR presentMode = SwapchainInitialConfig::SelectSwapPresentMode(
			swapChainSupport.m_PresentModes);
		VkExtent2D extent = SwapchainInitialConfig::SelectSwapExtent(
			swapChainSupport.m_Capabilities, frameBufferSize);

		u32       imageCount = swapChainSupport.m_Capabilities.minImageCount + 1;
		u32 const maxCount = swapChainSupport.m_Capabilities.maxImageCount;
		if (maxCount > 0 && imageCount > maxCount) { imageCount = maxCount; }

		VkSwapchainCreateInfoKHR createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface = m_VulkanInstance.GetSurface();

		createInfo.minImageCount = imageCount;
		createInfo.imageFormat = surfaceFormat.format;
		createInfo.imageColorSpace = surfaceFormat.colorSpace;
		createInfo.imageExtent = extent;
		createInfo.imageArrayLayers = 1;
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;

		QueueFamilyIndices indices = m_VulkanInstance.FindQueueFamilies(m_PhysicalDevice);
		Array<u32, 2>      queueFamilyIndices = {indices.GetGraphicsIdx(), indices.GetPresentIdx()};

		if (indices.GetGraphicsIdx() != indices.GetPresentIdx()) {
			createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			createInfo.queueFamilyIndexCount = queueFamilyIndices.size();
			createInfo.pQueueFamilyIndices = queueFamilyIndices.data();
		}
		else {
			createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		}

		createInfo.preTransform = swapChainSupport.m_Capabilities.currentTransform;
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		createInfo.presentMode = presentMode;
		createInfo.clipped = true;
		createInfo.oldSwapchain = m_SwapChain.m_vkSwapChain;

		if (vkCreateSwapchainKHR(m_Device, &createInfo, nullptr, &m_SwapChain.m_vkSwapChain) !=
			VK_SUCCESS) {
			std::cout << "Error creating SwapChain" << std::endl;
		}

		vkGetSwapchainImagesKHR(m_Device, m_SwapChain.m_vkSwapChain, &imageCount, nullptr);
		Vector<VkImage> swapchainImages{imageCount};
		vkGetSwapchainImagesKHR(m_Device, m_SwapChain.m_vkSwapChain, &imageCount,
		                        swapchainImages.data());

		int imgIdx = 1;
		for (VkImage swapchainImage : swapchainImages) {
			Texture* texture = m_ResourcesDB.CreateTexture();
			texture->m_vkImage = swapchainImage;
			m_SwapChain.m_Textures.push_back(texture->m_DBHandle);

			TextureViewDesc viewDesc{};
			viewDesc.m_Format = TextureFormat::B8G8R8A8_SRGB;
			viewDesc.m_AspectMask = TextureAspectMask::Color;
			viewDesc.m_Type = TextureViewType::Tex2D;
			viewDesc.m_Texture = texture->m_DBHandle;
			TextureViewHandle viewHandle = CreateTextureView(viewDesc);
			m_SwapChain.m_Views.push_back(viewHandle);

			// Set debug name
			String name{"Swapchain Image"};
			imgIdx++;
			VkDebugUtilsObjectNameInfoEXT debugInfo{};
			debugInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
			debugInfo.objectHandle = (u64)texture->m_vkImage;
			debugInfo.objectType = VK_OBJECT_TYPE_IMAGE;
			debugInfo.pObjectName = name.c_str();
			pfnSetDebugUtilsObjectNameEXT(m_Device, &debugInfo);
		}

		m_SwapChain.m_vkImageFormat = surfaceFormat.format;
		m_SwapChain.m_vkExtent = extent;
		// TODO: HARDCODED
		TextureDesc swapChainDesc{};
		swapChainDesc.m_Format = TextureFormat::B8G8R8A8_SRGB;
		swapChainDesc.m_Size = UInt3{extent.width, extent.height, 1};
		swapChainDesc.m_AspectMask = TextureAspectMask::Color;
		swapChainDesc.m_TextureType = TextureType::Tex2D;
		swapChainDesc.m_Usage = TextureUsage::Color_Attachment;
		m_SwapChain.m_Desc = swapChainDesc;
	}

	void RenderDevice::RecreateSwapChain(Int2 newSize) {
		// This is a weird case when we minimize the window
		if (newSize == Int2(0, 0)) { return; }

		vkDeviceWaitIdle(m_Device);

		DestroySwapChainViews();
		CreateSwapChain(newSize);
	}

	void RenderDevice::PrepareDescriptorPoolsAndSetsForPipeline(Vector<ShaderBinding> const& shaderBindings, Pipeline& pipeline,
	                                                            PipelineLayout*              pLayout) {
		// Convert the user-defined bindings into a more practical format
		Vector<Vector<ShaderBinding>> sortedBindings{4};
		for (ShaderBinding const& binding : shaderBindings) {
			sortedBindings[binding.m_SetIndex].emplace_back(binding);
		}

		for (i32 i = 0; i < RenderSettings::MAX_FRAMES_IN_FLIGHT; ++i) {
			m_Frame[i].m_PipelineFrameData.insert({pipeline.m_DBHandle, {}});
		}

		CreatePipelineDescriptorPool(pipeline, sortedBindings);

		for (i32 i = 0; i < sortedBindings.size(); ++i) {
			AllocateDescriptorSet(pipeline.m_DBHandle, pLayout, i, 1);
		}
	}

	PipelineHandle RenderDevice::CreateGraphicsPipeline(GraphicsPipelineDesc const& desc) {
		Pipeline& pipeline = m_ResourcesDB.CreatePipeline();

		// Shader Bindings
		//-----------------------------------------------------------------------------

		if (desc.m_LayoutHandle != RenderHandle::Invalid()) {
			pipeline.m_PipelineLayout = desc.m_LayoutHandle;
			PipelineLayout* pLayout = m_ResourcesDB.GetPipelineLayout(pipeline.m_PipelineLayout);
			PrepareDescriptorPoolsAndSetsForPipeline(pLayout->m_Desc.m_ShaderBindings, pipeline, pLayout);
		}

		// Shader Stages
		//-----------------------------------------------------------------------------

		Vector<VkPipelineShaderStageCreateInfo> shaderStages;
		Vector<VkShaderModule>                  shaderModules;
		shaderStages.reserve(5);
		shaderModules.reserve(5);

		VkPipelineShaderStageCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		createInfo.pName = "main";
		if (!desc.m_VertexShaderSource.empty()) {
			VkShaderModule vkShader = CreateShaderModule(desc.m_VertexShaderSource);
			createInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
			createInfo.module = vkShader;
			shaderStages.push_back(createInfo);
			shaderModules.push_back(vkShader);
		}
		if (!desc.m_FragmentShaderSource.empty()) {
			VkShaderModule vkShader = CreateShaderModule(desc.m_FragmentShaderSource);
			createInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
			createInfo.module = vkShader;
			shaderStages.push_back(createInfo);
			shaderModules.push_back(vkShader);
		}
		if (!desc.m_TesselationControlShaderSource.empty()) {
			VkShaderModule vkShader = CreateShaderModule(desc.m_TesselationControlShaderSource);
			createInfo.stage = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
			createInfo.module = vkShader;
			shaderStages.push_back(createInfo);
			shaderModules.push_back(vkShader);
		}
		if (!desc.m_TesselationEvaluationShaderSource.empty()) {
			VkShaderModule vkShader = CreateShaderModule(desc.m_TesselationEvaluationShaderSource);
			createInfo.stage = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
			createInfo.module = vkShader;
			shaderStages.push_back(createInfo);
			shaderModules.push_back(vkShader);
		}
		if (!desc.m_GeometryShaderSource.empty()) {
			VkShaderModule vkShader = CreateShaderModule(desc.m_GeometryShaderSource);
			createInfo.stage = VK_SHADER_STAGE_GEOMETRY_BIT;
			createInfo.module = vkShader;
			shaderStages.push_back(createInfo);
			shaderModules.push_back(vkShader);
		}

		// Dynamic State
		//-----------------------------------------------------------------------------

		// By default we set only the viewport and scissor as dynamic,
		// we could just set the viewport as we don't use the scissor
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

		Vector<VertexInputInfo> const& vertInput = desc.m_VertexInput.m_VertexInput;

		VkVertexInputBindingDescription vertexInputBinding{};
		vertexInputBinding.binding = 0;
		vertexInputBinding.stride = desc.m_VertexInput.m_Stride;
		vertexInputBinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		Vector<VkVertexInputAttributeDescription> vkVertexInputAttrDesc{};
		u32                                       currOffset = 0;
		for (int i = 0; i < vertInput.size(); ++i) {
			VkVertexInputAttributeDescription vertDesc{};
			vertDesc.binding = vertInput[i].m_Binding;
			vertDesc.location = i;
			vertDesc.format = ConversionsVK::GetVkVertexFormat(vertInput[i].m_Type);
			vertDesc.offset = currOffset;
			currOffset += ConversionsVK::GetVkAttributeSize(vertInput[i].m_Type);
			vkVertexInputAttrDesc.emplace_back(vertDesc);
		}

		VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputInfo.vertexBindingDescriptionCount = vertInput.empty() ? 0 : 1;
		vertexInputInfo.pVertexBindingDescriptions = vertInput.empty() ? nullptr : &vertexInputBinding;
		vertexInputInfo.vertexAttributeDescriptionCount = static_cast<u32>(vkVertexInputAttrDesc.size());
		vertexInputInfo.pVertexAttributeDescriptions = vkVertexInputAttrDesc.data();

		// Input Assembly
		//-----------------------------------------------------------------------------

		VkPipelineInputAssemblyStateCreateInfo inputAssemblyCreateInfo{};
		inputAssemblyCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssemblyCreateInfo.topology = ConversionsVK::GetVkPrimitiveTopology(desc.m_Topology);
		inputAssemblyCreateInfo.primitiveRestartEnable = VK_FALSE;

		// Viewports & Scissors
		//-----------------------------------------------------------------------------

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
		rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		rasterizer.depthBiasEnable = VK_FALSE;

		// Multi-sampling
		//-----------------------------------------------------------------------------

		VkPipelineMultisampleStateCreateInfo multisampling{};
		multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampling.sampleShadingEnable = VK_FALSE;
		multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

		// Depth-Stencil
		//-----------------------------------------------------------------------------

		DepthStencilState const*              depthStencilState = &desc.m_DepthStencilState;
		VkPipelineDepthStencilStateCreateInfo depthStencil{};
		depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depthStencil.depthTestEnable = depthStencilState->m_DepthTestEnable;
		depthStencil.depthWriteEnable = depthStencilState->m_DepthWrite;
		depthStencil.depthCompareOp = static_cast<VkCompareOp>(depthStencilState->m_DepthCompareOp);
		depthStencil.depthBoundsTestEnable = VK_FALSE;
		depthStencil.minDepthBounds = 0.0f;
		depthStencil.maxDepthBounds = 1.0f;
		depthStencil.stencilTestEnable = depthStencilState->m_StencilEnable;
		depthStencil.front = {};
		depthStencil.back = {};

		// Color Blending
		//-----------------------------------------------------------------------------

		BlendState const*                           blendState = &desc.m_BlendState;
		Vector<VkPipelineColorBlendAttachmentState> vkBlendStates{};
		vkBlendStates.reserve(blendState->m_AttachmentBlendStates.size());
		for (AttachmentBlendState const& attBlend : blendState->m_AttachmentBlendStates) {
			VkPipelineColorBlendAttachmentState colorBlendAttachment{};
			colorBlendAttachment.colorWriteMask = ConversionsVK::GetVkColorComponentFlags(
				attBlend.m_ColorWriteMask);
			colorBlendAttachment.blendEnable = attBlend.m_BlendEnable;

			colorBlendAttachment.srcColorBlendFactor = ConversionsVK::GetVkBlendFactor(
				attBlend.m_SrcColorBlendFactor);
			colorBlendAttachment.dstColorBlendFactor = ConversionsVK::GetVkBlendFactor(
				attBlend.m_DstColorBlendFactor);
			colorBlendAttachment.colorBlendOp = ConversionsVK::GetVkBlendOp(attBlend.m_ColorBlendOp);

			colorBlendAttachment.srcAlphaBlendFactor = ConversionsVK::GetVkBlendFactor(
				attBlend.m_SrcAlphaBlendFactor);
			colorBlendAttachment.dstAlphaBlendFactor = ConversionsVK::GetVkBlendFactor(
				attBlend.m_DstAlphaBlendFactor);
			colorBlendAttachment.alphaBlendOp = ConversionsVK::GetVkBlendOp(attBlend.m_AlphaBlendOp);

			vkBlendStates.push_back(colorBlendAttachment);
		}

		VkPipelineColorBlendStateCreateInfo colorBlending{};
		colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlending.logicOpEnable = VK_FALSE;
		colorBlending.logicOp = VK_LOGIC_OP_COPY;
		colorBlending.attachmentCount = static_cast<u32>(vkBlendStates.size());
		colorBlending.pAttachments = vkBlendStates.empty() ? nullptr : vkBlendStates.data();
		colorBlending.blendConstants[0] = blendState->m_BlendConstants[0];
		colorBlending.blendConstants[1] = blendState->m_BlendConstants[1];
		colorBlending.blendConstants[2] = blendState->m_BlendConstants[2];
		colorBlending.blendConstants[3] = blendState->m_BlendConstants[3];

		// Pipeline
		//-----------------------------------------------------------------------------

		Vector<VkFormat> colorAttachFormats{};
		colorAttachFormats.reserve(desc.m_AttachmentsInfo.m_ColorAttachments.size());
		for (auto& colorAttachment : desc.m_AttachmentsInfo.m_ColorAttachments) {
			colorAttachFormats.push_back(ConversionsVK::GetVkImageFormat(colorAttachment));
		}

		VkPipelineRenderingCreateInfo renderingCreateInfo{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
			.colorAttachmentCount = u32(colorAttachFormats.size()),
			.pColorAttachmentFormats = colorAttachFormats.size() > 0
				                           ? colorAttachFormats.data()
				                           : nullptr,
			.depthAttachmentFormat = ConversionsVK::GetVkImageFormat(desc.m_AttachmentsInfo.m_DepthStencil),
			.stencilAttachmentFormat = ConversionsVK::GetVkImageFormat(desc.m_AttachmentsInfo.m_DepthStencil)
		};

		VkGraphicsPipelineCreateInfo pipelineCreateInfo{};
		pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineCreateInfo.pNext = &renderingCreateInfo;
		pipelineCreateInfo.stageCount = shaderStages.size();
		pipelineCreateInfo.pStages = shaderStages.data();
		pipelineCreateInfo.pVertexInputState = &vertexInputInfo;
		pipelineCreateInfo.pInputAssemblyState = &inputAssemblyCreateInfo;
		pipelineCreateInfo.pViewportState = &viewportCreateInfo;
		pipelineCreateInfo.pRasterizationState = &rasterizer;
		pipelineCreateInfo.pMultisampleState = &multisampling;
		pipelineCreateInfo.pDepthStencilState = &depthStencil;
		pipelineCreateInfo.pColorBlendState = &colorBlending;
		pipelineCreateInfo.pDynamicState = &dynamicStateCreateInfo;

		if (!pipeline.m_PipelineLayout.IsNull()) {
			pipelineCreateInfo.layout = m_ResourcesDB.GetPipelineLayout(pipeline.m_PipelineLayout)->m_vkPipelineLayout;
		}

		// We are using dynamic rendering so we don't need these
		pipelineCreateInfo.renderPass = nullptr;
		pipelineCreateInfo.subpass = 0;

		pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
		pipelineCreateInfo.basePipelineIndex = -1;

		if (vkCreateGraphicsPipelines(m_Device, VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr
		                              , &pipeline.m_vkPipeline) != VK_SUCCESS) {
			std::cout << "Error creating graphics pipeline" << std::endl;
		}

		// Cleanup
		//-----------------------------------------------------------------------------

		for (VkShaderModule module : shaderModules) {
			vkDestroyShaderModule(m_Device, module, nullptr);
		}

		return pipeline.m_DBHandle;
	}

	PipelineHandle RenderDevice::CreateComputePipeline(ComputePipelineDesc const& desc) {
		Pipeline& pipeline = m_ResourcesDB.CreatePipeline();
		pipeline.m_PipelineLayout = desc.m_Layout;
		PipelineLayout* pLayout = m_ResourcesDB.GetPipelineLayout(desc.m_Layout);

		PrepareDescriptorPoolsAndSetsForPipeline(pLayout->m_Desc.m_ShaderBindings, pipeline, pLayout);

		VkShaderModule                  vkShader = CreateShaderModule(desc.m_ComputeShaderSrc);
		VkPipelineShaderStageCreateInfo shaderStageCreateInfo{};
		shaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shaderStageCreateInfo.pName = "main";
		shaderStageCreateInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
		shaderStageCreateInfo.module = vkShader;

		VkComputePipelineCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
		createInfo.layout = pLayout->m_vkPipelineLayout;
		createInfo.basePipelineHandle = VK_NULL_HANDLE;
		createInfo.basePipelineIndex = -1;
		createInfo.stage = shaderStageCreateInfo;
		if (vkCreateComputePipelines(m_Device, VK_NULL_HANDLE, 1, &createInfo, nullptr, &pipeline.m_vkPipeline) != VK_SUCCESS) {
			std::cout << "Error creating compute pipeline" << std::endl;
		}

		vkDestroyShaderModule(m_Device, vkShader, nullptr);
		return pipeline.m_DBHandle;
	}

	void RenderDevice::DestroyPipeline(PipelineHandle pipelineHandle) {
		Pipeline& pPipeline = m_ResourcesDB.GetPipeline(pipelineHandle);
		vkDestroyPipeline(m_Device, pPipeline.m_vkPipeline, nullptr);
		m_ResourcesDB.RemovePipeline(pipelineHandle);
	}

	VkShaderModule RenderDevice::CreateShaderModule(Vector<u8> const& code) {
		VkShaderModuleCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = code.size();
		createInfo.pCode = reinterpret_cast<u32 const*>(code.data());

		VkShaderModule shaderModule{};
		if (vkCreateShaderModule(m_Device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
			std::cout << "Error creating shader module" << std::endl;
		}
		return shaderModule;
	}

	void RenderDevice::CreateCommandList(CommandListDesc desc) {
		// Generate the vk cmd buffers and store them in a temporary array
		FrameArray<VkCommandBuffer> tempBuffer{};
		for (i32 i = 0; i < RenderSettings::MAX_FRAMES_IN_FLIGHT; ++i) {
			VkCommandBufferAllocateInfo allocInfo{};
			allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
			allocInfo.commandBufferCount = 1;
			if (desc.m_Type == CommandListType::Graphics)
				allocInfo.commandPool = m_Frame[i].m_GraphicsCommandPool;
			else if (desc.m_Type == CommandListType::Transfer)
				allocInfo.commandPool = m_Frame[i].m_TransferCommandPool;
			else if (desc.m_Type == CommandListType::Compute) {
				allocInfo.commandPool = m_Frame[i].m_ComputeCommandPool;
			}

			if (vkAllocateCommandBuffers(m_Device, &allocInfo, &tempBuffer[i]) != VK_SUCCESS) {
				std::cout << "Error creating command buffer" << std::endl;
			}
		}

		// Copy them to their final place
		for (u32 i = 0; i < RenderSettings::MAX_FRAMES_IN_FLIGHT; ++i) {
			if (desc.m_Type == CommandListType::Graphics)
				m_Frame[i].m_GraphicsCommandBuffer.push_back(tempBuffer[i]);
			else if (desc.m_Type == CommandListType::Transfer)
				m_Frame[i].m_TransferCommandBuffer.push_back(tempBuffer[i]);
			else if (desc.m_Type == CommandListType::Compute)
				m_Frame[i].m_ComputeCommandBuffer.push_back(tempBuffer[i]);
		}
	}

	void RenderDevice::CreateBufferInternal(BufferDesc bufferDesc, Buffer* pBuffer) {
		// Create Buffer

		Array<u32, 3> familyIndices = {
			m_QueueFamilyIndices.GetGraphicsIdx(),
			m_QueueFamilyIndices.GetTransferIdx(),
			m_QueueFamilyIndices.GetComputeIdx()
		};

		VkBufferCreateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = bufferDesc.m_SizeInBytes;
		bufferInfo.usage = ConversionsVK::GetVkBufferUsageFlags(bufferDesc.m_Usage);
		bufferInfo.sharingMode = bufferDesc.m_ConcurrentSharingMode ? VK_SHARING_MODE_CONCURRENT : VK_SHARING_MODE_EXCLUSIVE;
		bufferInfo.flags = 0;
		bufferInfo.queueFamilyIndexCount = familyIndices.size();
		bufferInfo.pQueueFamilyIndices = familyIndices.data();

		if (vkCreateBuffer(m_Device, &bufferInfo, nullptr, &pBuffer->m_vkBuffer) != VK_SUCCESS) {
			CKE_UNREACHABLE_CODE();
		}

		// Allocate Memory

		VkMemoryRequirements memoryRequirements;
		vkGetBufferMemoryRequirements(m_Device, pBuffer->m_vkBuffer, &memoryRequirements);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memoryRequirements.size;
		allocInfo.memoryTypeIndex = FindMemoryType(memoryRequirements.memoryTypeBits, ConversionsVK::GetVkMemoryProperties(bufferDesc.m_MemoryAccess));
		if (vkAllocateMemory(m_Device, &allocInfo, nullptr, &pBuffer->m_vkMemory) != VK_SUCCESS) {
			CKE_UNREACHABLE_CODE();
		}

		// Bind buffer to memory

		if (vkBindBufferMemory(m_Device, pBuffer->m_vkBuffer, pBuffer->m_vkMemory, 0) != VK_SUCCESS) {
			CKE_UNREACHABLE_CODE();
		}

		// Set buffer debug name

		if (bufferDesc.m_Name.GetStr()) {
			VkDebugUtilsObjectNameInfoEXT debugInfo{};
			debugInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
			debugInfo.objectHandle = (u64)pBuffer->m_vkBuffer;
			debugInfo.objectType = VK_OBJECT_TYPE_BUFFER;
			debugInfo.pObjectName = bufferDesc.m_Name.GetStr();
			pfnSetDebugUtilsObjectNameEXT(m_Device, &debugInfo);
		}
	}

	void RenderDevice::CopyDataToBuffer(BufferDesc bufferDesc, void* pInitialData, u32 dataSizeInBytes, Buffer& buffer) {
		// If the memory is in GPU-only memory space, it should have the transfer dst bit.
		// We copy the supplied initial data by using a staging buffer
		if (bufferDesc.m_MemoryAccess == MemoryAccess::GPU) {
			BufferDesc stagingBufferDesc{};
			stagingBufferDesc.m_Usage = BufferUsage::TransferSrc;
			stagingBufferDesc.m_MemoryAccess = MemoryAccess::CPU_GPU;
			stagingBufferDesc.m_SizeInBytes = dataSizeInBytes;
			BufferHandle stagingBufferHandle = CreateBuffer_DEPR(stagingBufferDesc, pInitialData, dataSizeInBytes);

			TransferCommandList transferCmdList = GetTransferCmdList();
			transferCmdList.Begin();
			transferCmdList.CopyBuffer(stagingBufferHandle, buffer.m_DBHandle, stagingBufferDesc.m_SizeInBytes);
			transferCmdList.End();
			SubmitTransferCommandList(transferCmdList, {});
			WaitTransferQueueIdle();

			DestroyBuffer(stagingBufferHandle);
		}
		else {
			memcpy(buffer.m_pMapped, pInitialData, dataSizeInBytes);
		}
	}

	BufferHandle RenderDevice::CreateBuffer_DEPR(BufferDesc bufferDesc, void* pInitialData, u64 dataByteSize) {
		BufferHandle handle{};

		if (bufferDesc.m_UpdateFrequency == UpdateFrequency::Static) {
			// If the update frequency is static then we only need
			// to create one global buffer 
			Buffer* pBuffer = m_ResourcesDB.CreateBuffer();
			handle = pBuffer->m_DBHandle;
			pBuffer->m_Size = bufferDesc.m_SizeInBytes;
			pBuffer->m_Offset = 0;
			pBuffer->m_IsPerFrame = false;

			CreateBufferInternal(bufferDesc, pBuffer);

			if (bufferDesc.m_MemoryAccess == MemoryAccess::CPU_GPU) {
				// We always permanently map per frame buffers
				vkMapMemory(m_Device, pBuffer->m_vkMemory, pBuffer->m_Offset, pBuffer->m_Size, 0,
				            &pBuffer->m_pMapped);
			}

			if (pInitialData != nullptr) {
				CopyDataToBuffer(bufferDesc, pInitialData, dataByteSize, *pBuffer);
			}
		}
		else if (bufferDesc.m_UpdateFrequency == UpdateFrequency::PerFrame) {
			// If the buffer is per frame then we have to create one buffer
			// for each possible frame in flight because the buffers
			// might need to have different data per frame.
			FrameArray<Buffer*> buffers = m_ResourcesDB.CreateBuffersPerFrame();

			// Each of these buffers is identified by the same BufferHandle.
			// When retrieving the underlying buffer, the Device handles this
			// and returns the correct buffer for the current frame.
			handle = buffers[0]->m_DBHandle;

			for (Buffer* pBuffer : buffers) {
				pBuffer->m_Size = bufferDesc.m_SizeInBytes;
				pBuffer->m_Offset = 0;
				pBuffer->m_IsPerFrame = true;

				CreateBufferInternal(bufferDesc, pBuffer);

				// We always permanently map per frame buffers
				vkMapMemory(m_Device, pBuffer->m_vkMemory, pBuffer->m_Offset, pBuffer->m_Size, 0,
				            &pBuffer->m_pMapped);

				// We have to copy the initial data to all of the buffers
				// just in case.
				if (pInitialData != nullptr) {
					CopyDataToBuffer(bufferDesc, pInitialData, dataByteSize, *pBuffer);
				}
			}
		}

		return handle;
	}

	void RenderDevice::DestroyBuffer(BufferHandle bufferHandle) {
		bool                isPerFrame = false;
		FrameArray<Buffer*> buffers = m_ResourcesDB.GetBuffer(bufferHandle, isPerFrame);

		if (isPerFrame) {
			for (Buffer* b : buffers) {
				vkDestroyBuffer(m_Device, b->m_vkBuffer, nullptr);
				vkFreeMemory(m_Device, b->m_vkMemory, nullptr);
			}
			m_ResourcesDB.RemoveBuffer(bufferHandle);
		}
		else {
			vkDestroyBuffer(m_Device, buffers[0]->m_vkBuffer, nullptr);
			vkFreeMemory(m_Device, buffers[0]->m_vkMemory, nullptr);
			m_ResourcesDB.RemoveBuffer(bufferHandle);
		}
	}

	TextureViewHandle RenderDevice::CreateTextureView(TextureViewDesc desc) {
		TextureView* pTexView = m_ResourcesDB.CreateTextureView();
		Texture*     pTex = m_ResourcesDB.GetTexture(desc.m_Texture);

		VkImageViewCreateInfo viewInfo{};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image = pTex->m_vkImage;
		viewInfo.viewType = ConversionsVK::GetVkImageViewType(desc.m_Type);
		viewInfo.format = ConversionsVK::GetVkImageFormat(desc.m_Format);
		viewInfo.subresourceRange.aspectMask = ConversionsVK::GetVkImageAspectFlags(desc.m_AspectMask);
		viewInfo.subresourceRange.baseMipLevel = desc.m_BaseMipLevel;
		viewInfo.subresourceRange.levelCount = desc.m_MipLevelCount;
		viewInfo.subresourceRange.baseArrayLayer = desc.m_BaseArrayLayer;
		viewInfo.subresourceRange.layerCount = desc.m_ArrayLayerCount;

		if (vkCreateImageView(m_Device, &viewInfo, nullptr, &pTexView->m_vkView) != VK_SUCCESS) {
			CKE_UNREACHABLE_CODE();
		}

		pTex->m_ExistingViews.push_back(pTexView->m_DBHandle);
		pTexView->m_Texture = desc.m_Texture;
		return pTexView->m_DBHandle;
	}

	void RenderDevice::DestroyTexture(TextureHandle textureHandle) {
		Texture* pTex = m_ResourcesDB.GetTexture(textureHandle);

		// Destroy all of the texture views if there are any left
		for (TextureViewHandle viewHandle : pTex->m_ExistingViews) {
			TextureView* textureView = m_ResourcesDB.GetTextureView(viewHandle);
			vkDestroyImageView(m_Device, textureView->m_vkView, nullptr);
			m_ResourcesDB.RemoveTextureView(viewHandle);
		}

		vkDestroyImage(m_Device, pTex->m_vkImage, nullptr);
		vkFreeMemory(m_Device, pTex->m_vkDeviceMemory, nullptr);
		m_ResourcesDB.RemoveTexture(textureHandle);
	}

	void RenderDevice::DestroyTextureView(TextureViewHandle handle) {
		TextureView* pView = m_ResourcesDB.GetTextureView(handle);
		Texture*     pTex = m_ResourcesDB.GetTexture(pView->m_Texture);

		vkDestroyImageView(m_Device, pView->m_vkView, nullptr);

		// Remove handle from texture reference
		for (int i = pTex->m_ExistingViews.size() - 1; i >= 0; --i) {
			if (pTex->m_ExistingViews[i] == handle) {
				pTex->m_ExistingViews[i] = pTex->m_ExistingViews.back();
				pTex->m_ExistingViews.pop_back();
			}
		}

		m_ResourcesDB.RemoveTextureView(handle);
	}

	void RenderDevice::CreateDefaultCommandPools() {
		VkCommandPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

		for (u32 i = 0; i < RenderSettings::MAX_FRAMES_IN_FLIGHT; ++i) {
			poolInfo.queueFamilyIndex = m_QueueFamilyIndices.GetGraphicsIdx();
			if (vkCreateCommandPool(m_Device, &poolInfo, nullptr, &m_Frame[i].m_GraphicsCommandPool)
				!= VK_SUCCESS) {
				std::cout << "Error creating command pool" << std::endl;
			}

			poolInfo.queueFamilyIndex = m_QueueFamilyIndices.GetTransferIdx();
			if (vkCreateCommandPool(m_Device, &poolInfo, nullptr, &m_Frame[i].m_TransferCommandPool)
				!= VK_SUCCESS) {
				std::cout << "Error creating command pool" << std::endl;
			}

			poolInfo.queueFamilyIndex = m_QueueFamilyIndices.GetComputeIdx();
			if (vkCreateCommandPool(m_Device, &poolInfo, nullptr, &m_Frame[i].m_ComputeCommandPool)
				!= VK_SUCCESS) {
				std::cout << "Error creating command pool" << std::endl;
			}
		}
	}

	u32 RenderDevice::FindMemoryType(u32 typeFilter, VkMemoryPropertyFlags properties) {
		VkPhysicalDeviceMemoryProperties memProperties;
		vkGetPhysicalDeviceMemoryProperties(m_PhysicalDevice, &memProperties);

		for (u32 i = 0; i < memProperties.memoryTypeCount; ++i) {
			if (typeFilter & (1 << i) &&
				(memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
				return i;
			}
		}

		CKE_UNREACHABLE_CODE();
		return 0;
	}

	void RenderDevice::AcquireNextBackBuffer() {
		VkFence     inFlightFence = m_ResourcesDB.GetFence(GetInFlightFence())->m_vkFence;
		VkSemaphore imageAvailableSemaphore = m_ResourcesDB.GetSemaphore(
			GetImageAvailableSemaphore()).m_vkSemaphore;

		vkWaitForFences(m_Device, 1, &inFlightFence, VK_TRUE, UINT64_MAX);
		VkResult result = vkAcquireNextImageKHR(m_Device, m_SwapChain.m_vkSwapChain, UINT64_MAX,
		                                        imageAvailableSemaphore, VK_NULL_HANDLE,
		                                        &m_SwapChain.m_CurrentTextureIdx);

		if (result != VK_SUCCESS) {
			std::cout << "Error acquiring SwapChain image" << std::endl;
		}

		vkResetFences(m_Device, 1, &inFlightFence);

		ResetAllPerFrameData();
	}

	void RenderDevice::ResetAllPerFrameData() {
		FrameData_Vk& newFrameData = GetCurrentFrameData();
		for (auto& [handle, pipeline] : m_ResourcesDB.GetAllPipelines()) {
			PipelineFrameData_Vk& pipelineFrameData = newFrameData.GetPipelineState(handle);
			vkResetDescriptorPool(m_Device, pipelineFrameData.m_DescriptorPool, 0);
		}
		newFrameData.ResetForNewFrame();
		m_ResourcesDB.DestroyAllDescriptorSets(m_CurrFrameInFlightIdx);
	}

	void RenderDevice::Present() {
		VkSemaphore vkRenderFinishedSemaphore = m_ResourcesDB.GetSemaphore(
			GetRenderFinishedSemaphore()).m_vkSemaphore;

		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = &vkRenderFinishedSemaphore;
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = &m_SwapChain.m_vkSwapChain;
		presentInfo.pImageIndices = &m_SwapChain.m_CurrentTextureIdx;
		presentInfo.pResults = nullptr;

		VkResult const result = vkQueuePresentKHR(m_PresentQueue, &presentInfo);

		// Resize SwapChain backbuffer if needed
		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_SwapChain.
			m_FrameBufferResized) {
			vkQueueWaitIdle(m_PresentQueue);
			m_SwapChain.m_FrameBufferResized = false;
			RecreateSwapChain(m_SwapChain.m_NewFramebufferSize);
		}

		// Advance frame idx counter
		m_CurrFrameInFlightIdx = (m_CurrFrameInFlightIdx + 1) % RenderSettings::MAX_FRAMES_IN_FLIGHT;
	}

	void RenderDevice::DestroySwapChainViews() {
		for (int i = 0; i < m_SwapChain.m_Textures.size(); ++i) {
			TextureHandle texHandle = m_SwapChain.m_Textures[i];
			TextureView*  pTexView = m_ResourcesDB.GetTextureView(m_SwapChain.m_Views[i]);
			vkDestroyImageView(m_Device, pTexView->m_vkView, nullptr);
			m_ResourcesDB.RemoveTexture(texHandle);
		}
		m_SwapChain.m_Textures.clear();
		m_SwapChain.m_Views.clear();

		// Textures get automatically deleted by this I believe
		// vkDestroySwapchainKHR(m_Device, m_SwapChain.m_vkSwapChain, nullptr);
	}

	TextureHandle RenderDevice::GetBackBuffer() {
		return m_SwapChain.m_Textures[m_SwapChain.m_CurrentTextureIdx];
	}

	TextureViewHandle RenderDevice::GetBackBufferView() {
		return m_SwapChain.m_Views[m_SwapChain.m_CurrentTextureIdx];
	}

	TextureDesc RenderDevice::GetBackBufferDesc() {
		return m_SwapChain.m_Desc;
	}

	UInt2 RenderDevice::GetBackBufferSize() const {
		return {m_SwapChain.m_vkExtent.width, m_SwapChain.m_vkExtent.height};
	}

	UInt3 RenderDevice::GetBackBufferSize3() const {
		return {m_SwapChain.m_vkExtent.width, m_SwapChain.m_vkExtent.height, 1};
	}

	void RenderDevice::RecordBackBufferResized(Int2 newSize) {
		m_SwapChain.m_FrameBufferResized = true;
		m_SwapChain.m_NewFramebufferSize = newSize;
	}

	void RenderDevice::PassRenderTargetData(void* pData) {
		m_pRenderTargetData = pData;
	}

	DescriptorSetBuilder
	RenderDevice::CreateDescriptorSetBuilder(PipelineHandle p, u64 setIndex) {
		DescriptorSetBuilder b{};
		b.m_PipelineHandle = p;
		b.m_SetIndex = setIndex;
		b.m_pDevice = this;
		return b;
	}

	void RenderDevice::UploadBufferData_DEPR(BufferHandle bufferHandle, void* pData, u64 dataByteSize,
	                                         u32          offsetInBuffer) {
		Buffer* buffer = m_ResourcesDB.GetBuffer(bufferHandle);
		CKE_ASSERT(buffer->m_Size >= dataByteSize);
		// The buffer must be a CPU mapped buffer to be able to upload data to it this way
		CKE_ASSERT(buffer->m_pMapped != nullptr);
		CKE_ASSERT(offsetInBuffer + dataByteSize <= buffer->m_Size);
		memcpy((u8*)buffer->m_pMapped + offsetInBuffer, pData, dataByteSize);
	}

	void* RenderDevice::GetBufferMappedPtr_DEPR(BufferHandle bufferHandle) {
		void* ptr = m_ResourcesDB.GetBuffer(bufferHandle)->m_pMapped;
		CKE_ASSERT(ptr != nullptr);
		return ptr;
	}

	TextureHandle RenderDevice::CreateTexture(TextureDesc desc) {
		Texture* pTex = m_ResourcesDB.CreateTexture();

		Array<u32, 3> familyIndices = {
			m_QueueFamilyIndices.GetGraphicsIdx(),
			m_QueueFamilyIndices.GetTransferIdx(),
			m_QueueFamilyIndices.GetComputeIdx()
		};

		VkImageCreateInfo imageInfo{};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = ConversionsVK::GetVkImageType(desc.m_TextureType);
		imageInfo.extent.width = desc.m_Size.x;
		imageInfo.extent.height = desc.m_Size.y;
		imageInfo.extent.depth = desc.m_Size.z;
		imageInfo.mipLevels = desc.m_MipLevels;
		imageInfo.arrayLayers = desc.m_ArraySize;
		imageInfo.format = ConversionsVK::GetVkImageFormat(desc.m_Format);
		imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageInfo.usage = ConversionsVK::GetVkImageUsageFlags(desc.m_Usage);
		imageInfo.sharingMode = desc.m_ConcurrentQueueUsage ? VK_SHARING_MODE_CONCURRENT : VK_SHARING_MODE_EXCLUSIVE;
		imageInfo.queueFamilyIndexCount = familyIndices.size();
		imageInfo.pQueueFamilyIndices = familyIndices.data();
		imageInfo.samples = ConversionsVK::GetVkSampleCountFlags(desc.m_SampleCount);
		imageInfo.flags = ConversionsVK::GetVkImageCreateFlags(desc.m_MiscFlags);

		if (vkCreateImage(m_Device, &imageInfo, nullptr, &pTex->m_vkImage) != VK_SUCCESS) {
			CKE_UNREACHABLE_CODE();
		}

		//-----------------------------------------------------------------------------

		VkMemoryRequirements memRequirements;
		vkGetImageMemoryRequirements(m_Device, pTex->m_vkImage, &memRequirements);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits,
		                                           VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		if (vkAllocateMemory(m_Device, &allocInfo, nullptr, &pTex->m_vkDeviceMemory) !=
			VK_SUCCESS) {
			CKE_UNREACHABLE_CODE();
		}

		vkBindImageMemory(m_Device, pTex->m_vkImage, pTex->m_vkDeviceMemory, 0);

		//-----------------------------------------------------------------------------

		if (desc.m_Name.GetStr()) {
			VkDebugUtilsObjectNameInfoEXT debugInfo{};
			debugInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
			debugInfo.objectHandle = reinterpret_cast<u64>(pTex->m_vkImage);
			debugInfo.objectType = VK_OBJECT_TYPE_IMAGE;
			debugInfo.pObjectName = desc.m_Name.GetStr();
			pfnSetDebugUtilsObjectNameEXT(m_Device, &debugInfo);
		}

		return pTex->m_DBHandle;
	}

	FrameData_Vk& RenderDevice::GetCurrentFrameData() { return m_Frame[GetFrameIdx()]; }

	u32 RenderDevice::GetFrameIdx() {
		return m_CurrFrameInFlightIdx;
	}

	SemaphoreHandle RenderDevice::GetImageAvailableSemaphore() {
		return GetCurrentFrameData().GetImageAvailableSemaphore();
	}

	SemaphoreHandle RenderDevice::GetRenderFinishedSemaphore() {
		return GetCurrentFrameData().GetRenderFinishedSemaphore();
	}

	FenceHandle RenderDevice::GetInFlightFence() {
		return GetCurrentFrameData().GetInFlightFence();
	}

	SamplerHandle RenderDevice::CreateSampler(SamplerDesc desc) {
		TextureSampler* pSampler = m_ResourcesDB.CreateTextureSampler();

		VkSamplerCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		createInfo.magFilter = ConversionsVK::GetVkFilter(desc.m_MagFilter);
		createInfo.minFilter = ConversionsVK::GetVkFilter(desc.m_MinFilter);

		createInfo.addressModeU = ConversionsVK::GetVkSamplerAdressMode(desc.m_WrapU);
		createInfo.addressModeV = ConversionsVK::GetVkSamplerAdressMode(desc.m_WrapV);
		createInfo.addressModeW = ConversionsVK::GetVkSamplerAdressMode(desc.m_WrapW);

		createInfo.anisotropyEnable = desc.m_AnisotropyEnable;
		// Check that the provided anisotropy is supported
		VkPhysicalDeviceProperties prop{};
		vkGetPhysicalDeviceProperties(m_PhysicalDevice, &prop);
		createInfo.maxAnisotropy = std::min(prop.limits.maxSamplerAnisotropy, desc.m_MaxAnisotropy);

		createInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		createInfo.unnormalizedCoordinates = VK_FALSE;

		createInfo.compareEnable = VK_FALSE;
		createInfo.compareOp = VK_COMPARE_OP_ALWAYS;

		createInfo.mipmapMode = ConversionsVK::GetVkMipMapMode(desc.m_MipMapMode);
		createInfo.mipLodBias = desc.m_LodBias;
		createInfo.minLod = desc.m_MinLod;
		createInfo.maxLod = desc.m_MaxLod;

		vkCreateSampler(m_Device, &createInfo, nullptr, &pSampler->m_vkSampler);
		return pSampler->m_DBHandle;
	}

	void RenderDevice::DestroySampler(SamplerHandle samplerHandle) {
		TextureSampler* pSampler = m_ResourcesDB.GetTextureSampler(samplerHandle);
		vkDestroySampler(m_Device, pSampler->m_vkSampler, nullptr);
		m_ResourcesDB.RemoveTextureSampler(samplerHandle);
	}

	PipelineLayoutHandle RenderDevice::CreatePipelineLayout(PipelineLayoutDesc const& layoutDesc) {
		PipelineLayout* pPipelineLayout = m_ResourcesDB.CreatePipelineLayout();

		pPipelineLayout->m_Desc = layoutDesc;

		Vector<Vector<ShaderBinding>> sortedBindings{4};
		for (ShaderBinding const& binding : layoutDesc.m_ShaderBindings) {
			sortedBindings[binding.m_SetIndex].emplace_back(binding);
		}

		CKE_ASSERT(sortedBindings.size() > 0 && sortedBindings.size() <= 4);
		Array<VkDescriptorSetLayout, 4> vkDescriptorSetLayouts{};

		for (int i = 0; i < sortedBindings.size(); ++i) {
			Vector<VkDescriptorSetLayoutBinding> vkBindings{};
			for (ShaderBinding const& bindingDesc : sortedBindings[i]) {
				VkDescriptorSetLayoutBinding b{};
				b.binding = bindingDesc.m_BindingPoint;
				b.descriptorCount = bindingDesc.m_Count;
				b.descriptorType = ConversionsVK::GetVkDescriptorType(bindingDesc.m_Type);
				b.stageFlags = ConversionsVK::GetVkShaderStageFlags(bindingDesc.m_StageMask);
				vkBindings.emplace_back(b);
			}

			VkDescriptorSetLayoutCreateInfo layoutInfo{};
			layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
			layoutInfo.bindingCount = static_cast<u32>(vkBindings.size());
			layoutInfo.pBindings = vkBindings.data();
			layoutInfo.flags = 0;

			if (vkCreateDescriptorSetLayout(m_Device, &layoutInfo, nullptr,
			                                &vkDescriptorSetLayouts[i]) != VK_SUCCESS) {
				CKE_UNREACHABLE_CODE();
			}
		}

		pPipelineLayout->m_DescriptorSetsInUse = sortedBindings.size();
		pPipelineLayout->m_DescriptorSetLayouts = vkDescriptorSetLayouts;

		VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
		pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutCreateInfo.setLayoutCount = pPipelineLayout->m_DescriptorSetsInUse;
		pipelineLayoutCreateInfo.pSetLayouts = pPipelineLayout->m_DescriptorSetLayouts.data();
		pipelineLayoutCreateInfo.pushConstantRangeCount = 0;
		pipelineLayoutCreateInfo.pPushConstantRanges = nullptr; //&pushConstantRange;

		if (vkCreatePipelineLayout(m_Device, &pipelineLayoutCreateInfo, nullptr, &pPipelineLayout->m_vkPipelineLayout)
			!= VK_SUCCESS) {
			std::cout << "Error creating pipeline layout" << std::endl;
		}

		return pPipelineLayout->m_DBHandle;
	}

	void RenderDevice::DestroyPipelineLayout(PipelineLayoutHandle handle) {
		PipelineLayout* pLayout = m_ResourcesDB.GetPipelineLayout(handle);
		vkDestroyPipelineLayout(m_Device, pLayout->m_vkPipelineLayout, nullptr);
	}

	void RenderDeviceDebugUtils::Initialize(RenderDevice* pDevice) {
		CKE_ASSERT(m_pDevice != nullptr);
		m_pDevice = pDevice;
	}

	void RenderDeviceDebugUtils::PrintAllResourcesState() {
		auto* db = &m_pDevice->m_ResourcesDB;

		std::cout << "FrameIdx: " << m_pDevice->GetFrameIdx() << "\n";

		for (std::pair<const RenderHandle, ResourceInfo> info : db->m_ResourceInfo) {
			std::cout << "Handle: " << info.first.m_Value << ", ";
			std::cout << "PerFrame: " << info.second.m_PerFrame << "\n";
		}
	}

	SemaphoreHandle RenderDevice::CreateSemaphoreGPU() {
		VkSemaphoreCreateInfo createInfo{
			.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
		};

		Semaphore& semaphore = m_ResourcesDB.AddSemaphore();
		if (vkCreateSemaphore(m_Device, &createInfo, nullptr, &semaphore.m_vkSemaphore) !=
			VK_SUCCESS) {
			CKE_UNREACHABLE_CODE();
		}

		return semaphore.m_DBHandle;
	}

	void RenderDevice::DestroySemaphore(SemaphoreHandle handle) {
		vkDestroySemaphore(m_Device, m_ResourcesDB.GetSemaphore(handle).m_vkSemaphore, nullptr);
	}

	FenceHandle RenderDevice::CreateFence(bool createSignaled) {
		VkFenceCreateInfo createInfo{
			.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO
		};

		if (createSignaled) {
			createInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
		}

		Fence* fence = m_ResourcesDB.CreateFence();
		if (vkCreateFence(m_Device, &createInfo, nullptr, &fence->m_vkFence) != VK_SUCCESS) {
			CKE_UNREACHABLE_CODE();
		}

		return fence->m_DBHandle;
	}

	void RenderDevice::DestroyFence(FenceHandle fence) {
		vkDestroyFence(m_Device, m_ResourcesDB.GetFence(fence)->m_vkFence, nullptr);
	}

	void RenderDevice::WaitForFence(FenceHandle fence) {
		vkWaitForFences(m_Device, 1, &m_ResourcesDB.GetFence(fence)->m_vkFence, true, UINT64_MAX);
	}

	void RenderDevice::ResetFence(FenceHandle fence) {
		vkResetFences(m_Device, 1, &m_ResourcesDB.GetFence(fence)->m_vkFence);
	}

	GraphicsCommandList RenderDevice::GetGraphicsCmdList() {
		FrameData_Vk&      f = GetCurrentFrameData();
		VkCommandBuffer cmdBuff = f.GetNextGraphicsCmdBuffer();
		return GraphicsCommandList{this, cmdBuff};
	}

	TransferCommandList RenderDevice::GetTransferCmdList() {
		FrameData_Vk&      f = GetCurrentFrameData();
		VkCommandBuffer cmdBuff = f.GetNextTransferCmdBuffer();
		return TransferCommandList{this, cmdBuff};
	}

	ComputeCommandList RenderDevice::GetComputeCmdList() {
		FrameData_Vk&      f = GetCurrentFrameData();
		VkCommandBuffer cmdBuff = f.GetNextComputeCmdBuffer();
		return ComputeCommandList{this, cmdBuff};
	}

	void RenderDevice::SubmitGraphicsCommandList(GraphicsCommandList& cmdList, CmdListSubmitInfo submitInfo) {
		Vector<GraphicsCommandList> v = {cmdList};
		SubmitGraphicsCommandLists(v, submitInfo);
	}

	void RenderDevice::SubmitTransferCommandList(TransferCommandList& cmdList, CmdListSubmitInfo submitInfo) {
		Vector<TransferCommandList> v = {cmdList};
		SubmitTransferCommandLists(v, submitInfo);
	}

	void RenderDevice::SubmitComputeCommandList(ComputeCommandList& cmdList, CmdListSubmitInfo submitInfo) {
		Vector<ComputeCommandList> v = {cmdList};
		SubmitComputeCommandLists(v, submitInfo);
	}

	void RenderDevice::DestroyDefaultCommandPools() {
		for (u32 i = 0; i < RenderSettings::MAX_FRAMES_IN_FLIGHT; ++i) {
			vkDestroyCommandPool(m_Device, m_Frame[i].m_GraphicsCommandPool, nullptr);
			vkDestroyCommandPool(m_Device, m_Frame[i].m_TransferCommandPool, nullptr);
			vkDestroyCommandPool(m_Device, m_Frame[i].m_ComputeCommandPool, nullptr);
		}
	}

	void RenderDevice::CreateDefaultCommandLists() {
		for (int i = 0; i < RenderSettings::GRAPHICS_CMDLIST_COUNT_PERFRAME; ++i) {
			CreateCommandList({CommandListType::Graphics, true});
		}
		for (int i = 0; i < RenderSettings::TRANSFER_CMDLIST_COUNT_PERFRAME; ++i) {
			CreateCommandList({CommandListType::Transfer, true});
		}
		for (int i = 0; i < RenderSettings::COMPUTE_CMDLIST_COUNT_PERFRAME; ++i) {
			CreateCommandList({CommandListType::Compute, true});
		}
	}

	void RenderDevice::CreateDescriptorSetLayouts(Vector<Vector<ShaderBinding>> const& sortedBindings) { }

	DescriptorSetBuilder::DescriptorSetBuilder() {
		// We assume that most descriptors will have less than 7 bindings
		// so we reserve this to avoid constant re-allocations
		m_Bindings.reserve(7);
	}

	DescriptorSetBuilder& DescriptorSetBuilder::BindUniformBuffer(u32 slot, BufferHandle buffer) {
		Bindings b{
			.m_Type = ShaderBindingType::UniformBuffer,
			.m_Slot = slot,
			.m_ResourceID1 = buffer.m_Value
		};
		m_Bindings.emplace_back(b);
		return *this;
	}

	DescriptorSetBuilder& DescriptorSetBuilder::BindStorageBuffer(u32 slot, BufferHandle buffer) {
		Bindings b{
			.m_Type = ShaderBindingType::StorageBuffer,
			.m_Slot = slot,
			.m_ResourceID1 = buffer.m_Value
		};
		m_Bindings.emplace_back(b);
		return *this;
	}

	DescriptorSetBuilder& DescriptorSetBuilder::BindTextureWithSampler(
		u32 slot, TextureViewHandle textureView, SamplerHandle sampler) {
		CKE_ASSERT(textureView.IsNotNull());
		CKE_ASSERT(sampler.IsNotNull());
		Bindings b{};
		b.m_Type = ShaderBindingType::ImageViewSampler;
		b.m_Slot = slot;
		b.m_ResourceID1 = textureView.m_Value;
		b.m_ResourceID2 = sampler.m_Value;
		m_Bindings.emplace_back(b);
		return *this;
	}

	DescriptorSetHandle DescriptorSetBuilder::Build() {
		return m_pDevice->CreateDescriptorSetForFrame(m_PipelineHandle, m_SetIndex, m_Bindings);
	}

	void RenderDevice::CreatePipelineDescriptorPool(Pipeline&                            pipeline,
	                                                Vector<Vector<ShaderBinding>> const& sortedBindings) {
		Vector<VkDescriptorPoolSize> poolSizes{};
		for (Vector<ShaderBinding> const& setBindings : sortedBindings) {
			for (ShaderBinding const& binding : setBindings) {
				VkDescriptorPoolSize s{};
				s.type = ConversionsVK::GetVkDescriptorType(binding.m_Type);
				s.descriptorCount = binding.m_Count;
				poolSizes.emplace_back(s);
			}
		}

		VkDescriptorPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.poolSizeCount = static_cast<u32>(poolSizes.size());
		poolInfo.pPoolSizes = poolSizes.data();
		poolInfo.maxSets = RenderSettings::MAX_OBJECTS;
		poolInfo.flags = 0;

		for (FrameData_Vk& frameData : m_Frame) {
			PipelineFrameData_Vk& pipelineFrameData = frameData.GetPipelineState(pipeline.m_DBHandle);
			if (vkCreateDescriptorPool(m_Device, &poolInfo, nullptr,
			                           &pipelineFrameData.m_DescriptorPool) !=
				VK_SUCCESS) {
				CKE_UNREACHABLE_CODE();
			}
		}
	}

	void RenderDevice::AllocateDescriptorSet(PipelineHandle pipelineHandle, PipelineLayout* pPipelineLayout, u32 setIndex,
	                                         u32            countPerFrame) {
		FrameArray<DescriptorSet*> descriptorSets = m_ResourcesDB.CreateDescriptorSet();

		for (u32 i = 0; i < RenderSettings::MAX_FRAMES_IN_FLIGHT; ++i) {
			PipelineFrameData_Vk const& frameState = m_Frame[i].GetPipelineState(pipelineHandle);

			VkDescriptorSetAllocateInfo allocInfo{};
			allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			allocInfo.descriptorPool = frameState.m_DescriptorPool;
			allocInfo.descriptorSetCount = countPerFrame;
			allocInfo.pSetLayouts = &pPipelineLayout->m_DescriptorSetLayouts[setIndex];

			Vector<VkDescriptorSet> vkSets{countPerFrame};
			if (vkAllocateDescriptorSets(m_Device, &allocInfo, vkSets.data()) != VK_SUCCESS) {
				CKE_UNREACHABLE_CODE();
			}

			for (u64 i = 0; i < vkSets.size(); ++i) {
				descriptorSets[i]->m_DescriptorSet = vkSets[i];
				descriptorSets[i]->m_LayoutIndex = setIndex;
			}
		}
	}

	DescriptorSetHandle RenderDevice::CreateDescriptorSetForFrame(PipelineHandle                          pipelineHandle,
	                                                              u32                                     layoutIndex,
	                                                              Vector<DescriptorSetBuilder::Bindings>& shaderBindings) {
		Pipeline&          pPipeline = m_ResourcesDB.GetPipeline(pipelineHandle);
		PipelineLayout*    pPipelineLayout = m_ResourcesDB.GetPipelineLayout(pPipeline.m_PipelineLayout);
		PipelineFrameData_Vk& pipelineFrameData = GetCurrentFrameData().GetPipelineState(pipelineHandle);

		// Allocate New Descriptor Set
		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = pipelineFrameData.m_DescriptorPool;
		allocInfo.descriptorSetCount = 1;
		allocInfo.pSetLayouts = &pPipelineLayout->m_DescriptorSetLayouts[layoutIndex];

		VkDescriptorSet vkDescriptorSet{};
		if (vkAllocateDescriptorSets(m_Device, &allocInfo, &vkDescriptorSet) != VK_SUCCESS) {
			CKE_UNREACHABLE_CODE();
		}

		// Because VkWriteDescriptorSet has a pointer to a buffer or image info
		// we have to store those structures somewhere until we submit the write
		// Its important that we reserve the vectors because if we don't the vector
		// will probably reallocate the resources and the pointers to them will become
		// invalid.
		static Vector<VkDescriptorBufferInfo> bufferInfos{30};
		static Vector<VkDescriptorImageInfo>  imageInfos{30};
		static Vector<VkWriteDescriptorSet>   descWrites{30};
		bufferInfos.clear();
		imageInfos.clear();
		descWrites.clear();

		for (auto const& binding : shaderBindings) {
			if (binding.m_Type == ShaderBindingType::UniformBuffer ||
				binding.m_Type == ShaderBindingType::StorageBuffer) {
				Buffer* pBuffer = m_ResourcesDB.GetBuffer(BufferHandle{binding.m_ResourceID1});

				VkDescriptorBufferInfo bufferInfo{};
				bufferInfo.buffer = pBuffer->m_vkBuffer;
				bufferInfo.offset = pBuffer->m_Offset;
				bufferInfo.range = pBuffer->m_Size;
				bufferInfos.push_back(bufferInfo);

				VkWriteDescriptorSet bufferWrite{};
				bufferWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				bufferWrite.dstSet = vkDescriptorSet;
				bufferWrite.dstBinding = binding.m_Slot;
				bufferWrite.dstArrayElement = 0;
				bufferWrite.descriptorType = ConversionsVK::GetVkDescriptorType(binding.m_Type);
				bufferWrite.descriptorCount = 1;
				bufferWrite.pBufferInfo = &bufferInfos.back();

				descWrites.push_back(bufferWrite);
			}
			else if (binding.m_Type == ShaderBindingType::ImageViewSampler) {
				TextureViewHandle viewHandle{binding.m_ResourceID1};
				SamplerHandle     samplerHandle{binding.m_ResourceID2};
				TextureView*      pTexView = m_ResourcesDB.GetTextureView(viewHandle);
				TextureSampler*   pSampler = m_ResourcesDB.GetTextureSampler(samplerHandle);

				VkDescriptorImageInfo imgInfo{};
				imgInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				imgInfo.imageView = pTexView->m_vkView;
				imgInfo.sampler = pSampler->m_vkSampler;
				imageInfos.push_back(imgInfo);

				VkWriteDescriptorSet imgWrite{};
				imgWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				imgWrite.dstSet = vkDescriptorSet;
				imgWrite.dstBinding = binding.m_Slot;
				imgWrite.dstArrayElement = 0;
				imgWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				imgWrite.descriptorCount = 1;
				imgWrite.pImageInfo = &imageInfos.back();

				descWrites.push_back(imgWrite);
			}
		}
		vkUpdateDescriptorSets(m_Device, descWrites.size(), descWrites.data(), 0, nullptr);

		DescriptorSet* pDescriptorSet = m_ResourcesDB.CreateDescriptorSetForFrame(GetFrameIdx());
		pDescriptorSet->m_DescriptorSet = vkDescriptorSet;
		pDescriptorSet->m_LayoutIndex = layoutIndex;
		pipelineFrameData.m_DescriptorSets.push_back(pDescriptorSet->m_DBHandle);
		return pDescriptorSet->m_DBHandle;
	}

	template <typename T>
	void RenderDevice::SubmitTCommandLists(Vector<T>& cmdList, CmdListSubmitInfo submitInfo, VkQueue queue) {
		static Vector<VkCommandBuffer> cmdBuffers{};
		cmdBuffers.reserve(cmdList.size());
		cmdBuffers.clear();
		for (auto& list : cmdList) {
			cmdBuffers.push_back(list.m_CmdBuffer);
		}
		SubmitCommandBuffers(cmdBuffers, queue, submitInfo);
	}

	void RenderDevice::SubmitGraphicsCommandLists(Vector<GraphicsCommandList>& cmdList,
	                                              CmdListSubmitInfo            submitInfo) {
		SubmitTCommandLists(cmdList, submitInfo, m_GraphicsQueue);
	}

	void RenderDevice::SubmitTransferCommandLists(Vector<TransferCommandList>& cmdList,
	                                              CmdListSubmitInfo            submitInfo) {
		SubmitTCommandLists(cmdList, submitInfo, m_TransferQueue);
	}

	void RenderDevice::SubmitComputeCommandLists(Vector<ComputeCommandList>& cmdList, CmdListSubmitInfo submitInfo) {
		SubmitTCommandLists(cmdList, submitInfo, m_ComputeQueue);
	}

	void RenderDevice::WaitGraphicsQueueIdle() {
		vkQueueWaitIdle(m_GraphicsQueue);
	}

	void RenderDevice::WaitTransferQueueIdle() {
		vkQueueWaitIdle(m_TransferQueue);
	}

	void RenderDevice::WaitComputeQueueIdle() {
		vkQueueWaitIdle(m_ComputeQueue);
	}

	QueueFamilyIndices RenderDevice::GetQueueFamilyIndices() {
		return m_QueueFamilyIndices;
	}

	void RenderDevice::SubmitCommandBuffers(Vector<VkCommandBuffer> const& cmdBuffers, VkQueue queue, CmdListSubmitInfo submitInfo) {
		// Convert API objects into Vulkan counterpart

		Vector<VkSemaphore> vkWaitSemaphores;
		vkWaitSemaphores.reserve(submitInfo.m_WaitSemaphores.size());
		Vector<VkPipelineStageFlags> vkWaitPipelineStages;
		vkWaitPipelineStages.reserve(submitInfo.m_WaitSemaphores.size());

		for (CmdListWaitSemaphoreInfo& waitInfo : submitInfo.m_WaitSemaphores) {
			Semaphore& s = m_ResourcesDB.GetSemaphore(waitInfo.m_Semaphore);
			vkWaitSemaphores.push_back(s.m_vkSemaphore);
			vkWaitPipelineStages.push_back(ConversionsVK::GetVkPipelineStageFlags(waitInfo.m_Stage));
		}

		Vector<VkSemaphore> signalSemaphores{};
		signalSemaphores.reserve(submitInfo.m_SignalSemaphores.size());

		for (TRenderHandle<Semaphore> semaphore : submitInfo.m_SignalSemaphores) {
			Semaphore& s = m_ResourcesDB.GetSemaphore(semaphore);
			signalSemaphores.push_back(s.m_vkSemaphore);
		}

		VkFence fence = VK_NULL_HANDLE;
		if (!submitInfo.m_SignalFence.IsNull()) {
			Fence* f = m_ResourcesDB.GetFence(submitInfo.m_SignalFence);
			fence = f->m_vkFence;
		}

		// Fill submit info and make the call

		VkSubmitInfo vkSubmitInfo{};
		vkSubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		vkSubmitInfo.waitSemaphoreCount = static_cast<u32>(vkWaitSemaphores.size());
		vkSubmitInfo.pWaitSemaphores = vkWaitSemaphores.data();
		vkSubmitInfo.pWaitDstStageMask = vkWaitPipelineStages.data();

		vkSubmitInfo.signalSemaphoreCount = static_cast<u32>(signalSemaphores.size());
		vkSubmitInfo.pSignalSemaphores = signalSemaphores.data();

		vkSubmitInfo.commandBufferCount = cmdBuffers.size();
		vkSubmitInfo.pCommandBuffers = cmdBuffers.data();

		if (vkQueueSubmit(queue, 1, &vkSubmitInfo, fence) != VK_SUCCESS) {
			std::cout << "Error submitting command buffer to queue" << std::endl;
		}
	}

	void RenderDevice::WaitForDevice() {
		vkDeviceWaitIdle(m_Device);
	}

	BufferHandle RenderDevice::CreateBuffer(BufferDesc bufferDesc) {
		return CreateBuffer_DEPR(bufferDesc, nullptr, 0);
	}
}


namespace CKE {
	VkSurfaceFormatKHR SwapchainInitialConfig::SelectSwapSurfaceFormat(
		Vector<VkSurfaceFormatKHR>& availableFormats) {
		CKE_ASSERT(availableFormats.size() > 0);

		for (VkSurfaceFormatKHR& format : availableFormats) {
			if (format.format == VK_FORMAT_B8G8R8A8_SRGB &&
				format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
				return format;
			}
		}

		return availableFormats[0];
	}

	VkPresentModeKHR SwapchainInitialConfig::SelectSwapPresentMode(
		Vector<VkPresentModeKHR>& availablePresentModes) {
		for (VkPresentModeKHR& presentMode : availablePresentModes) {
			if (presentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
				return presentMode;
			}
		}

		return VK_PRESENT_MODE_FIFO_KHR;
	}

	VkExtent2D SwapchainInitialConfig::SelectSwapExtent(VkSurfaceCapabilitiesKHR& capabilities,
	                                                    Int2                      framebufferSize) {
		if (capabilities.currentExtent.width != std::numeric_limits<u32>::max()) {
			return capabilities.currentExtent;
		}
		else {
			i32 width, height;
			width = framebufferSize.x;
			height = framebufferSize.y;

			VkExtent2D actualExtent = {
				static_cast<u32>(width),
				static_cast<u32>(height)
			};

			actualExtent.width = std::clamp(actualExtent.width,
			                                capabilities.minImageExtent.width,
			                                capabilities.maxImageExtent.width);
			actualExtent.height = std::clamp(actualExtent.height,
			                                 capabilities.minImageExtent.height,
			                                 capabilities.maxImageExtent.height);

			return actualExtent;
		}
	}
}

namespace CKE {
	void PipelineLayoutDesc::SetShaderBindings(Vector<ShaderBinding> const& bindings) {
		m_ShaderBindings = bindings;
	}

	void VertexInputLayoutDesc::SetVertexInput(Vector<VertexInputInfo> const& vertexInput) {
		for (VertexInputInfo input : vertexInput) {
			VertexInputInfo attr{};
			attr.m_Type = input.m_Type;
			attr.m_ByteSize = ConversionsVK::GetVkAttributeSize(input.m_Type);
			m_Stride += attr.m_ByteSize;
			m_VertexInput.emplace_back(attr);
		}
	}
}
