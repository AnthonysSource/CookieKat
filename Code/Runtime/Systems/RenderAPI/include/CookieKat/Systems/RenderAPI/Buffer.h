#pragma once

#include "CookieKat/Core/Containers/String.h"

namespace CKE {
	enum class BufferUsageFlags : u32
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

	constexpr inline BufferUsageFlags operator&(BufferUsageFlags a, BufferUsageFlags b) {
		return static_cast<BufferUsageFlags>(static_cast<std::underlying_type<BufferUsageFlags>::type>(a) &
			static_cast<
				std::underlying_type<BufferUsageFlags>::type>(b));
	}

	constexpr inline BufferUsageFlags operator|(BufferUsageFlags a, BufferUsageFlags b) {
		return static_cast<BufferUsageFlags>(static_cast<std::underlying_type<BufferUsageFlags>::type>(a) |
			static_cast<
				std::underlying_type<BufferUsageFlags>::type>(b));
	}

	enum class MemoryAccess
	{
		GPU,
		CPU_GPU_Coherent
	};

	enum class DuplicationStrategy
	{
		Unique,
		PerFrameInFlight
	};

	struct QueueFamilyIndex
	{
		u32 m_Value = 0;

		QueueFamilyIndex() = default;
		QueueFamilyIndex(u32 value) : m_Value{value} {}
	};

	enum class QueueFamilyFlags : u32
	{
		Graphics = 1 << 0,
		Compute = 1 << 1,
		Transfer = 1 << 2,
		All = Graphics | Compute | Transfer,
	};

	constexpr inline QueueFamilyFlags operator&(QueueFamilyFlags a, QueueFamilyFlags b) {
		return static_cast<QueueFamilyFlags>(static_cast<std::underlying_type<QueueFamilyFlags>::type>(a) &
			static_cast<std::underlying_type<QueueFamilyFlags>::type>(b));
	}

	constexpr inline QueueFamilyFlags operator|(QueueFamilyFlags a, QueueFamilyFlags b) {
		return static_cast<QueueFamilyFlags>(static_cast<std::underlying_type<QueueFamilyFlags>::type>(a) |
			static_cast<std::underlying_type<QueueFamilyFlags>::type>(b));
	}

	// Description used to create a buffer on the GPU
	struct BufferDesc
	{
		BufferUsageFlags    m_Usage = BufferUsageFlags::Vertex;
		MemoryAccess        m_MemoryAccess = MemoryAccess::CPU_GPU_Coherent;
		DuplicationStrategy m_DuplicationStrategy = DuplicationStrategy::Unique;
		u64                 m_SizeInBytes = 0;
		i32                 m_StrideInBytes = 0; // Only necessary if its a buffer used for per-vertex or per-instance data
		bool                m_ConcurrentSharingMode = true;
		QueueFamilyFlags    m_QueueFamilies = QueueFamilyFlags::All;
		DebugString         m_DebugName{};

		BufferDesc() = default;

		BufferDesc(BufferUsageFlags mType, MemoryAccess mCpuMemoryAccess, DuplicationStrategy mUpdateFrequency, u32 mSizeInBytes,
		           i32              mStrideInBytes,
		           DebugString      mName)
			: m_Usage{mType},
			  m_MemoryAccess{mCpuMemoryAccess},
			  m_DuplicationStrategy{mUpdateFrequency},
			  m_SizeInBytes{mSizeInBytes},
			  m_StrideInBytes{mStrideInBytes},
			  m_DebugName{mName} {}
	};
}
