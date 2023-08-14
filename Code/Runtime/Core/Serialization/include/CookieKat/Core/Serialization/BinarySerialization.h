#pragma once

#include "IWriterReader.h"

#include "CookieKat/Core/Platform/Asserts.h"
#include "CookieKat/Core/FileSystem/FileSystem.h"

#include <iostream>

//-----------------------------------------------------------------------------

namespace CKE
{
	class BinaryWriter : public IWriter
	{
	public:
		BinaryWriter() = default;

		// Returns a pointer to the in memory buffer that is being written to
		inline char const* GetData() const { return m_pData.data(); }
		inline u64         GetSizeInBytes() const { return m_SizeInBytes; }

		inline Vector<char> const& GetDataVec() { return m_pData; }

		void Write(i8 value) override;
		void Write(i16 value) override;
		void Write(i32 value) override;
		void Write(i64 value) override;

		void Write(u8 value) override;
		void Write(u16 value) override;
		void Write(u32 value) override;
		void Write(u64 value) override;

		void Write(f32 value) override;
		void Write(f64 value) override;

		void Write(String const& value) override;

		void WriteBlob(void* pData, u64 sizeInBytes);

	private:
		template <typename T>
		inline void WritePrimitiveType(T& value);

	private:
		Vector<char> m_pData;
		u64          m_SizeInBytes = 0;
	};

	//-----------------------------------------------------------------------------

	class BinaryReader : public IReader
	{
	public:
		BinaryReader() = default;

		void BeginReading(const char* pData, u64 sizeInBytes);

		void Read(i8& value) override;
		void Read(i16& value) override;
		void Read(i32& value) override;
		void Read(i64& value) override;

		void Read(u8& value) override;
		void Read(u16& value) override;
		void Read(u32& value) override;
		void Read(u64& value) override;

		void Read(f32& value) override;
		void Read(f64& value) override;

		void Read(String& value) override;

		void ReadBlob(void* pData, u64 sizeInBytes);

	private:
		template <typename T>
		inline void ReadPrimitiveType(T& value);

	private:
		const char* m_pData{nullptr};
		u64         m_SizeInBytes{0};
		u64         m_CurrByteOffset{0};
	};
}


//-----------------------------------------------------------------------------


namespace CKE
{
	template <typename T>
	void BinaryWriter::WritePrimitiveType(T& value)
	{
		m_pData.resize(m_pData.size() + sizeof(T));
		memcpy(m_pData.data() + m_SizeInBytes, &value, sizeof(T));
		m_SizeInBytes += sizeof(T);
	}

	template <typename T>
	void BinaryReader::ReadPrimitiveType(T& value)
	{
		CKE_ASSERT(m_pData != nullptr);
		memcpy(&value, m_pData + m_CurrByteOffset, sizeof(T));
		m_CurrByteOffset += sizeof(T);
	}
}
