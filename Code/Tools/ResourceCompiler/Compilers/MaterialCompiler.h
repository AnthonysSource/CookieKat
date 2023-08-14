#pragma once

#include "CookieKat/Core/FileSystem/FileSystem.h"
#include "CookieKat/Core/Serialization/Archive.h"
#include "CookieKat/Engine/Resources/Resources/RenderTextureResource.h"
#include "CookieKat/Systems/Resources/IResource.h"
#include "rapidjson/document.h"
#include <stb_image.h>

namespace CKE
{
	struct CompilationParams
	{
		String m_OutputFilePath;
	};


	class MaterialCompiler 
	{
	public:
		void Initialize() ;
		void Compile(Blob assetDef, CompilationParams params) ;
	};

	class TextureCompiler 
	{
	public:
		void Initialize() ;

		void CompileTexture(String const& fileBaseName)
		{
			String pInputPath = String(fileBaseName).append(".ckadef");
			String pResourcePath = String(fileBaseName).append(".tex");

			// Open Json Asset Definition
			//-----------------------------------------------------------------------------

			Blob assetDefBlob = g_FileSystem.ReadBinaryFile(pInputPath);
			rapidjson::Document doc;
			String const assetDefJson = String(assetDefBlob.begin(), assetDefBlob.end());
			doc.Parse(assetDefJson.c_str());

			// Parse Asset Definition
			//-----------------------------------------------------------------------------

			if (doc["AssetType"].GetString() != String("Texture"))
			{
				std::cout << "Input file is not a material definition" << std::endl;
				return;
			}

			String path = doc["TexturePath"].GetString();
			String format = doc["Format"].GetString();

			// Write to file
			//-----------------------------------------------------------------------------

			BinaryOutputArchive ar{};

			ResourceHeader header{};
			header.m_ResourceType = 3; // TODO: Replace with Type System
			header.m_ResourcePath = pResourcePath;

			ar << header;

			RenderTextureResource tex{};

			// Load image binary data
			Blob imageBlob = g_FileSystem.ReadBinaryFile(path);
			u64 imageByteSize = imageBlob.size();
			i32 numChannels = 0;
			i32 width = 0;
			i32 height = 0;
			void* pData = stbi_load_from_memory(imageBlob.data(), imageByteSize,
				&width, &height, &numChannels, 4);
			tex.m_Data.resize(width * height * 4);
			memcpy(tex.m_Data.data(), pData, width * height * 4);
			stbi_image_free(pData);

			if (format == "SRGB") { tex.m_Desc.m_Format = TextureFormat::R8G8B8A8_SRGB; }
			else if (format == "UNORM") { tex.m_Desc.m_Format = TextureFormat::R8G8B8A8_UNORM; }

			tex.m_Desc.m_Size.x = width;
			tex.m_Desc.m_Size.y = height;

			ar << tex;

			ar.WriteToFile(pResourcePath.c_str());
		}

		void Compile(rapidjson::Document& ckadef, BinaryOutputArchive& ar, CompilationParams params) {

			RenderTextureResource tex{};
			String texturePath = ckadef["TexturePath"].GetString();
			Blob imageBlob = g_FileSystem.ReadBinaryFile(texturePath);
			i32 width = 0, height = 0, numChannels = 0;

			void* pData = stbi_load_from_memory(imageBlob.data(), imageBlob.size(),
				&width, &height, &numChannels, 4);
			tex.m_Data.resize(width * height * 4);
			memcpy(tex.m_Data.data(), pData, width * height * 4);
			stbi_image_free(pData);

			String format = ckadef["Format"].GetString();
			if (format == "SRGB") { tex.m_Desc.m_Format = TextureFormat::R8G8B8A8_SRGB; }
			else if (format == "UNORM") { tex.m_Desc.m_Format = TextureFormat::R8G8B8A8_UNORM; }

			tex.m_Desc.m_Size.x = width;
			tex.m_Desc.m_Size.y = height;

			ar << tex;
		}
	};
}
