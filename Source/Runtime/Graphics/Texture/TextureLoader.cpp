#include "TextureLoader.h"
#include <stb_image.h>

namespace Oksijen
{
	bool TextureLoader::LoadFromPath(const std::string& path, unsigned char** ppDataOut, unsigned long long& dataSizeOut, unsigned int& widthOut, unsigned int& heightOut, unsigned int& channelCountOut)
	{
		int width = 0;
		int height = 0;
		int channelCount = 0;

		unsigned char* pData = stbi_load(path.c_str(), &width, &height, &channelCount, 4);

		if (pData == nullptr)
		{
			*ppDataOut = nullptr;
			dataSizeOut = 0;
			widthOut = 0;
			heightOut = 0;
			channelCountOut = 0;
			return false;
		}

		*ppDataOut = pData;
		widthOut = width;
		heightOut = height;
		dataSizeOut = width * height * channelCount;
		channelCountOut = channelCount;

		return true;
	}
}