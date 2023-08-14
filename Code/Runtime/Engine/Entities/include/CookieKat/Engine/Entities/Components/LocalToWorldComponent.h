#pragma once

#include "API.h"
#include "CookieKat/Core/Math/Math.h"

namespace CKE
{
	struct CKE_API VelocityComponent
	{
		Vec3 m_Velocity;
	};

	struct CKE_API LocalToWorldComponent
	{
		Mat4 m_LocalToWorld;
	};

	inline Mat4 GetTransformL2W(Vec3 pos, Vec3 eulerRot, Vec3 scale) {
		Mat4 model = glm::translate(Mat4{1.0f}, pos);
		model = glm::rotate(model, glm::radians(eulerRot.z), Vec3{ 0.0f, 0.0f, 1.0f });
		model = glm::rotate(model, glm::radians(eulerRot.y), Vec3{ 0.0f, 1.0f, 0.0f });
		model = glm::rotate(model, glm::radians(eulerRot.x), Vec3{ 1.0f, 0.0f, 0.0f });
		model = glm::scale(model, scale);
		return model;
	};
}
