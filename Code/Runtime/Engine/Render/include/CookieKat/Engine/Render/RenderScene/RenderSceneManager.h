#pragma once

#include "CookieKat/Core/Containers/Containers.h"
#include "CookieKat/Core/Math/Math.h"
#include "CookieKat/Systems/RenderAPI/RenderHandle.h"

namespace CKE {
	class EntityDatabase;
}

namespace CKE {
	// View & Projection data uploaded to the GPU for rendering
	struct ViewDataGPU
	{
		Mat4 m_ViewProj;
		Mat4 m_View;
		Mat4 m_Proj;
		Mat4 m_ViewInv;
		Mat4 m_ProjInv;
	};

	// Representation of a point light structure in the GPU
	struct PointLightGPU
	{
		Vec4 m_ViewSpacePosition;
		Vec4 m_Radiance;
	};

	// Scene lights data uploaded to the GPU for rendering
	struct LightsDataGPU
	{
		Vec4          m_Size;
		PointLightGPU m_PointLights[1000];
	};

	// Per-Object data that is uploaded to the GPU for rendering
	struct ObjectDataGPU
	{
		Mat4             m_Local2World;
		Mat4             m_NormalMat;
		alignas(16) Vec3 m_AlbedoOverride;
		f32              m_RoughnessOverride;
		f32              m_MetallicOverride;
		f32              m_Reflectance;
	};

	struct SHCoeffs9GPU
	{
		Vec4 m_Coeffs[9];
	};

	struct EnvironmentGPU
	{
		SHCoeffs9GPU m_EnvSH;     // SH for the environment map
		SHCoeffs9GPU m_TestSH[9]; // To test individual contributions of each coeff
	};
}

namespace CKE {
	struct ViewportData
	{
		Vec2 m_Offset{0.0, 0.0};
		Vec2 m_Extent{0.0, 0.0};
	};

	// Base Render Area, Viewport and Scissor values that will be used when rendering
	struct RenderingSettings
	{
		Vec2         m_RenderArea;
		ViewportData m_Viewport;
	};
}

namespace CKE {
	// All of the scene data that will be uploaded to the GPU for rendering
	struct RenderSceneData
	{
		ViewDataGPU           m_ViewData{};
		Vector<ObjectDataGPU> m_ObjectData{};
		LightsDataGPU         m_LightsData{};
		EnvironmentGPU        m_EnviorementData{};
	};

	// Contains all of the object and lights data to render a scene from a specific view.
	// Handles the lifetime and contents of the GPU-side buffers of said data.
	class RenderSceneManager
	{
	public:
		// Creates the required GPU buffers that will contain this scene data
		void InitializeGPUBuffers(RenderDevice* pDevice);
		// Destroys all of the GPU-side buffers that represent this scene
		void CleanupGPUBuffers(RenderDevice* pDevice);

		// Sets and uploads the new camera matrices to the GPU
		void SetCameraMatrices(Mat4 view, Mat4 proj);
		// Sets the base Render Area, Viewport and Scissor settings that will be used for the rendering commands
		void SetRenderingViewSettings(RenderingSettings view);
		// Sets and uploads environment data to the GPU
		void SetEnviorementData(EnvironmentGPU data);

		// Copies all of the scene data from an entity world and sends it to the GPU
		void CopySceneDataFromEntityWorld(RenderDevice* pDevice, EntityDatabase* pEntities);

	public:
		RenderDevice* m_pDevice = nullptr;

		RenderingSettings m_RenderingViewSettings{};
		RenderSceneData   m_Scene{};

		BufferHandle m_ViewBuffer;
		BufferHandle m_ObjectDataBuffer;
		BufferHandle m_LightsBuffer;
		BufferHandle m_EnviorementBuffer;
	};
}
