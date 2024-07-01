#include "MeshVertexLayout.h"
#include <Runtime/Graphics/Utils/FormatUtils.h>

namespace Oksijen
{
    MeshVertexLayout::MeshVertexLayout(const std::vector<VkFormat>& formats) : mFormats(formats)
    {
        mSize = 0;
        for (const VkFormat format : formats)
        {
            mSize += FormatUtils::GetFormatSize(format);
        }
    }
}