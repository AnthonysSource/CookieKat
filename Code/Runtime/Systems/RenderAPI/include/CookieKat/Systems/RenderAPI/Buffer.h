#pragma once

#include "CookieKat/Core/Containers/String.h"

namespace CKE {
	enum class BufferUsage : u32
	{
		TransferSrc = 1 << 0,
		TransferDst = 1 << 1,
		UniformTexel = 1 << 2,
		StorageTexel = 1 << 3,
		Uniform = 1 << 4,
		Storage = 1 << 5,
		Index = 1 << 6,
		Vertex = 1 << 7,
		Indirect = 1 << 8,
	};

	constexpr inline BufferUsage operator&(BufferUsage a, BufferUsage b) {
		return static_cast<BufferUsage>(static_cast<std::underlying_type<BufferUsage>::type>(a) &
			static_cast<
			std::underlying_type<BufferUsage>::type>(b));
	}

	constexpr inline BufferUsage operator|(BufferUsage a, BufferUsage b) {
		return static_cast<BufferUsage>(static_cast<std::underlying_type<BufferUsage>::type>(a) |
			static_cast<
			std::underlying_type<BufferUsage>::type>(b));
	}

	enum class MemoryAccess
	{
		GPU,
		CPU_GPU
	};

	enum class UpdateFrequency
	{
		Static,
		PerFrame
	};

	// Description used to create a buffer on the GPU
	struct BufferDesc
	{
		BufferUsage     m_Usage = BufferUsage::Vertex;
		MemoryAccess    m_MemoryAccess = MemoryAccess::CPU_GPU;
		UpdateFrequency m_UpdateFrequency = UpdateFrequency::Static;
		u32             m_SizeInBytes{};
		i32             m_StrideInBytes{}; // Only necessary if its a buffer used for per-vertex or per-instance data
		DebugString     m_Name{};
		bool            m_ConcurrentSharingMode = true;

		BufferDesc() = default;

		BufferDesc(BufferUsage mType, MemoryAccess mCpuMemoryAccess, UpdateFrequency mUpdateFrequency, u32 mSizeInBytes, i32 mStrideInBytes,
		           DebugString mName)
			: m_Usage{mType},
			  m_MemoryAccess{mCpuMemoryAccess},
			  m_UpdateFrequency{mUpdateFrequency},
			  m_SizeInBytes{mSizeInBytes},
			  m_StrideInBytes{mStrideInBytes},
			  m_Name{mName} {}
	};
}
