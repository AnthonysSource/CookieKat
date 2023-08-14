#pragma once

#include "CookieKat/Core/Containers/Containers.h"
#include "CookieKat/Core/Platform/PrimitiveTypes.h"

#include "CookieKat/Systems/RenderAPI/RenderingInfo.h"

namespace CKE {
	enum class AccessMask : u32
	{
		// The flag values must be in sync with
		// Vulkan flag values
		None = 0,
		Shader_Read = (1 << 5),
		Shader_Write = (1 << 6),
		ColorAttachment_Read = (1 << 7),
		ColorAttachment_Write = (1 << 8),
		DepthStencilAttachment_Read = (1 << 9),
		DepthStencilAttachment_Write = (1 << 10),
		Transfer_Read = (1 << 11),
		Transfer_Write = (1 << 12),
	};

	constexpr inline AccessMask operator&(AccessMask a, AccessMask b) {
		return static_cast<AccessMask>(static_cast<std::underlying_type<AccessMask>::type>(a) & static_cast<
			std::underlying_type<AccessMask>::type>(b));
	}

	constexpr inline AccessMask operator|(AccessMask a, AccessMask b) {
		return static_cast<AccessMask>(static_cast<std::underlying_type<AccessMask>::type>(a) | static_cast<
			std::underlying_type<AccessMask>::type>(b));
	}

	enum class PipelineStage : u32
	{
		// The flag values must be in sync with
		// Vulkan flag values
		TopOfPipe = (1 << 0),
		DrawIndirect = (1 << 1),
		VertexInput = (1 << 2),
		VertexShader = (1 << 3),
		GeometryShader = (1 << 6),
		FragmentShader = (1 << 7),
		EarlyFragmentTest = (1 << 8),
		LateFragmentTest = (1 << 9),
		ColorAttachmentOutput = (1 << 10),

		ComputeShader = (1 << 11),
		Transfer = (1 << 12),
		BottomOfPipe = (1 << 13),
		Host = (1 << 14),
		AllGraphics = (1 << 15),
		AllCommands = (1 << 16),
		None = 0,
	};

	constexpr inline PipelineStage operator&(PipelineStage a, PipelineStage b) {
		return static_cast<PipelineStage>(static_cast<std::underlying_type<PipelineStage>::type>(a) & static_cast<
			std::underlying_type<PipelineStage>::type>(b));
	}

	constexpr inline PipelineStage operator|(PipelineStage a, PipelineStage b) {
		return static_cast<PipelineStage>(static_cast<std::underlying_type<PipelineStage>::type>(a) | static_cast<
			std::underlying_type<PipelineStage>::type>(b));
	}

	enum class BlendFactor
	{
		Zero,
		One,
		SrcColor,
		OneMinusSrcColor,
		DstColor,
		OneMinusDstColor,
		SrcAlpha,
		OneMinusSrcAlpha,
		DstAlpha,
		OneMinusDstAlpha
	};

	enum class ColorComponentMask : u32
	{
		R = (1 << 0),
		G = (1 << 1),
		B = (1 << 2),
		A = (1 << 3)
	};

	constexpr inline ColorComponentMask operator&(ColorComponentMask a, ColorComponentMask b) {
		return static_cast<ColorComponentMask>(static_cast<std::underlying_type<ColorComponentMask>::type>(a) &
			static_cast<
				std::underlying_type<ColorComponentMask>::type>(b));
	}

	constexpr inline ColorComponentMask operator|(ColorComponentMask a, ColorComponentMask b) {
		return static_cast<ColorComponentMask>(static_cast<std::underlying_type<ColorComponentMask>::type>(a) |
			static_cast<
				std::underlying_type<ColorComponentMask>::type>(b));
	}

	enum class BlendOp
	{
		Add,
	};

	enum class PrimitiveTopology
	{
		TriangleList
	};

	enum class CompareOp
	{
		Never = 0,
		Less = 1,
		Equal = 2,
		LessOrEqual = 3,
		Greater = 4,
		NotEqual = 5,
		GreaterOrEqual = 6,
		Always = 7,
	};
}


namespace CKE {
	// Shader Bindings
	//-----------------------------------------------------------------------------

