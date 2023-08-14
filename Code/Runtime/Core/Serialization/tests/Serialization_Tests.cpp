#include "CookieKat/Core/Serialization/IWriterReader.h"
#include "CookieKat/Core/Serialization/BinarySerialization.h"
#include "CookieKat/Core/Serialization/Archive.h"
#include "CookieKat/Core/Containers/Containers.h"

#include <gtest/gtest.h>

//-----------------------------------------------------------------------------

using namespace CKE;

// Utilities
//-----------------------------------------------------------------------------

constexpr bool KEEP_BINARY_FILES_AFTER_TESTS = false;

enum class EnumType : u32
{
	ValueA,
	ValueB,
	ValueC,
};

class InnerClass
{
	CKE_SERIALIZE(m_str, m_u32)

public:
	String m_str;
	u32    m_u32;

	friend bool operator==(const InnerClass& lhs, const InnerClass& rhs) {
		return lhs.m_str == rhs.m_str
				&& lhs.m_u32 == rhs.m_u32;
	}

	friend bool operator!=(const InnerClass& lhs, const InnerClass& rhs) {
		return !(lhs == rhs);
	}
};

class SerializableClass
{
	CKE_SERIALIZE(m_i32, m_u32, m_f32, m_f64, m_str, m_i32Vec, m_strVec, m_InnerClass, m_EnumA)

public:
	i32            m_i32;
	u32            m_u32;
	f32            m_f32;
	f64            m_f64;
	String         m_str;
	Vector<i32>    m_i32Vec;
	Vector<String> m_strVec;
	InnerClass     m_InnerClass;
	EnumType       m_EnumA;

	friend bool operator==(const SerializableClass& lhs, const SerializableClass& rhs) {
		return lhs.m_i32 == rhs.m_i32
				&& lhs.m_u32 == rhs.m_u32
				&& lhs.m_f32 == rhs.m_f32
				&& lhs.m_f64 == rhs.m_f64
				&& lhs.m_str == rhs.m_str
				&& lhs.m_i32Vec == rhs.m_i32Vec
				&& lhs.m_strVec == rhs.m_strVec
				&& lhs.m_InnerClass == rhs.m_InnerClass
				&& lhs.m_EnumA == rhs.m_EnumA;
	}

	friend bool operator!=(const SerializableClass& lhs, const SerializableClass& rhs) {
		return !(lhs == rhs);
	}
};

class ClassWithEnums
{
	CKE_SERIALIZE(m_EnumA, m_EnumB, m_EnumC);

public:
	EnumType m_EnumA;
	EnumType m_EnumB;
	EnumType m_EnumC;

	friend bool operator==(const ClassWithEnums& lhs, const ClassWithEnums& rhs) {
		return lhs.m_EnumA == rhs.m_EnumA
				&& lhs.m_EnumB == rhs.m_EnumB
				&& lhs.m_EnumC == rhs.m_EnumC;
	}

	friend bool operator!=(const ClassWithEnums& lhs, const ClassWithEnums& rhs) {
		return !(lhs == rhs);
	}
};

//-----------------------------------------------------------------------------

// Simple read write to file test
// T must implement an equality operator
template <typename T>
void BinaryArchive_ReadWrite(T& writeData, const char* fileName) {
	BinaryOutputArchive writeArchive{};
	writeArchive << writeData;
	writeArchive.WriteToFile(fileName);

	T                  readData;
	BinaryInputArchive readArchive{};
	readArchive.ReadFromFile(fileName);
	readArchive << readData;

	EXPECT_EQ(readData, writeData);
}

//-----------------------------------------------------------------------------

TEST(BinaryArchive, ComplexClass) {
	// Test data to write
	SerializableClass wData{};
	wData.m_i32 = -25;
	wData.m_u32 = 25;
	wData.m_f32 = 3.1415f;
	wData.m_f64 = 6.282;
	wData.m_str = "This is Sirialised";
	wData.m_i32Vec = {42, 24, 55, 69, 77};
	wData.m_strVec = {"Firsto", "Sekondo", "Thirderino"};
	wData.m_InnerClass.m_str = "InnerClass";
	wData.m_InnerClass.m_u32 = 33;
	wData.m_EnumA = EnumType::ValueC;

	const char* fileName = "automatik.hehe";
	BinaryArchive_ReadWrite<SerializableClass>(wData, fileName);
}

TEST(BinaryArchive, Vector) {
	const char* fileName = "test_vector.hehe";
	Vector<i32> writeVec{0, 1, 2, 3, 4, 5};
	BinaryArchive_ReadWrite(writeVec, fileName);
}

TEST(BinaryArchive, Array) {
	const char*   fileName = "test_array.hehe";
	Array<i32, 6> writeArray{0, 1, 2, 3, 4, 5};
	BinaryArchive_ReadWrite(writeArray, fileName);
}

TEST(BinaryArchive, Map) {
	const char*   fileName = "test_map.hehe";
	Map<i32, f64> writeMap{};
	writeMap.insert({{2, 2.22f}, {3, 3.33f}, {4, 4.44f}});
	BinaryArchive_ReadWrite(writeMap, fileName);
}

TEST(BinaryArchive, Set) {
	const char* fileName = "test_set.hehe";
	Set<i32>    writeSet{0, 1, 2, 3, 4, 5};
	BinaryArchive_ReadWrite(writeSet, fileName);
}

TEST(BinaryArchive, Pair) {
	const char*    fileName = "test_pair.hehe";
	Pair<i32, f64> writePair{9, 9.99f};
	BinaryArchive_ReadWrite(writePair, fileName);
}

TEST(BinaryArchive, MultipleValues) {
	const char* fileName = "test_multiple.hehe";
	i32         a = 2;
	f32         b = 2.22f;
	f64         c = 4.24;
	{
		BinaryOutputArchive writeArchive{};
		writeArchive.Serialize(a, b, c);
		writeArchive.WriteToFile(fileName);
	}

	i32 ar;
	f32 br;
	f64 cr;
	{
		BinaryInputArchive readArchive{};
		readArchive.ReadFromFile(fileName);
		readArchive.Serialize(ar, br, cr);
	}

	EXPECT_EQ(a, ar);
	EXPECT_EQ(b, br);
	EXPECT_EQ(c, cr);
}

TEST(BinaryArchive, Enums) {
	const char*    fileName = "enum.hehe";
	ClassWithEnums enumClass{
		EnumType::ValueA,
		EnumType::ValueB,
		EnumType::ValueC
	};
	BinaryArchive_ReadWrite(enumClass, fileName);
}
