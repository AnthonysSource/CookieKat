#include "ResourceCompiler.h"

#include "CookieKat/Core/Containers/String.h"
#include "CookieKat/Core/FileSystem/FileSystem.h"
#include "CookieKat/Core/Serialization/BinarySerialization.h"
#include "CookieKat/Core/Serialization/Archive.h"

#include "CookieKat/Systems/Resources/ResourceID.h"
#include "CookieKat/Engine/Resources/Resources/PipelineResource.h"
#include "CookieKat/Engine/Resources/Loaders/PipelineLoader.h"
#include "CookieKat/Engine/Resources/Resources/RenderMaterialResource.h"

#include <rapidjson/document.h>
#include <stb_image.h>
#include <lodepng.h>

//-----------------------------------------------------------------------------

namespace CKE {
	void ResourceCompiler::Initialize(const char* inputBasePath, const char* outputBasePath) {
		m_CompilerData.m_InputBasePath = inputBasePath;
		m_CompilerData.m_OutputBasePath = outputBasePath;
		m_MaterialCompiler.Initialize();
	}

	void ResourceCompiler::CompileMaterial(String const& fileBaseName) {
		String pInputPath = String(fileBaseName).append(".ckadef");
		String pResourcePath = String(fileBaseName).append(".mat");

		// Open Json Asset Definition
		//-----------------------------------------------------------------------------

		Blob                assetDefBlob = g_FileSystem.ReadBinaryFile(pInputPath);
		rapidjson::Document doc;
		String const        assetDefJson = String(assetDefBlob.begin(), assetDefBlob.end());
		doc.Parse(assetDefJson.c_str());

		// Parse Asset Definition
		//-----------------------------------------------------------------------------

		if (doc["AssetType"].GetString() != String("Material")) {
			std::cout << "Input file is not a material definition" << std::endl;
			return;
		}

		String albedoID = doc["AlbedoTextureID"].GetString();
		String roughnessID = doc["RoughnessTextureID"].GetString();
		String metalicID = doc["MetalicTextureID"].GetString();
		String normalID = doc["NormalTextureID"].GetString();

		// Write to file
		//-----------------------------------------------------------------------------

		BinaryOutputArchive ar{};

		ResourceHeader header{};
		header.m_ResourceType = 2; // TODO: Replace with Type System
		header.m_ResourcePath = pResourcePath;
		header.m_DependencyPaths.push_back(albedoID);
		header.m_DependencyPaths.push_back(roughnessID);
		header.m_DependencyPaths.push_back(metalicID);
		header.m_DependencyPaths.push_back(normalID);

		ar << header;

		RenderMaterialResource material{};
		material.m_AlbedoTexture = TResourceID<RenderTextureResource>{0};
		material.m_RoughnessTexture = TResourceID<RenderTextureResource>{1};
		material.m_MetalicTexture = TResourceID<RenderTextureResource>{2};
		material.m_NormalTexture = TResourceID<RenderTextureResource>{3};

		ar << material;

		ar.WriteToFile(pResourcePath.c_str());
	}

	void ResourceCompiler::CompilePipeline(String const& fileBaseName) {
		String pInputPath = String(fileBaseName).append(".ckadef");
		String pOutputPath = String(fileBaseName).append(".pipeline");

		// Open Json Asset Definition
		//-----------------------------------------------------------------------------

		Blob                assetDefBlob = g_FileSystem.ReadBinaryFile(pInputPath);
		rapidjson::Document doc;
		String const        assetDefJson = String(assetDefBlob.begin(), assetDefBlob.end());
		doc.Parse(assetDefJson.c_str());

		// Parse Asset Definition
		//-----------------------------------------------------------------------------

		if (doc["AssetType"].GetString() != String("Pipeline")) {
			std::cout << "Input file is not a material definition" << std::endl;
			return;
		}

		//String pipelineType = doc["PipelineType"].GetString();
		//if(pipelineType == "Graphics") {
		//	
		//}else if (pipelineType == "Compute") {
		//	
		//}
		String vertPath = doc["VertexShader"].GetString();
		String fragPath = doc["FragmentShader"].GetString();
		// String computePath = doc["ComputeShader"].GetString();

		// Read Source Files
		//-----------------------------------------------------------------------------

		bool enableDebugInfo = false;

		String compileCmdVert = "glslangValidator.exe -V " + vertPath +
				" -o tempShader.vert";
		String compileCmdFrag = "glslangValidator.exe -V " + fragPath +
				" -o tempShader.frag";
		system(compileCmdVert.c_str());
		system(compileCmdFrag.c_str());

		Blob   vertBlob = g_FileSystem.ReadBinaryFile("tempShader.vert");
		Blob   fragBlob = g_FileSystem.ReadBinaryFile("tempShader.frag");
		String vertShaderSource = String(vertBlob.begin(), vertBlob.end());
		String fragShaderSource = String(fragBlob.begin(), fragBlob.end());
		g_FileSystem.RemoveFile("tempShader.vert");
		g_FileSystem.RemoveFile("tempShader.frag");

		// Compile
		//-----------------------------------------------------------------------------

		BinaryOutputArchive archive{};

		ResourceHeader header{};
		header.m_ResourceType = 1; // TODO: Replace with Type System
		header.m_ResourcePath = pOutputPath;

		PipelineResource pipelineResource{};
		pipelineResource.m_VertShaderSource = vertBlob;
		pipelineResource.m_FragShaderSource = fragBlob;

		archive << header << pipelineResource;
		archive.WriteToFile(pOutputPath.c_str());
	}

