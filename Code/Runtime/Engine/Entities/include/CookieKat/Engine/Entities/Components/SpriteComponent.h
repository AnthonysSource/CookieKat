#pragma once

#include "CookieKat/Engine/Resources/Resources/RenderMaterialResource.h"

namespace CKE
{
	class CKE_API SpriteComponent
	{
	public:
		TResourceID<TextureHandle> m_TextureHandle;
	};
}
