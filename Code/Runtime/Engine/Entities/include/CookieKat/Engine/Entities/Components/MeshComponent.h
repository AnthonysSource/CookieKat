#pragma once

#include "CookieKat/Systems/Resources/ResourceID.h"
#include "CookieKat/Engine/Resources/Resources/MeshResource.h"
#include "CookieKat/Engine/Resources/Resources/RenderMaterialResource.h"

namespace CKE {
	struct PBRTextureModifiers
	{
		Vec3  m_Albedo{1.0f};
		float m_Roughness{1.0f};
		float m_MetalMask{1.0f};
		float m_Reflectance{0.04f};

		PBRTextureModifiers() = default;

		PBRTextureModifiers(Vec3 albedo, f32 roughness, f32 metalMask)
			: m_Albedo{albedo}, m_Roughness{roughness}, m_MetalMask{metalMask} {}
	};

	struct MeshComponent
	{
		TResourceID<MeshResource>           m_MeshID;
		TResourceID<RenderMaterialResource> m_MaterialID;
		PBRTextureModifiers                 m_MaterialModifiers;
		u64                                 m_ObjectIdx = 0;

		MeshComponent() = default;

		MeshComponent(TResourceID<MeshResource> meshID, TResourceID<RenderMaterialResource> materialID, u64 objIdx) {
			m_MeshID = meshID;
			m_MaterialID = materialID;
			m_ObjectIdx = objIdx;
		}

		MeshComponent(TResourceID<MeshResource> meshID, PBRTextureModifiers matModifiers, u64 objIdx) {
			m_MeshID = meshID;
			m_MaterialID = TResourceID<RenderMaterialResource>{0};
			m_MaterialModifiers = matModifiers;
			m_ObjectIdx = objIdx;
		}
	};
}