	void ResourceCompiler::CompileTexture(String const& fileBaseName) {
		String pInputPath = String(fileBaseName).append(".ckadef");
		String pResourcePath = String(fileBaseName).append(".tex");

		// Open Json Asset Definition
		//-----------------------------------------------------------------------------

		Blob                assetDefBlob = g_FileSystem.ReadBinaryFile(pInputPath);
		rapidjson::Document doc;
		String const        assetDefJson = String(assetDefBlob.begin(), assetDefBlob.end());
		doc.Parse(assetDefJson.c_str());

		// Parse Asset Definition
		//-----------------------------------------------------------------------------

		if (doc["AssetType"].GetString() != String("Texture")) {
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
		Blob  imageBlob = g_FileSystem.ReadBinaryFile(path);
		u64   imageByteSize = imageBlob.size();
		i32   numChannels = 0;
		i32   width = 0;
		i32   height = 0;
		void* pRawTextureBytes = stbi_load_from_memory(imageBlob.data(), imageByteSize,
		                                               &width, &height, &numChannels, 4);
		tex.m_Data.resize(width * height * 4);
		memcpy(tex.m_Data.data(), pRawTextureBytes, width * height * 4);

		Vector<u8> compressedTexture{};
		compressedTexture.reserve(width * height * 4);
		if (lodepng::encode(compressedTexture, tex.m_Data, width, height));
		tex.m_Data = compressedTexture;

		stbi_image_free(pRawTextureBytes);

		if (format == "SRGB") { tex.m_Desc.m_Format = TextureFormat::R8G8B8A8_SRGB; }
		else if (format == "UNORM") { tex.m_Desc.m_Format = TextureFormat::R8G8B8A8_UNORM; }

		tex.m_Desc.m_Size.x = width;
		tex.m_Desc.m_Size.y = height;

		ar << tex;

		ar.WriteToFile(pResourcePath.c_str());
	}

	void ResourceCompiler::CompileCubeMap(String const& fileBaseName) {
		String pInputPath = String(fileBaseName).append(".ckadef");
		String pResourcePath = String(fileBaseName).append(".cubeMap");

		// Open Json Asset Definition
		//-----------------------------------------------------------------------------

		Blob                assetDefBlob = g_FileSystem.ReadBinaryFile(pInputPath);
		rapidjson::Document doc;
		String const        assetDefJson = String(assetDefBlob.begin(), assetDefBlob.end());
		doc.Parse(assetDefJson.c_str());

		// Parse Asset Definition
		//-----------------------------------------------------------------------------

		if (doc["AssetType"].GetString() != String("CubeMap")) {
			std::cout << "Input file is not a material definition" << std::endl;
			return;
		}

		Array<String, 6> cubeMapPaths;
		cubeMapPaths[0] = doc["N_Positive_Path"].GetString();
		cubeMapPaths[1] = doc["N_Negative_Path"].GetString();
		cubeMapPaths[2] = doc["Y_Positive_Path"].GetString();
		cubeMapPaths[3] = doc["Y_Negative_Path"].GetString();
		cubeMapPaths[4] = doc["Z_Positive_Path"].GetString();
		cubeMapPaths[5] = doc["Z_Negative_Path"].GetString();
		String format = doc["Format"].GetString();

		// Write to file
		//-----------------------------------------------------------------------------

		BinaryOutputArchive ar{};

		// Write Asset Header
		ResourceHeader header{};
		header.m_ResourceType = 3; // TODO: Replace with Type System
		header.m_ResourcePath = pResourcePath;
		ar << header;

		// Load All of the 6 faces and save them to the converted file
		i32   numChannels = 0, width = 0, height = 0;
		Vector<u8> uncompressedTexture{};

		RenderCubeMapResource tex{};
		for (i32 i = 0; i < 6; ++i) {
			String const& path = cubeMapPaths[i];
			Vector<u8>& faceData = tex.m_Faces[i];
			
			Blob  imageBlob = g_FileSystem.ReadBinaryFile(path);
			void* pRawTextureBytes = stbi_load_from_memory(imageBlob.data(), imageBlob.size(),
			                                               &width, &height, &numChannels, 4);
			uncompressedTexture.resize(width * height * 4);

			memcpy(uncompressedTexture.data(), pRawTextureBytes, width * height * 4);

			faceData.reserve(width * height * 4);
			lodepng::encode(faceData, uncompressedTexture, width, height);

			stbi_image_free(pRawTextureBytes);
		}

		tex.m_FaceWidth = width;
		tex.m_FaceHeight = height;

		ar << tex;

		ar.WriteToFile(pResourcePath.c_str());
	}
};
