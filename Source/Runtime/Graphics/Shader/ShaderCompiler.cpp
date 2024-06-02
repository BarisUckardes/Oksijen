#include "ShaderCompiler.h"

namespace Oksijen
{
	bool ShaderCompiler::TextToSPIRV(const std::string& source, const std::string& entryMethod, const shaderc_shader_kind kind, const shaderc_source_language langauge, unsigned char** ppBytesOut, unsigned int& bytesSizeOut, std::string& errorMessageOut)
	{
		shaderc::Compiler compiler;
		shaderc::CompileOptions options;

		//Set options
		options.SetSourceLanguage(langauge);
		options.SetSuppressWarnings();

		//Preprocess
		const shaderc::PreprocessedSourceCompilationResult preprocessResult = compiler.PreprocessGlsl(source, kind, entryMethod.c_str(), options);
		if (preprocessResult.GetNumErrors() > 0 || preprocessResult.GetCompilationStatus() != shaderc_compilation_status_success)
		{
			errorMessageOut = preprocessResult.GetErrorMessage();
			*ppBytesOut = nullptr;
			bytesSizeOut = 0;
			return false;
		}

		//Compile
		const shaderc::SpvCompilationResult result = compiler.CompileGlslToSpv(source.c_str(),source.size(), kind,entryMethod.c_str(), entryMethod.c_str(), options);
		if (result.GetCompilationStatus() != shaderc_compilation_status_success)
		{
			errorMessageOut = result.GetErrorMessage();
			*ppBytesOut = nullptr;
			bytesSizeOut = 0;
			return false;
		}

		const unsigned long long bufferSize = (unsigned char*)result.cend() - (unsigned char*)result.cbegin();
		unsigned char* pBuffer = new unsigned char[bufferSize];
		memcpy(pBuffer, result.cbegin(), bufferSize);
		*ppBytesOut = pBuffer;
		bytesSizeOut = bufferSize;
		return true;
	}
}