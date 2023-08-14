#include <algorithm>

#include "VulkanDevice.h"
#include "RenderBackend.h"

#include <iostream>
#include <optional>
#include "CookieKat/Core/Memory/Memory.h"

#include <glm/gtc/type_ptr.hpp>
#include <stb_image.h>

using namespace CKE;

//-----------------------------------------------------------------------------

static void GLFW_FrameBufferResizeCallback(GLFWwindow* pWindow, i32 width, i32 height)
{
	auto device = reinterpret_cast<VulkanDevice*>(glfwGetWindowUserPointer(g_pWindow));
	device->TriggerFrameBufferResized();
}

//-----------------------------------------------------------------------------

int main()
{
	//glfwInit();

	//glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	//g_pWindow = glfwCreateWindow(1280, 720, "Vulkanik CookieKat", nullptr, nullptr);
	//glfwSetFramebufferSizeCallback(g_pWindow, GLFW_FrameBufferResizeCallback);

	//VulkanBackend vulkanBackend{};

	//vulkanBackend.Initialize();
	//VulkanDevice d = vulkanBackend.CreateLogicalDevice();
	//glfwSetWindowUserPointer(g_pWindow, &d);

	//d.Initialize();

	//// Create Object Buffers

	//VkDeviceSize vertexBufferSize = sizeof(rectangleVertices[0]) * rectangleVertices.size();
	//VkDeviceSize indexBufferSize = sizeof(rectangleIndices[0]) * rectangleIndices.size();
	//VkDeviceSize totalBufferSize = vertexBufferSize + indexBufferSize;

	//// Staging Buffer
	//VkBufferUsageFlags bufferUsage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	//int memoryProperties =
	//	VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
	//	VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	//Buffer stagingBuffer = d.CreateBuffer(totalBufferSize, bufferUsage, memoryProperties);

	//void* mappedPtr;
	//d.MapMemory(stagingBuffer.m_Memory, 0, totalBufferSize, &mappedPtr);
	//memcpy(mappedPtr, rectangleVertices.data(), vertexBufferSize);
	//memcpy((u8*)mappedPtr + vertexBufferSize, rectangleIndices.data(), indexBufferSize);
	//d.UnmapMemory(stagingBuffer.m_Memory);

	//// Actual Buffer
	//bufferUsage =
	//	VK_BUFFER_USAGE_TRANSFER_DST_BIT |
	//	VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |
	//	VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
	//memoryProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	//Buffer testObjectBuffer = d.CreateBuffer(totalBufferSize, bufferUsage, memoryProperties);

	//TransferContext transferContext = d.GetTransferContext();
	//transferContext.Begin();
	//transferContext.CopyBuffer(stagingBuffer, testObjectBuffer, totalBufferSize);
	//transferContext.End();
	//d.SubmitTransferWork(transferContext);

	//d.DestroyBuffer(stagingBuffer);

	//// Uniform Buffer

	//// Load texture and store it in a staging buffer
	//i32 w, h, nChannels;
	//u8* tex = stbi_load("cookie.png", &w, &h, &nChannels, STBI_rgb_alpha);

	//Pipeline pipeline = d.CreatePipeline();

	//while (!glfwWindowShouldClose(g_pWindow))
	//{
	//	glfwPollEvents();

	//	u32 imageIndex = d.AcquireNextImage();
	//	GraphicsContext ctx = d.GetGraphicsContext();

	//	ctx.Begin();
	//	ctx.BeginRenderPass(d.m_RenderPass, imageIndex);
	//	ctx.SetPipeline(d.m_GraphicsPipeline);

	//	VkViewport viewport{};
	//	viewport.x = 0.0f;
	//	viewport.y = 0.0f;
	//	viewport.width = static_cast<f32>(d.GetSwapChainExtent().width);
	//	viewport.height = static_cast<f32>(d.GetSwapChainExtent().height);
	//	viewport.minDepth = 0.0f;
	//	viewport.maxDepth = 1.0f;
	//	ctx.SetViewport(viewport);

	//	VkRect2D scissor{};
	//	scissor.offset = { 0, 0 };
	//	scissor.extent = d.GetSwapChainExtent();
	//	ctx.SetScissor(scissor);

	//	Mat4 view = glm::lookAt(glm::vec3(0.0f, 0.0f, -2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	//	Mat4 projection = glm::perspective(glm::radians(45.f), (f32)d.m_SwapChain.m_Extent.width / d.m_SwapChain.m_Extent.height, 0.1f, 10.0f);
	//	projection[1][1] *= -1;
	//	Mat4 model = Mat4(1.0f);
	//	Mat4 MVP = projection * view * model;
	//	PushConstants ct{ MVP };

	//	memcpy(uniformBuffersMapped[d.m_Frame.m_Idx], &MVP, sizeof(Mat4));

	//	ctx.SetVertexBuffer(testObjectBuffer);
	//	ctx.BindDescriptor(d.m_PipelineLayout, d.m_DescriptorSets[d.m_Frame.m_Idx]);
	//	ctx.SetIndexBuffer(testObjectBuffer, vertexBufferSize);
	//	ctx.PushConstant(d.m_PipelineLayout, sizeof(PushConstants), &ct);
	//	ctx.DrawIndexed(static_cast<u32>(rectangleIndices.size()), 0);

	//	ctx.EndRenderPass();
	//	ctx.End();

	//	d.SubmitGraphicsWork(ctx);

	//	d.Present();
	//}

	//d.WaitIdle();

	//for (auto ub : uniformBuffers) { d.DestroyBuffer(ub); }
	//d.DestroyBuffer(testObjectBuffer);
	//d.DestroyPipeline(pipeline);

	//d.Shutdown();
	//vulkanBackend.Shutdown();

	//glfwDestroyWindow(g_pWindow);
	//glfwTerminate();

	//return 0;
}
