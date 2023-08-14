#pragma once

#undef NDEBUG
#include <assert.h>
#include <iostream>

#ifdef CKE_BUILDSYSTEM_ASSERTS_ENABLE
#	define CKE_ASSERT(x) do{ assert(x); } while(false)
#else
#	define CKE_ASSERT(x)
#endif

#define CKE_UNREACHABLE_CODE() do{ assert(false); } while(false)

#define CKE_FORCE_INLINE __forceinline
