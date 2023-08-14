#include <iostream>

#include "ResourceCompiler.h"
#include "ThirdParty/CLI11.hpp"

#include "CookieKat/Core/Containers/Containers.h"

int main(int argc, char* argv[])
{
	using namespace CKE;

	ResourceCompiler compiler{};

	CLI::App app{ "CKE Resource Compiler" };
	Vector<String> args(argv + 1, argv + argc);

	String fileType = "Unnamed";
	String inputBaseName = "Unnamed";
	app.add_option("-t,--type", fileType, "Type of resource: [texture, material, pipeline]");
	app.add_option("-i,--input", inputBaseName, "File name of the .ckedef asset file without the extension");

	//-----------------------------------------------------------------------------

	try {
		app.parse(args);
	}
	catch (const CLI::ParseError& e) {
		return app.exit(e);
	}

	//-----------------------------------------------------------------------------

	if (fileType == "pipeline")
	{
		std::cout << "Compiling Pipeline...\n";
		compiler.CompilePipeline(inputBaseName);
		std::cout << "Pipeline Compiled\n";
	}
	else if (fileType == "material")
	{
		std::cout << "Compiling Material...\n";
		compiler.CompileMaterial(inputBaseName);
		std::cout << "Material Compiled\n";
	}
	else if (fileType == "texture")
	{
		std::cout << "Compiling Texture...\n";
		compiler.CompileTexture(inputBaseName);
		std::cout << "Texture Compiled\n";
	}
	else if (fileType == "cubemap")
	{
		std::cout << "Compiling CubeMap...\n";
		compiler.CompileCubeMap(inputBaseName);
		std::cout << "CubeMap Compiled\n";
	}
	else
	{
		std::cout << "Resource type [ " << fileType << " ] not supported\n";
	}

	return 0;
}
