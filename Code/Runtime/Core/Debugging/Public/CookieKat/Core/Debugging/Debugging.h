#pragma once

namespace CKE::Debug {
	class Manager
	{
	public:

	private:
	};
}

#define CKE_FORCE_INLINE __forceinline
#define CKE_DEBUG_BREAK(x) __debugbreak()

#define CKE_DEBUG_ENABLE_OPTIMIZATION(x) __pragma(optimize("", on ))
#define CKE_DEBUG_DISABLE_OPTIMIZATION(x) __pragma(optimize("", off))

#ifdef CKE_BUILDSYSTEM_ASSERTS_ENABLE
#
#	define CKE_DEBUG_ASSERT(x) assert(x)
#
#else
#
#	define CKE_DEBUG_ASSERT(x)
#
#endif