	enum class ShaderStageMask : u32
	{
		Vertex = 1 << 0,
		Tessellation_Control = 1 << 1,
		Tessellation_Evaluation = 1 << 2,
		Geometry = 1 << 3,
		Fragment = 1 << 4,
		Compute = 1 << 5,
	};

	enum class ShaderBindingType
	{
		UniformBuffer,
		StorageBuffer,
		ImageViewSampler
	};

	struct ShaderBinding
	{
		u64               m_SetIndex;
		u32               m_BindingPoint;
		ShaderBindingType m_Type;
		u32               m_Count;
		ShaderStageMask   m_StageMask;
	};

	// Vertex Input
	//-----------------------------------------------------------------------------

	enum class VertexInputFormat
	{
		Float_R32G32B32,
		Float_R32G32,
		UInt,
		Int,
	};

	enum class VertexInputRate
	{
		Per_Vertex,
		Per_Instance
	};

	struct VertexInputInfo
	{
		VertexInputFormat m_Type = VertexInputFormat::Float_R32G32B32;
		u32               m_Location = 0;
		u32               m_Binding = 0;

		u32 m_ByteSize = 0;
	};

	// Pipeline Layout
	//-----------------------------------------------------------------------------

	class PipelineLayoutDesc
	{
		friend class RenderDevice;

	public:
		PipelineLayoutDesc() = default;
		PipelineLayoutDesc(Vector<ShaderBinding> const& bindings) { SetShaderBindings(bindings); }

		void SetShaderBindings(Vector<ShaderBinding> const& bindings);

	private:
		Vector<ShaderBinding> m_ShaderBindings{};
	};

	struct DepthStencilState
	{
		bool      m_DepthTestEnable = false;
		bool      m_DepthWrite = false;
		CompareOp m_DepthCompareOp = CompareOp::LessOrEqual;
		bool      m_StencilEnable = false;
	};

	struct AttachmentsInfo
	{
		Vector<TextureFormat> m_ColorAttachments;
		TextureFormat         m_DepthStencil = TextureFormat::D24_UNORM_S8_UINT;
	};

	struct AttachmentBlendState
	{
		bool               m_BlendEnable = true;
		ColorComponentMask m_ColorWriteMask =
				ColorComponentMask::R |
				ColorComponentMask::G |
				ColorComponentMask::B |
				ColorComponentMask::A;

		BlendFactor m_SrcColorBlendFactor = BlendFactor::SrcAlpha;
		BlendFactor m_DstColorBlendFactor = BlendFactor::OneMinusSrcAlpha;
		BlendOp     m_ColorBlendOp = BlendOp::Add;

		BlendFactor m_SrcAlphaBlendFactor = BlendFactor::One;
		BlendFactor m_DstAlphaBlendFactor = BlendFactor::Zero;
		BlendOp     m_AlphaBlendOp = BlendOp::Add;

		AttachmentBlendState() = default;
	};

	struct BlendState
	{
		Vector<AttachmentBlendState> m_AttachmentBlendStates{};
		Array<f32, 4>                m_BlendConstants{};
	};

	struct VertexInputLayoutDesc
	{
		Vector<VertexInputInfo> m_VertexInput{};
		u32                     m_Stride{};

		void SetVertexInput(Vector<VertexInputInfo> const& vertexInput);
	};

	struct GraphicsPipelineDesc
	{
		// If the vectors have size 0 that means we don't
		// use a shader of that type
		Vector<u8> m_VertexShaderSource;
		Vector<u8> m_FragmentShaderSource;
		Vector<u8> m_TesselationControlShaderSource;
		Vector<u8> m_TesselationEvaluationShaderSource;
		Vector<u8> m_GeometryShaderSource;

		PipelineLayoutHandle  m_LayoutHandle{};
		PipelineLayoutDesc    m_LayoutDesc_DEPRECATED{};
		VertexInputLayoutDesc m_VertexInput{};
		AttachmentsInfo       m_AttachmentsInfo{};
		DepthStencilState     m_DepthStencilState{};
		BlendState            m_BlendState{};
		PrimitiveTopology     m_Topology = PrimitiveTopology::TriangleList;
	};

	struct ComputePipelineDesc
	{
		PipelineLayoutHandle m_Layout;
		Vector<u8>           m_ComputeShaderSrc;
	};
}