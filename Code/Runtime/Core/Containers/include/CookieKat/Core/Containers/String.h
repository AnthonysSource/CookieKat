#pragma once

#include "CookieKat/Core/Platform/Asserts.h"
#include "CookieKat/Core/Platform/PrimitiveTypes.h"

#include <string>

#include "Containers.h"

// Forward Declarations
namespace CKE {
	template <u16 ByteSize>
	class FixedString;
}

namespace CKE {
	using String = std::string;

	//-----------------------------------------------------------------------------

	class DebugString
	{
	public:
		DebugString() = default;
		DebugString(const char* pStr) : m_pStr{ pStr } {}

		inline char const* GetStr() { return m_pStr; }

	private:
		char const* m_pStr{ nullptr };
	};

	// Compile time u64 ID generated from a string
	class StringID
	{
	public:
		consteval StringID(const char* str);

		inline u64 GetID() const { return m_HashID; }

	private:
		consteval u64 Hash(const char* str);

	private:
		u64 m_HashID{ 0 };
	};

	// String with a compile time stack size and an extra dynamically allocated heap buffer
	template <u64 ByteSize>
	class InlineString
	{
	public:
		// TODO
	private:
		char  m_StrBuffer[ByteSize]{};
		char* m_pDynamicExtraBuffer{ nullptr };
		u32   m_DynamicExtraBufferSize{ 0 };
	};

	//-----------------------------------------------------------------------------

	// 32 Byte stack string that can hold 30 characters
	using FixedString32 = FixedString<32>;

	// 64 Byte stack string that can hold 62 characters
	using FixedString64 = FixedString<64>;

	// 128 Byte stack string that can hold 126 characters
	using FixedString128 = FixedString<128>;

	// A "ByteSize" bytes stack string that can hold "ByteSize-2" characters
	template <u16 ByteSize>
	class FixedString
	{
	public:
		FixedString();
		FixedString(const char* str);

		inline u64 Size() const;
		inline u16 MaxStringSize() const;

		inline FixedString<ByteSize>& operator=(const char* str);
		inline FixedString<ByteSize>& operator=(FixedString str);

		inline const char* c_str();

	private:
		u16  m_Size{};
		char m_StrBuffer[ByteSize - 2]{};
	};
}


namespace CKE{
	//-----------------------------------------------------------------------------

	consteval u64 StringID::Hash(const char* str) {
		u64 pos = 0;
		u64 hash{14695981039346656037u};
		while (str[pos] != '\0') {
			hash = hash ^ str[pos];
			hash = hash * 1099511628211;
			pos++;
		}
		return hash;
	}

	consteval StringID::StringID(const char* str) {
		m_HashID = Hash(str);
	}

	//-----------------------------------------------------------------------------

	template <u16 ByteSize>
	FixedString<ByteSize>::FixedString() {
		m_StrBuffer[0] = '\0';
		m_Size = 0;
	}

	template <u16 ByteSize>
	FixedString<ByteSize>::FixedString(const char* str) {
		operator=(str);
	}

	template <u16 ByteSize>
	FixedString<ByteSize>& FixedString<ByteSize>::operator=(const char* str) {
		u16 fullSize = strlen(str);
		// Raise a warning if the string is bigger
		// than the fixedString size
		CKE_ASSERT(fullSize <= 65534);
		u16 size = static_cast<u16>(fullSize);
		u16 copiedSize = std::min(size, static_cast<u16>(ByteSize - 3));
		memcpy(m_StrBuffer, str, copiedSize);
		m_StrBuffer[copiedSize] = '\0';
		m_Size = copiedSize;
		return *this;
	}

	template <u16 ByteSize>
	FixedString<ByteSize>& FixedString<ByteSize>::operator=(FixedString str) {
		operator=(str.c_str());
		return *this;
	}

	template <u16 ByteSize>
	u64 FixedString<ByteSize>::Size() const {
		return m_Size;
	}

	template <u16 ByteSize>
	u16 FixedString<ByteSize>::MaxStringSize() const {
		return ByteSize - 2;
	}

	template <u16 ByteSize>
	const char* FixedString<ByteSize>::c_str() {
		return m_StrBuffer;
	}
}
