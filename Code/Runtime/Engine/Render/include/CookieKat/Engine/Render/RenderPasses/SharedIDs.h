#pragma once

#include "CookieKat/Core/Containers/String.h"

namespace CKE {
	struct SceneGlobal
	{
		inline static const String View{ "ViewUBO" };
		inline static const String Lights{ "LightsUBO" };
		inline static const String ObjectData{ "ObjectData" };
		inline static const String EnviorementData{ "EnviorementData" };
	};


	struct GBuffer
	{
		inline static const String Albedo = "Albedo";
		inline static const String Normals = "Normals";
		inline static const String Position = "Position";
		inline static const String RoughMetalRefl = "RoughnessMetallic";
		inline static const String DepthStencil = "DepthBuffer";
		inline static const String ObjectIdx = "ObjectIdx";
	};

	struct General
	{
		inline static const String Swapchain{ "Swapchain" };
		inline static const String DepthStencil = "DepthBuffer";
	};
}
