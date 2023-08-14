#pragma once

#include "CookieKat/Core/Containers/String.h"

#include "Compilers/MaterialCompiler.h"

namespace CKE {
	struct CompilerData
	{
		String m_InputFilePath;
		String m_OutputFilePath;

		String m_InputBasePath;
		String m_OutputBasePath;
	};

	class ResourceCompiler
	{
	public:
		void Initialize(const char* inputBasePath, const char* outputBasePath);

		void CompileMaterial(String const& fileBaseName);
		void CompilePipeline(String const& fileBaseName);
		void CompileTexture(String const& fileBaseName);
		void CompileCubeMap(String const& fileBaseName);

	private:
		MaterialCompiler m_MaterialCompiler{};
		CompilerData     m_CompilerData;
	};
}
