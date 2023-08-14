#pragma once

#include "IWriterReader.h"

#include <rapidjson/rapidjson.h>
#include <rapidjson/document.h>

//-----------------------------------------------------------------------------

namespace CKE
{
	class JSONWriter : IWriter
	{
	public:
		JSONWriter();

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

	private:
		rapidjson::Document doc;
	};
}
