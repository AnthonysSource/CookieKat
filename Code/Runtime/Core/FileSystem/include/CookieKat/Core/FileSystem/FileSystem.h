#pragma once

#include "CookieKat/Core/Containers/String.h"
#include "CookieKat/Core/Containers/Containers.h"

namespace CKE {
	using Path = String;

	// Container of raw bytes
	using Blob = Vector<u8>;

	//-----------------------------------------------------------------------------

	class FileSystem
	{
	public:
		// Text files
		//-----------------------------------------------------------------------------

		String ReadTextFile(const char* path) const;
		void   WriteTextFile(const char* path, const char* pData);

		void   WriteTextFile(Path const& path, String const& data);
		String ReadTextFile(Path const& path) const;

		// Binary Files
		//-----------------------------------------------------------------------------

		Blob ReadBinaryFile(const char* pPath) const;
		void WriteBinaryFile(const char* pPath, void const* pData, i64 dataSizeInBytes);

		Blob ReadBinaryFile(Path const& path) const;
		void WriteBinaryFile(Path const& path, void const* pData, i64 dataSizeInBytes);

		bool RemoveFile(const char* pPath);
		bool RemoveFile(Path const& path);
	};

	// Global File System Instance
	// We could just have a namespace called file system
	// are stateless because pretty much all of the operations
	inline FileSystem g_FileSystem{};
}
