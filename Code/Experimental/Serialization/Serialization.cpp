#include "CookieKat/Core/Platform/PlatformTime.h"
#include "CookieKat/Core/Containers/String.h"
#include "CookieKat/Core/Containers/Containers.h"

#include <iostream>
#include <fstream>

using namespace CKE;

enum class Type : i32
{
	A = 0,
	B = 1
};

std::ostream& operator<<(std::ostream& os, Type& type)
{
	switch (type)
	{
	case Type::A:
		os << "Type A";
		break;
	case Type::B:
		os << "Type B";
		break;
	default:;
	}
	return os;
}

struct Header
{
	Type m_AssetType = Type::A;
	String m_Path = "data:://Buildings/modern_01_roof.mat";
	Vector<String> m_Dependencies = {
		"data:://Buildings/modern_01_roof_albedo.tex",
		"data:://Buildings/modern_01_roof_normal.tex",
		"data:://Buildings/modern_01_roof_metalic.tex",
		"data:://Buildings/modern_01_roof_ao.tex",
	};
};

struct TypeA
{
	u32 m_A = 42;
	i32 m_B = 24;

	friend std::ostream& operator<<(std::ostream& os, const TypeA& dt)
	{
		os << dt.m_A << "/" << dt.m_B;
		return os;
	}
};

struct TypeB
{
	u64 m_A = 2000;

	friend std::ostream& operator<<(std::ostream& os, const TypeB& dt)
	{
		os << dt.m_A;
		return os;
	}
};

int main()
{
	// Write
	//-------------------------------------------------------------------
	const char* FILE_NAME = "a.ckf";

	Header hdr{
		.m_AssetType = Type::A,
		.m_Path = "This/Is/A/Path.ckat",
	};
	TypeA typeA{};
	TypeB typeB{};
	float f = 1.0f;

	std::ofstream outputStream(FILE_NAME);
	if (outputStream.is_open())
	{
		u32 strSize = hdr.m_Path.size();
		outputStream.write((const char*)&hdr.m_AssetType, sizeof(Type));
		outputStream.write((const char*)&strSize, sizeof(u32));
		outputStream.write((const char*)hdr.m_Path.c_str(), strSize * sizeof(char));
		outputStream.write((const char*)&typeA, sizeof(TypeA));
	}
	outputStream.close();

	// Read
	//-------------------------------------------------------------------
	std::ifstream s(FILE_NAME, std::ios::binary);
	if (s.is_open())
	{
		// Read the header

		// Path Buffer
		char str[1024];
		u32 strSize = 20;
		s.read(reinterpret_cast<char*>(&hdr.m_AssetType), sizeof(Type));
		s.read(reinterpret_cast<char*>(&strSize), sizeof(u32));
		s.read(reinterpret_cast<char*>(&str), strSize * sizeof(char));
		str[strSize] = '\0';
		hdr.m_Path = str;

		std::cout << hdr.m_AssetType << std::endl;
		std::cout << strSize << " -> " << hdr.m_Path << std::endl;

		// Generic Data Container
		char data[50]{};

		if (hdr.m_AssetType == Type::A)
		{
			s.read(data, sizeof(TypeA));
			auto pType = reinterpret_cast<TypeA*>(&data);
			std::cout << *pType << std::endl;
		}
		else if (hdr.m_AssetType == Type::B)
		{
			s.read(data, sizeof(TypeB));
			auto pType = reinterpret_cast<TypeB*>(&data);
			std::cout << *pType << std::endl;
		}
	}

	return 0;
}