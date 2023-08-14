#pragma once

#include "CookieKat/Core/Platform/PrimitiveTypes.h"
#include "CookieKat/Systems/RenderAPI/Pipeline.h"
#include "CookieKat/Systems/RenderAPI/Texture.h"
#include "CookieKat/Systems/RenderAPI/Buffer.h"

#include <vulkan/vulkan_core.h>

namespace CKE {
	class ConversionsVK
	{
	public:
		static VkShaderStageFlags GetVkShaderStageFlags(ShaderStageMask shaderStage) {
			return (VkShaderStageFlags)shaderStage;
		};

		static VkBufferUsageFlags GetVkBufferUsageFlags(BufferUsage bufferUsage) {
			return static_cast<VkBufferUsageFlags>(bufferUsage);
		}

		static VkMemoryPropertyFlags GetVkMemoryProperties(MemoryAccess memoryAccess) {
			VkMemoryPropertyFlags vkFlags{};
			switch (memoryAccess) {
			case MemoryAccess::GPU: vkFlags = vkFlags | VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
				break;
			case MemoryAccess::CPU_GPU:
				vkFlags = vkFlags | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
				break;
			default: CKE_UNREACHABLE_CODE();
			}
			return vkFlags;
		}

		static VkImageType GetVkImageType(TextureType type) {
			switch (type) {
			case TextureType::Tex1D: return VK_IMAGE_TYPE_1D;
			case TextureType::Tex2D: return VK_IMAGE_TYPE_2D;
			case TextureType::Tex3D: return VK_IMAGE_TYPE_3D;
			default: CKE_UNREACHABLE_CODE();
			}
			return (VkImageType)0;
		}

		static VkFilter GetVkFilter(TextureFilter filter) {
			switch (filter) {
			case TextureFilter::Nearest: return VK_FILTER_NEAREST;
			case TextureFilter::Linear: return VK_FILTER_LINEAR;
			default: CKE_UNREACHABLE_CODE();
			}
			return (VkFilter)0;
		}

		static VkDescriptorType GetVkDescriptorType(ShaderBindingType type) {
			switch (type) {
			case ShaderBindingType::UniformBuffer: return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			case ShaderBindingType::StorageBuffer: return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			case ShaderBindingType::ImageViewSampler: return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			default: CKE_UNREACHABLE_CODE();
			}
			return (VkDescriptorType)0;
		}

		static VkSamplerMipmapMode GetVkMipMapMode(TextureFilter filter) {
			switch (filter) {
			case TextureFilter::Nearest: return VK_SAMPLER_MIPMAP_MODE_NEAREST;
			case TextureFilter::Linear: return VK_SAMPLER_MIPMAP_MODE_LINEAR;
			default: CKE_UNREACHABLE_CODE();
			}
			return (VkSamplerMipmapMode)0;
		}

		static VkColorComponentFlags GetVkColorComponentFlags(ColorComponentMask colorMask) {
			return (VkColorComponentFlags)colorMask;
		}

		static VkBlendFactor GetVkBlendFactor(BlendFactor blendFactor) {
			switch (blendFactor) {
			case BlendFactor::Zero: return VK_BLEND_FACTOR_ZERO;
			case BlendFactor::One: return VK_BLEND_FACTOR_ONE;
			case BlendFactor::SrcColor: return VK_BLEND_FACTOR_SRC_COLOR;
			case BlendFactor::OneMinusSrcColor: return VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
			case BlendFactor::DstColor: return VK_BLEND_FACTOR_DST_COLOR;
			case BlendFactor::OneMinusDstColor: return VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
			case BlendFactor::SrcAlpha: return VK_BLEND_FACTOR_SRC_ALPHA;
			case BlendFactor::OneMinusSrcAlpha: return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
			case BlendFactor::DstAlpha: return VK_BLEND_FACTOR_DST_ALPHA;
			case BlendFactor::OneMinusDstAlpha: return VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
			default: CKE_UNREACHABLE_CODE();
			}
			return (VkBlendFactor)0;
		}

		static VkBlendOp GetVkBlendOp(BlendOp blendOp) {
			switch (blendOp) {
			case BlendOp::Add: return VK_BLEND_OP_ADD;
			default: CKE_UNREACHABLE_CODE();
			}
			return (VkBlendOp)0;
		}

		static VkPrimitiveTopology GetVkPrimitiveTopology(PrimitiveTopology topology) {
			switch (topology) {
			case PrimitiveTopology::TriangleList: return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
			default: CKE_UNREACHABLE_CODE();
			}
			return (VkPrimitiveTopology)0;
		}

		static VkSamplerAddressMode GetVkSamplerAdressMode(TextureWrapMode wrap) {
			switch (wrap) {
			case TextureWrapMode::Repeat: return VK_SAMPLER_ADDRESS_MODE_REPEAT;
			case TextureWrapMode::ClampToBorder: return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
			case TextureWrapMode::ClampToEdge: return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
			case TextureWrapMode::MirroredRepeat: return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
			default: CKE_UNREACHABLE_CODE();
			}
			return (VkSamplerAddressMode)0;
		}

		static VkImageViewType GetVkImageViewType(TextureViewType type) {
			switch (type) {
			case TextureViewType::Tex1D: return VK_IMAGE_VIEW_TYPE_1D;
			case TextureViewType::Tex2D: return VK_IMAGE_VIEW_TYPE_2D;
			case TextureViewType::Tex3D: return VK_IMAGE_VIEW_TYPE_3D;
			case TextureViewType::Cube: return VK_IMAGE_VIEW_TYPE_CUBE;
			default: CKE_UNREACHABLE_CODE();
			}
			return (VkImageViewType)0;
		}

