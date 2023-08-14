#pragma once

#include "CookieKat/Core/Math/Math.h"
#include "CookieKat/Core/Containers/Containers.h"

#define NOMINMAX
#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

// Model Data
//-----------------------------------------------------------------------------

using namespace CKE;

struct Vertex
{
	Vec3 m_Pos;
	Vec3 m_Color;

	static VkVertexInputBindingDescription GetBindingDesc()
	{
		VkVertexInputBindingDescription desc{};
		desc.binding = 0;
		desc.stride = sizeof(Vertex);
		desc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		return desc;
	}

	static Vector<VkVertexInputAttributeDescription> GetAttributeDesc()
	{
		Vector<VkVertexInputAttributeDescription> desc{ 2 };

		desc[0].binding = 0;
		desc[0].location = 0;
		desc[0].format = VK_FORMAT_R32G32B32_SFLOAT;
		desc[0].offset = offsetof(Vertex, m_Pos);

		desc[1].binding = 0;
		desc[1].location = 1;
		desc[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		desc[1].offset = offsetof(Vertex, m_Color);

		return desc;
	}
};

inline Vector<Vertex> triangleVertices = {
	{{0.0f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}},
	{{0.5f, 0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}},
	{{-0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}},
};

inline Vector<Vertex> rectangleVertices = {
	{{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}},
	{{0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}},
	{{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}},
	{{-0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 0.0f}},
};

inline Vector<u32> rectangleIndices = {
	0, 1, 2,
	2, 3, 0
};