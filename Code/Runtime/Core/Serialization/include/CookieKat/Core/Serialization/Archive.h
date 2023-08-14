#pragma once

#include "IWriterReader.h"
#include "BinarySerialization.h"

#include "CookieKat/Core/Containers/Containers.h"
#include "CookieKat/Core/Containers/String.h"

#include <type_traits>

//-----------------------------------------------------------------------------

// Backends
//	MPack
//	Json
// Tests
// Data validation checks

//-----------------------------------------------------------------------------

namespace CKE {
	template <typename Serializer>
	concept IsSerializer = std::is_base_of_v<CKE::IWriter, Serializer> || std::is_base_of_v<CKE::IReader, Serializer>;

	// Base Archive Logic
	//-----------------------------------------------------------------------------

	template <typename Serializer>
		requires IsSerializer<Serializer>
	class Archive
	{
	public:
		Archive() = default;

		//-----------------------------------------------------------------------------

		// Process an individual T type
		template <typename T>
		Archive& operator<<(T& value);

		//-----------------------------------------------------------------------------

		template <typename T>
		Archive& operator<<(Vector<T>& vector);

		template <typename T, usize Size>
		Archive& operator<<(Array<T, Size>& array);

		template <typename K, typename V>
		Archive& operator<<(Map<K, V>& map);

		template <typename T>
		Archive& operator<<(Set<T>& set);

		template <typename T, typename K>
		Archive& operator<<(Pair<T, K>& pair);

		//-----------------------------------------------------------------------------

		// Forwards all of the "values" to the "<<" operator sequentially
		template <typename... Values>
		Archive& Serialize(Values&&... values);

	protected:
		Serializer m_Serializer;
	};

	// Binary InputOutput Archives
	//-----------------------------------------------------------------------------

	class BinaryOutputArchive : public Archive<BinaryWriter>
	{
	public:
		void WriteToFile(char const* path);
	};

	class BinaryInputArchive : public Archive<BinaryReader>
	{
	public:
		~BinaryInputArchive();

		void ReadFromFile(char const* path);
		void ReadFromBlob(Blob const& blob);

	private:
		char* m_pData = nullptr;
	};

	// Helper Macros for the user
	//-----------------------------------------------------------------------------

	// Macro to add serialization functionality to a class/struct
	// Only the members passed as argument to the macro will be serialized
#define CKE_SERIALIZE(...) \
	template <typename Serializer> \
		requires  IsSerializer<Serializer> \
	friend class CKE::Archive; \
		\
		template <typename Serializer> \
		requires  IsSerializer<Serializer> \
	void Serialize(CKE::Archive<Serializer>&archive) { archive.Serialize(__VA_ARGS__); }
}


//-----------------------------------------------------------------------------


namespace CKE {
	template <typename T>
	concept IsNotSerializablePrimitive = std::is_class_v<T> &&
			!std::is_same_v<T, CKE::String>;

	template <typename Serializer> requires IsSerializer<Serializer>
	template <typename T>
	Archive<Serializer>& Archive<Serializer>::operator<<(T& value) {
		// If T is not a primitive type, call the serialization function of T
		if constexpr (IsNotSerializablePrimitive<T>) { value.Serialize(*this); }
		else if constexpr (std::is_enum_v<T>) {
			if constexpr (std::is_base_of_v<IWriter, Serializer>) {
				auto enumValue = static_cast<std::underlying_type_t<T>>(value);
				m_Serializer.Write(enumValue);
			}
			else {
				std::underlying_type_t<T> enumValue;
				m_Serializer.Read(enumValue);
				value = (T)enumValue;
			}
		}
		else {
			// T is a primitive type so we serialize/deserialize it directly
			if constexpr (std::is_base_of_v<IWriter, Serializer>) { m_Serializer.Write(value); }
			else { m_Serializer.Read(value); }
		}
		return *this;
	}

	template <typename Serializer> requires IsSerializer<Serializer>
	template <typename T>
	Archive<Serializer>& Archive<Serializer>::operator<<(Vector<T>& vector) {
		u64 numElements = 0;

		// Read/Write vector size
		if constexpr (std::is_base_of_v<IWriter, Serializer>) {
			numElements = vector.size();
			m_Serializer.Write(numElements);
		}
		else {
			m_Serializer.Read(numElements);
			vector.resize(numElements);
		}

		if constexpr (std::is_integral_v<T> || std::is_floating_point_v<T>) {
			if constexpr (std::is_base_of_v<IWriter, Serializer>) {
				m_Serializer.WriteBlob(vector.data(), numElements * sizeof(T));
			}
			else {
				m_Serializer.ReadBlob(vector.data(), numElements * sizeof(T));
			}
		}
		else {
			// Serialize Elements Individually
			for (u64 i = 0; i < numElements; ++i) { *this << vector[i]; }
		}

		return *this;
	}

	template <typename Serializer> requires IsSerializer<Serializer>
	template <typename T, usize Size>
	Archive<Serializer>& Archive<Serializer>::operator<<(Array<T, Size>& array) {
		u64 numElements = array.size(); // Cast to u64

		if constexpr (std::is_base_of_v<IWriter, Serializer>) { m_Serializer.Write(numElements); }
		else {
			m_Serializer.Read(numElements);
			if (numElements != Size) { CKE_UNREACHABLE_CODE(); } // Reading an array of different size
		}

		// Forward the rest of the elements to be written to/read from
		for (u64 i = 0; i < Size; ++i) { *this << array[i]; }

		return *this;
	}

	template <typename Serializer> requires IsSerializer<Serializer>
	template <typename K, typename V>
	Archive<Serializer>& Archive<Serializer>::operator<<(Map<K, V>& map) {
		u64 numElements = map.size();
		if constexpr (std::is_base_of_v<IWriter, Serializer>) {
			m_Serializer.Write(numElements);
			for (auto const& [key, value] : map) {
				*this << key << value;
			}
		}
		else {
			m_Serializer.Read(numElements);
			map.reserve(numElements);

			K key;
			V value;
			for (u64 i = 0; i < numElements; ++i) {
				// Read key and value and populate the map with it
				*this << key << value;
				map.insert({key, value});
			}
		}

		return *this;
	}

	template <typename Serializer> requires IsSerializer<Serializer>
	template <typename T>
	Archive<Serializer>& Archive<Serializer>::operator<<(Set<T>& set) {
		u64 numElements = set.size();
		if constexpr (std::is_base_of_v<IWriter, Serializer>) {
			m_Serializer.Write(numElements);
			for (auto const& value : set) { *this << value; }
		}
		else {
			m_Serializer.Read(numElements);
			set.reserve(numElements);
			T element;
			for (u64 i = 0; i < numElements; ++i) {
				*this << element;
				set.insert(element);
			}
		}

		return *this;
	}

	template <typename Serializer> requires IsSerializer<Serializer>
	template <typename T, typename K>
	Archive<Serializer>& Archive<Serializer>::operator<<(Pair<T, K>& pair) {
		*this << pair.first << pair.second;
		return *this;
	}

	template <typename Serializer> requires IsSerializer<Serializer>
	template <typename... Values>
	Archive<Serializer>& Archive<Serializer>::Serialize(Values&&... values) {
		((*this) << ... << values);
		return *this;
	}
}