		static VkImageAspectFlags GetVkImageAspectFlags(TextureAspectMask type) {
			return static_cast<std::underlying_type<TextureAspectMask>::type>(type);
		}

		static VkFormat GetVkVertexFormat(VertexInputFormat const type) {
			switch (type) {
			case VertexInputFormat::Float_R32G32B32: return VK_FORMAT_R32G32B32_SFLOAT;
			case VertexInputFormat::Float_R32G32: return VK_FORMAT_R32G32_SFLOAT;
			case VertexInputFormat::UInt: break;
			case VertexInputFormat::Int: break;
			default: CKE_UNREACHABLE_CODE();
			}
			return (VkFormat)0;
		}

		static VkImageUsageFlags GetVkImageUsageFlags(TextureUsage usage) {
			return static_cast<std::underlying_type<TextureUsage>::type>(usage);
		}

		static VkSampleCountFlagBits GetVkSampleCountFlags(u32 samples) {
			if (samples == 1) {
				return VK_SAMPLE_COUNT_1_BIT;
			}
			else if (samples == 2) {
				return VK_SAMPLE_COUNT_2_BIT;
			}
			else if (samples == 4) {
				return VK_SAMPLE_COUNT_4_BIT;
			}
			else if (samples == 8) {
				return VK_SAMPLE_COUNT_8_BIT;
			}
			else if (samples == 16) {
				return VK_SAMPLE_COUNT_16_BIT;
			}
			else if (samples == 32) {
				return VK_SAMPLE_COUNT_32_BIT;
			}
			else if (samples == 64) {
				return VK_SAMPLE_COUNT_64_BIT;
			}
			CKE_UNREACHABLE_CODE();
			return (VkSampleCountFlagBits)0;
		}

		static VkImageCreateFlags GetVkImageCreateFlags(TextureMiscFlags flags) {
			VkImageCreateFlags vkFlags = 0;
			if ((u32)flags & (u32)TextureMiscFlags::Texture_CubeMap) {
				vkFlags = vkFlags | VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
			}
			return vkFlags;
		}

		static VkFormat GetVkImageFormat(TextureFormat const format) {
			switch (format) {
			case TextureFormat::R8G8B8A8_SRGB: return VK_FORMAT_R8G8B8A8_SRGB;
			case TextureFormat::R8G8B8A8_UNORM: return VK_FORMAT_R8G8B8A8_UNORM;
			case TextureFormat::R8G8B8A8_USCALED: return VK_FORMAT_R8G8B8A8_USCALED;
			case TextureFormat::R8G8B8A8_SSCALED: return VK_FORMAT_R8G8B8A8_SSCALED;
			case TextureFormat::R16G16B16A16_SFLOAT: return VK_FORMAT_R16G16B16A16_SFLOAT;
			case TextureFormat::R32G32B32A32_SFLOAT: return VK_FORMAT_R32G32B32A32_SFLOAT;
			case TextureFormat::D24_UNORM_S8_UINT: return VK_FORMAT_D24_UNORM_S8_UINT;
			case TextureFormat::B8G8R8A8_SRGB: return VK_FORMAT_B8G8R8A8_SRGB;
			case TextureFormat::R32_UINT: return VK_FORMAT_R32_UINT;
			default: CKE_UNREACHABLE_CODE();
			}
			return (VkFormat)0;
		}

		static u32 GetVkAttributeSize(VertexInputFormat const type) {
			switch (type) {
			case VertexInputFormat::Float_R32G32B32: return sizeof(f32) * 3;
			case VertexInputFormat::Float_R32G32: return sizeof(f32) * 2;
			case VertexInputFormat::UInt: break;
			case VertexInputFormat::Int: break;
			default: CKE_UNREACHABLE_CODE();
			}
			return 0;
		}

		static VkAccessFlags GetVkAccessFlags(AccessMask access) {
			return static_cast<std::underlying_type<AccessMask>::type>(access);
		}

		static VkPipelineStageFlags GetVkPipelineStageFlags(PipelineStage stage) {
			return static_cast<std::underlying_type<PipelineStage>::type>(stage);
		}

		static VkImageLayout GetVkImageLayout(TextureLayout layout) {
			switch (layout) {
			case TextureLayout::Undefined: return VK_IMAGE_LAYOUT_UNDEFINED;
			case TextureLayout::General: return VK_IMAGE_LAYOUT_GENERAL;
			case TextureLayout::Color_Attachment: return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			case TextureLayout::DepthStencil_Attachment: return
						VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			case TextureLayout::DepthStencil_ReadOnly: return
						VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
			case TextureLayout::Shader_ReadOnly: return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			case TextureLayout::Transfer_Src: return VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
			case TextureLayout::Transfer_Dst: return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			case TextureLayout::Present_Src: return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
			default: CKE_UNREACHABLE_CODE();
			}
			return (VkImageLayout)0;
		}

		static VkAttachmentLoadOp GetVkLoadOp(LoadOp op) {
			switch (op) {
			case LoadOp::Load: return VK_ATTACHMENT_LOAD_OP_LOAD;
			case LoadOp::Clear: return VK_ATTACHMENT_LOAD_OP_CLEAR;
			case LoadOp::DontCare: return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			default: CKE_UNREACHABLE_CODE();
			}
			return (VkAttachmentLoadOp)0;
		}

		static VkAttachmentStoreOp GetVkStoreOp(StoreOp op) {
			switch (op) {
			case StoreOp::Store: return VK_ATTACHMENT_STORE_OP_STORE;
			case StoreOp::None: return VK_ATTACHMENT_STORE_OP_NONE;
			case StoreOp::DontCare: return VK_ATTACHMENT_STORE_OP_DONT_CARE;
			default: CKE_UNREACHABLE_CODE();
			}
			return (VkAttachmentStoreOp)0;
		}
	};
}