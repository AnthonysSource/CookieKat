#pragma once
#include "CookieKat/Core/Containers/Containers.h"
#include "CookieKat/Core/Math/Math.h"
#include "CookieKat/Core/Platform/PrimitiveTypes.h"

namespace CKE {
	// Auxiliary class that provides easier access to a CubeMap texture
	class CubeMapWrapper
	{
	public:
		// NOTE: Currently only textures in R8G8B8A8 format are supported
		CubeMapWrapper(void* pFaces[6], i32 faceSize);

		// Returns the color at the given cubemap coords
		Vec3 GetPixelColor(i32 faceIdx, i32 x, i32 y);

		// Returns the solid angle associated to a cubemap pixel
		f32 GetPixelSolidAngle(i32 faceIdx, i32 x, i32 y);

		// Returns the direction that corresponds to a cubemap pixel
		Vec3 GetPixelSphericalDirection(i32 faceIdx, i32 faceX, i32 faceY);

		// Returns the width/height of a cubemap face
		i32 GetFaceSize();

	private:
		float AreaIntegral(f32 x, f32 y);

		// Convert pixel coords to texture coordinate and remap the range from [0, 1] to [-1, 1]
		float ConvertPixelCoordToTextureCoord(i32 pixelCoord, i32 maxSize);

	private:
		u32* m_pFaces[6];
		i32  m_FaceSize;
	};
}

namespace CKE {
	class SphericalHarmonicsUtils
	{
	public:
		// Calculates all 2nd Degree Spherical Harmonics coefficients for a given CubeMap texture.
		// Returns one set of 9 coefficients per color channel.
		static void SHProjectCubeMap(Vec3 shCoeff[9], CubeMapWrapper& cubeMap);
		static void SHProjectCubeMap(Vec3 shCoeff[9], void* pFaces[6], i32 faceSize);
		static void SHProjectCubeMap(Vec3 shCoeff[9], Array<Vector<u8>, 6> const& faces, i32 faceSize);
	};
}
