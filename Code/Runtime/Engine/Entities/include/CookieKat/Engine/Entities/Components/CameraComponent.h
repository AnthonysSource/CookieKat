#pragma once

#include "CookieKat/Core/Math/Math.h"

namespace CKE
{
	struct CameraComponent
	{
		Mat4 m_View = Mat4{1.0f};
		Mat4 m_Proj = glm::perspective(glm::radians(58.7155f), 16.0f / 9.0f, 0.01f, 1'000.0f);
		Vec3 m_CamPos = Vec3(0.0f, 0.0f, 4.0f);
		f32 m_MovSpeed = 1.0f;
	};
}