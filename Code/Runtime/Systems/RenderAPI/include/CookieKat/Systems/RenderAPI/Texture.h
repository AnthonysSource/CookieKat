#pragma once

#include "CookieKat/Core/Math/Math.h"
#include "CookieKat/Core/Platform/PrimitiveTypes.h"
#include "CookieKat/Core/Serialization/Archive.h"
#include "CookieKat/Systems/RenderAPI/RenderHandle.h"

namespace CKE {
	enum class TextureUsage : u32
	{
		Transfer_Src = (1 << 0),
		Transfer_Dst = (1 << 1),
		Sampled = (1 << 2),
		Storage = (1 << 3),
		Color_Attachment = (1 << 4),
		DepthStencil_Attachment = (1 << 5),
		Transient_Attachment = (1 << 6),
		Input_Attachment = (1 << 7),
	};

	constexpr inline TextureUsage operator&(TextureUsage a, TextureUsage b) {
		return static_cast<TextureUsage>(static_cast<std::underlying_type<TextureUsage>::type>(a) &
			static_cast<
				std::underlying_type<TextureUsage>::type>(b));
	}

	constexpr inline TextureUsage operator|(TextureUsage a, TextureUsage b) {
		return static_cast<TextureUsage>(static_cast<std::underlying_type<TextureUsage>::type>(a) |
			static_cast<
				std::underlying_type<TextureUsage>::type>(b));
	}

	enum class TextureAspectMask : u32
	{
		Color = (1 << 0),
		Depth = (1 << 1),
		Stencil = (1 << 2)
	};

	constexpr inline TextureAspectMask operator&(TextureAspectMask a, TextureAspectMask b) {
		return static_cast<TextureAspectMask>(static_cast<std::underlying_type<
				TextureAspectMask>::type>(a) &
			static_cast<std::underlying_type<TextureAspectMask>::type>(b));
	}

	constexpr inline TextureAspectMask operator|(TextureAspectMask a, TextureAspectMask b) {
		return static_cast<TextureAspectMask>(static_cast<std::underlying_type<
				TextureAspectMask>::type>(a) |
			static_cast<std::underlying_type<TextureAspectMask>::type>(b));
	}

	enum class TextureLayout
	{
		Undefined,
		General,
		Color_Attachment,
		DepthStencil_Attachment,
		DepthStencil_ReadOnly,
		Shader_ReadOnly,
		Transfer_Src,
		Transfer_Dst,
		Present_Src,
	};

	enum class TextureWrapMode
	{
		Repeat,
		MirroredRepeat,
		ClampToBorder,
		ClampToEdge,
	};

	enum class TextureMipmapMode
	{
		Nearest,
		Linear,
	};

	enum class TextureFilter
	{
		Nearest,
		Linear,
	};

	enum class TextureType
	{
		Tex1D,
		Tex2D,
		Tex3D,
	};

	enum class TextureFormat : u32
	{
		R8G8B8A8_SRGB,
		R8G8B8A8_UNORM,
		R8G8B8A8_USCALED,
		R8G8B8A8_SSCALED,
		D24_UNORM_S8_UINT,
		B8G8R8A8_SRGB,
		R16G16B16A16_SFLOAT,
		R32G32B32A32_SFLOAT,
		R32_UINT,
	};

	enum class TextureMiscFlags
	{
		None = 0,
		Texture_CubeMap = 1 << 0,
	};

	struct SamplerDesc
	{
		TextureWrapMode   m_WrapU = TextureWrapMode::ClampToEdge;
		TextureWrapMode   m_WrapV = TextureWrapMode::ClampToEdge;
		TextureWrapMode   m_WrapW = TextureWrapMode::ClampToEdge;
		TextureFilter     m_MagFilter = TextureFilter::Linear;
		TextureFilter     m_MinFilter = TextureFilter::Linear;
		TextureMipmapMode m_MipmapMode = TextureMipmapMode::Linear;
		bool              m_AnisotropyEnable = true;
		f32               m_MaxAnisotropy = 16.0f;

		TextureFilter m_MipMapMode = TextureFilter::Linear;
		f32           m_LodBias = 0.0f;
		f32           m_MinLod = 0.0f;
		f32           m_MaxLod = 0.0f;

		bool IsEqual(SamplerDesc const& other) const {
			return
					m_WrapU == other.m_WrapU &&
					m_WrapV == other.m_WrapV &&
					m_WrapW == other.m_WrapW &&
					m_MagFilter == other.m_MagFilter &&
					m_MinFilter == other.m_MinFilter &&
					m_MipmapMode == other.m_MipmapMode &&
					m_AnisotropyEnable == other.m_AnisotropyEnable &&
					std::abs(m_MaxAnisotropy - other.m_MaxAnisotropy) < 0.00001f;
		}
	};

	struct TextureDesc
	{
		template <typename Serializer>
			requires IsSerializer<Serializer>
		friend class CKE::Archive;

		// Well we have to do this mess until we discover how to create custom serialization functions
		// for externally defined data types
		template <typename Serializer>
			requires IsSerializer<Serializer>
		void Serialize(CKE::Archive<Serializer>& archive) {
			archive.Serialize(m_Size.x, m_Size.y, m_Size.z, m_Format);
		}

		TextureFormat     m_Format = TextureFormat::R8G8B8A8_SRGB;
		TextureType       m_TextureType = TextureType::Tex2D;
		TextureAspectMask m_AspectMask = TextureAspectMask::Color;
		TextureUsage      m_Usage = TextureUsage::Transfer_Dst | TextureUsage::Sampled;
		UInt3             m_Size = UInt3{1, 1, 1};
		u32               m_ArraySize = 1;
		u32               m_MipLevels = 1;
		u32               m_SampleCount = 1;
		TextureMiscFlags  m_MiscFlags = TextureMiscFlags::None;
		DebugString       m_Name{};
		bool              m_ConcurrentQueueUsage = true;
	};

	enum class TextureViewType
	{
		Tex1D,
		Tex2D,
		Tex3D,
		Cube,
	};

	struct TextureViewDesc
	{
		TextureHandle     m_Texture = TextureHandle{};
		TextureViewType   m_Type = TextureViewType::Tex2D;
		TextureAspectMask m_AspectMask = TextureAspectMask::Color;
		TextureFormat     m_Format = TextureFormat::R8G8B8A8_SRGB;
		u32               m_BaseMipLevel = 0;
		u32               m_MipLevelCount = 1;
		u32               m_BaseArrayLayer = 0;
		u32               m_ArrayLayerCount = 1;

		TextureViewDesc() = default;

		TextureViewDesc(TextureHandle tex, TextureViewType type, TextureAspectMask aspect,
		                TextureFormat format, u32          baseMip, u32            mipCount, u32 baseLayer,
		                u32           layerCount) :
			m_Texture{tex}, m_Type{type}, m_AspectMask{aspect}, m_Format{format},
			m_BaseMipLevel{baseMip}, m_MipLevelCount{mipCount}, m_BaseArrayLayer{baseLayer},
			m_ArrayLayerCount{layerCount} {}
	};
}
