#pragma once

#include "CookieKat/Core/Containers/String.h"

namespace CKE
{
	// Base interface for a data writer
	class IWriter
	{
	protected:
		virtual ~IWriter() = default;

		inline virtual void Write(i8 value) = 0;
		inline virtual void Write(i16 value) = 0;
		inline virtual void Write(i32 value) = 0;
		inline virtual void Write(i64 value) = 0;

		inline virtual void Write(u8 value) = 0;
		inline virtual void Write(u16 value) = 0;
		inline virtual void Write(u32 value) = 0;
		inline virtual void Write(u64 value) = 0;

		inline virtual void Write(f32 value) = 0;
		inline virtual void Write(f64 value) = 0;

		inline virtual void Write(String const& value) = 0;
	};

	// Base interface for a data reader
	class IReader
	{
	protected:
		virtual ~IReader() = default;

		inline virtual void Read(i8& value) = 0;
		inline virtual void Read(i16& value) = 0;
		inline virtual void Read(i32& value) = 0;
		inline virtual void Read(i64& value) = 0;

		inline virtual void Read(u8& value) = 0;
		inline virtual void Read(u16& value) = 0;
		inline virtual void Read(u32& value) = 0;
		inline virtual void Read(u64& value) = 0;

		inline virtual void Read(f32& value) = 0;
		inline virtual void Read(f64& value) = 0;

		inline virtual void Read(String& value) = 0;
	};
}
