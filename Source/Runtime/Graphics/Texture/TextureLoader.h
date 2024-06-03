#pragma once
#include <string>

namespace Oksijen
{
	class RUNTIME_API TextureLoader final
	{
	public:
		static bool LoadFromPath(const std::string& path,const unsigned char requestedChannelCount, unsigned char** ppDataOut, unsigned long long& dataSizeOut,unsigned int& widthOut,unsigned int& heightOut,unsigned int& channelCountOut);
	public:
		TextureLoader() = delete;
		~TextureLoader() = delete;
	};
}