#include "CookieKat/Core/Containers/Containers.h"
#include "CookieKat/Core/Containers/String.h"

#include <gtest/gtest.h>

using namespace CKE;

TEST(Core_Containers, FixedString32)
{
	{
		const char* char1 = "";
		FixedString32 str{ char1 };
		EXPECT_STREQ(char1, str.c_str());
		EXPECT_EQ(strlen(char1), str.Size());

		EXPECT_EQ(str.MaxStringSize(), 30);
	}

	{
		const char* char5 = "Hehe";
		FixedString32 str{ char5 };
		EXPECT_STREQ(char5, str.c_str());
	}

	{
		const char* char30 = "aaaaaaaaaaaaaaaaaaaaaaaaaaaaa";
		FixedString32 str{ char30 };
		EXPECT_STREQ(char30, str.c_str());
	}
}

TEST(Core_Containers, FixedString32_Truncation)
{
	{
		const char* char31 = "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaa";
		FixedString32 str{ char31 };
		EXPECT_STRNE(char31, str.c_str());
	}

	{
		const char* char32 = "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa";
		FixedString32 str{ char32 };
		EXPECT_STRNE(char32, str.c_str());
	}

	{
		const char* char33 = "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa";
		FixedString32 str{ char33 };
		EXPECT_STRNE(char33, str.c_str());
	}

	{
		const char* char34 = "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa";
		FixedString32 str{ char34 };
		EXPECT_STRNE(char34, str.c_str());
	}
}

TEST(Core_Containers, StringID)
{
	StringID id{ "Hello There" };
	StringID id2{ "Hello There" };
	StringID id3{ "Oh no" };
	EXPECT_NE(id.GetID(), 0);
	EXPECT_EQ(id.GetID(), id2.GetID());
	EXPECT_NE(id.GetID(), id3.GetID());
}