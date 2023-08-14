#pragma once
#include "CookieKat/Core/Containers/Containers.h"
#include "CookieKat/Core/Math/Math.h"

namespace CKE {
	struct TriangleVert
	{
		Vec3 m_Pos;
		Vec3 m_Color;
	};

	class CommonShapes
	{
	public:
		inline static Array<TriangleVert, 3> m_TriangleVerts = {
			TriangleVert{Vec3{0.0f, 1.0f, 0.0f}, Vec3{1.0f, 0.0f, 0.0f}},
			TriangleVert{Vec3{1.0f, 1.0f, 0.0f}, Vec3{0.0f, 1.0f, 0.0f}},
			TriangleVert{Vec3{0.0f, 0.0f, 0.0f}, Vec3{0.0, 0.0, 1.0f}}
		};
	};
}
