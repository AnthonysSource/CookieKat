#include "CookieKat/Core/FileSystem/FileSystem.h"
#include "CookieKat/Core/Containers/String.h"

#include <iostream>
#include <filesystem>
#include <sstream>
#include <fstream>

namespace CKE {
	String FileSystem::ReadTextFile(const char* path) const {
		String              outputStr{};
		const std::ifstream fs(path);
		if (fs.is_open()) {
			std::ostringstream stream{};
			stream << fs.rdbuf();
			outputStr = stream.str();
		}
		else {
			// TODO: Use logging system when implemented
			printf("ERROR: Failed to read a file [%s]\n", path);
		}
		return outputStr;
	}

	String FileSystem::ReadTextFile(Path const& path) const {
		return ReadTextFile(path.c_str());
	}

	void FileSystem::WriteTextFile(const char* path, const char* pData) {
		std::ofstream ofs(path);
		if (ofs.is_open()) {
			ofs << pData;
		}
		else {
			// TODO: Use logging system when implemented
			printf("ERROR: Failed to write a file [%s]\n", path);
		}
	}

	void FileSystem::WriteTextFile(Path const& path, String const& data) {
		WriteTextFile(path.c_str(), data.c_str());
	}

	Blob FileSystem::ReadBinaryFile(const char* pPath) const {
		std::ifstream ifs(pPath, std::ios::binary);
		CKE_ASSERT(!ifs.fail());
		Blob blob(std::istreambuf_iterator(ifs), {});
		return blob;
	}

	void FileSystem::WriteBinaryFile(Path const& path, void const* pData, i64 dataSizeInBytes) {
		WriteBinaryFile(path.c_str(), pData, dataSizeInBytes);
	}

	bool FileSystem::RemoveFile(const char* pPath) {
		return !std::remove(pPath);
	}

	bool FileSystem::RemoveFile(Path const& path) {
		return RemoveFile(path.c_str());
	}

	void FileSystem::WriteBinaryFile(const char* pPath, void const* pData, i64 dataSizeInBytes) {
		std::ofstream ofs(pPath, std::ios::binary);
		ofs.write((char*)pData, dataSizeInBytes);
	}

	Blob FileSystem::ReadBinaryFile(Path const& path) const {
		return ReadBinaryFile(path.c_str());
	}
}
