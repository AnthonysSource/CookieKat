#pragma once

#include <algorithm>
#include <iomanip>

#include "CookieKat/Systems/Input/InputSystem.h"
#include "CookieKat/Core/Profilling/Profilling.h"

#include "CookieKat/Engine/Entities/EntitySystem.h"
#include "CookieKat/Engine/Entities/Components/CameraComponent.h"
#include "CookieKat/Engine/Entities/Components/InputComponent.h"

namespace CKE {
	class FlyCameraSystem : public ECSBaseSystem
	{
	public:
		Vec2 m_Rotation;

		inline void Update(SystemUpdateContext ctx) override {
			CKE_PROFILE_EVENT();

			InputStateComponent* inputComp = ctx.GetEntityDatabase()->GetSingletonComponent<InputStateComponent>();
			Keyboard const*      keyboard = inputComp->m_pInputContext.GetKeyboard();
			Mouse const*         mouse = inputComp->m_pInputContext.GetMouse();

			Vec3 movDir{0.0f};
			if (keyboard->GetKeyHeld(KeyCode::D)) { movDir.x += 1.0f; }
			if (keyboard->GetKeyHeld(KeyCode::A)) { movDir.x -= 1.0f; }
			if (keyboard->GetKeyHeld(KeyCode::W)) { movDir.z -= 1.0f; }
			if (keyboard->GetKeyHeld(KeyCode::S)) { movDir.z += 1.0f; }
			if (keyboard->GetKeyHeld(KeyCode::E)) { movDir.y += 1.0f; }
			if (keyboard->GetKeyHeld(KeyCode::Q)) { movDir.y -= 1.0f; }

			if (glm::length(movDir) > 0.1f) { movDir = glm::normalize(movDir); }

			m_Rotation.x -= (mouse->m_DeltaPos.x * 1.0f * ctx.GetDeltaTime()) / (3.1415f);
			m_Rotation.y -= (mouse->m_DeltaPos.y * 1.0f * ctx.GetDeltaTime()) / (3.1415f);
			m_Rotation.y = std::clamp(m_Rotation.y, -3.1415f / 2.0f, 3.1415f / 2.0f);

			for (auto cam : ctx.GetEntityDatabase()->GetSingleCompIter<CameraComponent>()) {
				Mat4 camTransf{1.0f};
				Mat4 rotX = glm::rotate(Mat4{1.0f}, m_Rotation.y, Vec3(1.0f, 0.0f, 0.0f));
				Mat4 rotY = glm::rotate(Mat4{1.0f}, m_Rotation.x, Vec3(0.0f, 1.0f, 0.0f));

				camTransf = rotY * rotX * camTransf;
				movDir = rotY * rotX * Vec4(movDir, 0.0f);

				camTransf = glm::translate(Mat4{1.0f}, cam->m_CamPos + movDir * cam->m_MovSpeed * ctx.GetDeltaTime()) * camTransf;

				cam->m_CamPos = camTransf[3];

				cam->m_View = glm::inverse(camTransf);
			}
		}
	};
}
