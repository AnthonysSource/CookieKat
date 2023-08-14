#include "CookieKat/Core/Containers/Containers.h"
#include "CookieKat/Core/Containers/String.h"
#include "CookieKat/Core/FileSystem/FileSystem.h"

#include <gtest/gtest.h>

using namespace CKE;

TEST(FileSystem, WriteRead_TextFile)
{
	String filePath{ "FileSystem_TestWrite.txt" };
	String fileContents{ "Text to write in the txt." };
	g_FileSystem.WriteTextFile(filePath, fileContents);

	String readFile = g_FileSystem.ReadTextFile(filePath);
	ASSERT_EQ(readFile, fileContents);

	g_FileSystem.RemoveFile(filePath);
}

TEST(FileSystem, WriteRead_BinaryFile)
{
	Vector<u8> bin{ 1u,6u,3u,8u,9u,43u };
	String filePath{ "FileSystem_TestWrite.bin" };
	g_FileSystem.WriteBinaryFile(filePath, (char*)bin.data(), bin.size());

	Vector<u8> readBin = g_FileSystem.ReadBinaryFile(filePath);
	EXPECT_EQ(bin.size(), readBin.size());
	for (i32 i = 0; i < bin.size(); ++i) {
		EXPECT_EQ(bin[i], readBin[i]);
	}

	g_FileSystem.RemoveFile(filePath);
}
