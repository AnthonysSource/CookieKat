#pragma once

#include "CookieKat/Core/Platform/PrimitiveTypes.h"

#define GLM_FORCE_INTRINSICS // Forces the use of vectorization instruction sets
#define GLM_FORCE_DEPTH_ZERO_TO_ONE // Force the use of Vulkan's depth range
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

namespace CKE {
	// Defines for all of the common math types and values
	// Prefer using these names instead of the library-specific ones
	//-----------------------------------------------------------------------------

	static constexpr f64 F64_EPSILON = DBL_EPSILON;
	static constexpr f32 F32_EPSILON = FLT_EPSILON;

	using Vec4 = glm::vec4;
	using Vec3 = glm::vec3;
	using Vec2 = glm::vec2;

	using Int2 = glm::ivec2;
	using Int3 = glm::ivec3;
	using Int4 = glm::ivec4;

	using UInt2 = glm::uvec2;
	using UInt3 = glm::uvec3;
	using UInt4 = glm::uvec4;

	using Quaternion = glm::quat;

	using Mat4 = glm::mat4;
	using Mat3 = glm::mat3;
}


// TODO: In-development custom mathematics library, do not use pls
//-----------------------------------------------------------------------------

namespace CKEMath {
	template <typename T>
	struct Vec3
	{
		T x;
		T y;
		T z;

		constexpr Vec3();
		constexpr Vec3(T x, T y, T z);

		constexpr Vec3 operator+(Vec3 const& rhs) const;
		constexpr Vec3 operator-(Vec3 const& rhs) const;

		T Magnitude() const;

		static constexpr T    Dot(Vec3 const& a, Vec3 const& b);
		static constexpr Vec3 Cross(Vec3 const& a, Vec3 const& b);
	};
}


namespace CKEMath {
	template <typename T>
	T Vec3<T>::Magnitude() const {
		return std::sqrt(x * x + y * y + z * z);
	}

	template <typename T>
	constexpr Vec3<T>::Vec3() : x{0}, y{0}, z{0} { }

	template <typename T>
	constexpr Vec3<T>::Vec3(T x, T y, T z) : x{x}, y{y}, z{z} { }

	template <typename T>
	constexpr Vec3<T> Vec3<T>::operator+(Vec3 const& rhs) const {
		return Vec3
		{
			x + rhs.x,
			y + rhs.y,
			z + rhs.z,
		};
	}

	template <typename T>
	constexpr Vec3<T> Vec3<T>::operator-(Vec3 const& rhs) const {
		return Vec3
		{
			x - rhs.x,
			y - rhs.y,
			z - rhs.z,
		};
	}

	template <typename T>
	constexpr T Vec3<T>::Dot(Vec3 const& a, Vec3 const& b) {
		return a.x * b.x + a.y * b.y + a.z * b.z;
	}

	template <typename T>
	constexpr Vec3<T> Vec3<T>::Cross(Vec3 const& a, Vec3 const& b) {
		return Vec3{
			a.y * b.z - b.y * a.z,
			b.x * a.z - a.x * b.z,
			a.x * b.y - b.x * a.y,
		};
	}
}
