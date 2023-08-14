#include "CookieKat/Systems/RenderUtils/SphericalHarmonicsUtils.h"
#include "CookieKat/Core/Platform/Asserts.h"

namespace CKE {
	CubeMapWrapper::CubeMapWrapper(void* pFaces[6], i32 faceSize) {
		CKE_ASSERT(faceSize > 0);
		CKE_ASSERT(pFaces != nullptr);

		m_pFaces[0] = (u32*)pFaces[0];
		m_pFaces[1] = (u32*)pFaces[1];
		m_pFaces[2] = (u32*)pFaces[2];
		m_pFaces[3] = (u32*)pFaces[3];
		m_pFaces[4] = (u32*)pFaces[4];
		m_pFaces[5] = (u32*)pFaces[5];
		m_FaceSize = faceSize;
	}

	Vec3 CubeMapWrapper::GetPixelColor(i32 faceIdx, i32 x, i32 y) {
		u32 pixelColor255 = m_pFaces[faceIdx][x + y * m_FaceSize];
		// NOTE: Maybe bit shifts are better for converting to u8
		u8   r = *((u8*)&pixelColor255);
		u8   g = *((u8*)&pixelColor255 + 1);
		u8   b = *((u8*)&pixelColor255 + 2);
		Vec3 pixelColorF{ r / 255.0f, g / 255.0f, b / 255.0f };
		return pixelColorF;
	}

	f32 CubeMapWrapper::GetPixelSolidAngle(i32 faceIdx, i32 x, i32 y) {
		f32 s = ConvertPixelCoordToTextureCoord(x, m_FaceSize) * 2.0f - 1.0f;
		f32 t = ConvertPixelCoordToTextureCoord(y, m_FaceSize) * 2.0f - 1.0f;

		f32 half = 1.0f / m_FaceSize;
		f32 x0 = s - half;
		f32 y0 = t - half;
		f32 x1 = s + half;
		f32 y1 = t + half;

		return AreaIntegral(x0, y0) - AreaIntegral(x0, y1) - AreaIntegral(x1, y0) + AreaIntegral(x1, y1);
	}

	Vec3 CubeMapWrapper::GetPixelSphericalDirection(i32 faceIdx, i32 faceX, i32 faceY) {
		f32 s = ConvertPixelCoordToTextureCoord(faceX, m_FaceSize) * 2.0f - 1.0f;
		f32 t = ConvertPixelCoordToTextureCoord(faceY, m_FaceSize) * 2.0f - 1.0f;

		f32 x, y, z;

		// Get un-normalized direction of the pixel in the CubeMap
		switch (faceIdx) {
		case 4: x = s;
			y = -t;
			z = 1;
			break;
		case 5: x = -s;
			y = -t;
			z = -1;
			break;
		case 1: x = -1;
			y = -t;
			z = s;
			break;
		case 0: x = 1;
			y = -t;
			z = -s;
			break;
		case 2: x = s;
			y = 1;
			z = t;
			break;
		case 3: x = s;
			y = -1;
			z = -t;
			break;
		}

		// Normalize vector
		f32 inv_len = 1.0f / std::sqrtf(x * x + y * y + z * z);
		return Vec3{ x * inv_len, y * inv_len, z * inv_len };
	}

	i32 CubeMapWrapper::GetFaceSize() {
		return m_FaceSize;
	}

	f32 CubeMapWrapper::AreaIntegral(f32 x, f32 y) {
		return std::atan2f(x * y, std::sqrtf(x * x + y * y + 1));
	}

	f32 CubeMapWrapper::ConvertPixelCoordToTextureCoord(i32 pixelCoord, i32 maxSize) {
		return (pixelCoord + 0.5f) / maxSize;
	}

	void SphericalHarmonicsUtils::SHProjectCubeMap(Vec3 shCoeff[9], CubeMapWrapper& cubeMap) {
		for (int i = 0; i < 9; ++i) {
			shCoeff[i] = Vec3{0.0f};
		}

		i32 faceSize = cubeMap.GetFaceSize();
		f32 normWeight = 0.0f;

		for (i32 faceIdx = 0; faceIdx < 6; ++faceIdx) {
			for (i32 y = 0; y < faceSize; ++y) {
				for (i32 x = 0; x < faceSize; ++x) {
					constexpr f32 SHconst_0 = 0.28209479177387814347f; // 1/2 * sqrt(1/pi)
					constexpr f32 SHconst_1 = 0.48860251190291992159f; // sqrt(3 /(4pi))
					constexpr f32 SHconst_2 = 1.09254843059207907054f; // 1/2 * sqrt(15/pi)
					constexpr f32 SHconst_3 = 0.31539156525252000603f; // 1/4 * sqrt(5/pi)
					constexpr f32 SHconst_4 = 0.54627421529603953527f; // 1/4 * sqrt(15/pi)

					Vec3 pixelColor = cubeMap.GetPixelColor(faceIdx, x, y);
					f32  pixelSolidAngle = cubeMap.GetPixelSolidAngle(faceIdx, x, y); // sin(0)d0dPhi
					Vec3 pixelDir = cubeMap.GetPixelSphericalDirection(faceIdx, x, y);

					normWeight += pixelSolidAngle;

					shCoeff[0] += pixelColor * SHconst_0 * pixelSolidAngle;

					shCoeff[1] += pixelColor * SHconst_1 * pixelDir.y * pixelSolidAngle;
					shCoeff[2] += pixelColor * SHconst_1 * pixelDir.z * pixelSolidAngle;
					shCoeff[3] += pixelColor * SHconst_1 * pixelDir.x * pixelSolidAngle;

					shCoeff[4] += pixelColor * SHconst_2 * pixelDir.x * pixelDir.y * pixelSolidAngle;
					shCoeff[5] += pixelColor * SHconst_2 * pixelDir.y * pixelDir.z * pixelSolidAngle;
					shCoeff[6] += pixelColor * SHconst_3 * (3.0f * pixelDir.z * pixelDir.z - 1.0f) *
							pixelSolidAngle;
					shCoeff[7] += pixelColor * SHconst_2 * pixelDir.x * pixelDir.z * pixelSolidAngle;
					shCoeff[8] += pixelColor * SHconst_4 * (pixelDir.x * pixelDir.x - pixelDir.y * pixelDir.y) *
							pixelSolidAngle;
				}
			}
		}
	}

	void SphericalHarmonicsUtils::SHProjectCubeMap(Vec3 shCoeff[9], void* pFaces[6], i32 faceSize) {
		CubeMapWrapper wrapper{ pFaces, faceSize };
		SHProjectCubeMap(shCoeff, wrapper);
	}

	void SphericalHarmonicsUtils::SHProjectCubeMap(Vec3 shCoeff[9], Array<Vector<u8>, 6> const& faces, i32 faceSize) {
		void* pTexData[6];
		for (int i = 0; i < 6; ++i) {
			pTexData[i] = (void*)faces[i].data();
		}
		return SHProjectCubeMap(shCoeff, pTexData, faceSize);
	}
}
