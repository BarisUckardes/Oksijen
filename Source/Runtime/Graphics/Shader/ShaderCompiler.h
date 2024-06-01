#pragma once
#include <string>
#include <shaderc/shaderc.hpp>

namespace Oksijen
{
	class RUNTIME_API ShaderCompiler final
	{
	public:
		static bool TextToSPIRV(const std::string& source, const std::string& entryMethod, const shaderc_shader_kind kind, const shaderc_source_language langauge,unsigned char** ppBytesOut,unsigned int& bytesSizeOut,std::string& errorMessageOut);
	public:
		ShaderCompiler() = delete;
		~ShaderCompiler() = delete;
	};
}