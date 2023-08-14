#include "CookieKat/Core/Serialization/BinarySerialization.h"

namespace CKE
{
	// Binary Writer
	//-----------------------------------------------------------------------------

	void BinaryWriter::Write(i8 value) { WritePrimitiveType(value); }
	void BinaryWriter::Write(i16 value) { WritePrimitiveType(value); }
	void BinaryWriter::Write(i32 value) { WritePrimitiveType(value); }
	void BinaryWriter::Write(i64 value) { WritePrimitiveType(value); }

	void BinaryWriter::Write(u8 value) { WritePrimitiveType(value); }
	void BinaryWriter::Write(u16 value) { WritePrimitiveType(value); }
	void BinaryWriter::Write(u32 value) { WritePrimitiveType(value); }
	void BinaryWriter::Write(u64 value) { WritePrimitiveType(value); }

	void BinaryWriter::Write(f32 value) { WritePrimitiveType(value); }
	void BinaryWriter::Write(f64 value) { WritePrimitiveType(value); }

	void BinaryWriter::Write(String const& value)
	{
		u64 strSize = value.size();
		Write(strSize);
		m_pData.resize(m_pData.size() + strSize * sizeof(char));
		memcpy(m_pData.data() + m_SizeInBytes, value.c_str(), strSize * sizeof(char));
		m_SizeInBytes += strSize * sizeof(char);
	}

	void BinaryWriter::WriteBlob(void* pData, u64 sizeInBytes)
	{
		m_pData.resize(m_pData.size() + sizeInBytes);
		memcpy(m_pData.data() + m_SizeInBytes, pData, sizeInBytes);
		m_SizeInBytes += sizeInBytes;
	}

	// Binary Reader
	//-----------------------------------------------------------------------------

	void BinaryReader::BeginReading(char const* pData, u64 sizeInBytes)
	{
		CKE_ASSERT(pData != nullptr);

		m_pData = pData;
		m_SizeInBytes = sizeInBytes;
		m_CurrByteOffset = 0;
	}

	void BinaryReader::Read(i8& value) { ReadPrimitiveType(value); }
	void BinaryReader::Read(i16& value) { ReadPrimitiveType(value); }
	void BinaryReader::Read(i32& value) { ReadPrimitiveType(value); }
	void BinaryReader::Read(i64& value) { ReadPrimitiveType(value); }

	void BinaryReader::Read(u8& value) { ReadPrimitiveType(value); }
	void BinaryReader::Read(u16& value) { ReadPrimitiveType(value); }
	void BinaryReader::Read(u32& value) { ReadPrimitiveType(value); }
	void BinaryReader::Read(u64& value) { ReadPrimitiveType(value); }

	void BinaryReader::Read(f32& value) { ReadPrimitiveType(value); }
	void BinaryReader::Read(f64& value) { ReadPrimitiveType(value); }

	void BinaryReader::Read(String& value)
	{
		CKE_ASSERT(m_pData != nullptr);

		// Read string size
		u64 strSize = 0;
		Read(strSize);

		// Generate temp string buffer
		char* tempBuffer = new char[strSize + 1];

		memcpy(tempBuffer, m_pData + m_CurrByteOffset, strSize * sizeof(char));
		m_CurrByteOffset += strSize * sizeof(char);

		tempBuffer[strSize] = '\0';
		value = tempBuffer;
		delete[] tempBuffer;
	}

	void BinaryReader::ReadBlob(void* pData, u64 sizeInBytes)
	{
		CKE_ASSERT(m_pData != nullptr);

		memcpy(pData, m_pData + m_CurrByteOffset, sizeInBytes);
		m_CurrByteOffset += sizeInBytes;
	}
}
