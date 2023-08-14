#pragma once

#include "CookieKat/Systems/RenderAPI/RenderHandle.h"

namespace CKE {
	template <typename T>
	class RenderResource
	{
	public:
		RenderResource() = default;
		RenderResource(TRenderHandle<T> handle) : m_DBHandle(handle) { }

		TRenderHandle<T> m_DBHandle;
	};
}