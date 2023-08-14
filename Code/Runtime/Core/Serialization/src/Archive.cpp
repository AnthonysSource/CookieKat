#include "CookieKat/Core/Serialization/Archive.h"
#include "CookieKat/Core/Memory/Memory.h"

#include <fstream>

namespace CKE
{
	void BinaryOutputArchive::WriteToFile(char const* path)
	{
		std::ofstream of(path, std::ios::binary);
		of.write(m_Serializer.GetData(), m_Serializer.GetSizeInBytes());
	}

	BinaryInputArchive::~BinaryInputArchive()
	{
		if (m_pData != nullptr)
		{
			CKE::DeleteArray<>(m_pData);
		}
	}

	void BinaryInputArchive::ReadFromFile(char const* path)
	{
		// Open the file
		std::ifstream ifs(path, std::ios::binary);
		// If it can't be opened we crash
		if (!ifs) { CKE_UNREACHABLE_CODE(); }

		// Get file size
		ifs.seekg(0, ifs.end);
		i64 const fileSize = ifs.tellg();

		// Read raw file from the beginning
		m_pData = CKE::NewArray<char>(fileSize);
		ifs.seekg(0, ifs.beg);
		ifs.read(m_pData, fileSize);

		// Parse the entire file
		m_Serializer.BeginReading(m_pData, fileSize);
	}

	void BinaryInputArchive::ReadFromBlob(Blob const& blob)
	{
		m_Serializer.BeginReading(reinterpret_cast<char const*>(blob.data()), blob.size() * sizeof(char));
	}
}
