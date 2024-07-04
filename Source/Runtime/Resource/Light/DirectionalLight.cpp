#include "DirectionalLight.h"

namespace Oksijen
{
	DirectionalLight::DirectionalLight(GraphicsDevice* pDevice, GraphicsMemory* pMemory, const unsigned int width, const unsigned int height)
		: mDevice(pDevice), mShadowTexture(nullptr), mShadowTextureView(nullptr),mDirection({1.0f,-1.0f,1.0f}),mColor({1.0f,1.0f,1.0f}),mWidth(width),mHeight(height),mPower(1.0f)
	{
		Resize(pMemory, width, height);
	}
	DirectionalLight::~DirectionalLight()
	{
		ClearResources();
	}
	void DirectionalLight::SetDirection(const glm::vec3& direction)
	{
		mDirection = direction;
	}
	void DirectionalLight::SetPower(const float power)
	{
		mPower = power;
	}
	void DirectionalLight::SetColor(const glm::vec3& color)
	{
		mColor = color;
	}
	void DirectionalLight::Resize(GraphicsMemory* pMemory,const unsigned int width, const unsigned int height)
	{
		//Clear former resources
		ClearResources();

		//Create texture
		mShadowTexture = mDevice->CreateTexture(pMemory, VK_IMAGE_TYPE_2D, VK_FORMAT_D16_UNORM, width, height, 1, 1, 1, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_SHARING_MODE_EXCLUSIVE, VK_IMAGE_LAYOUT_UNDEFINED);

		//Create texture view
		constexpr VkComponentMapping mapping =
		{
			VK_COMPONENT_SWIZZLE_IDENTITY,
			VK_COMPONENT_SWIZZLE_IDENTITY,
			VK_COMPONENT_SWIZZLE_IDENTITY,
			VK_COMPONENT_SWIZZLE_IDENTITY
		};
		mShadowTextureView = mDevice->CreateTextureView(mShadowTexture, 0, 0, VK_IMAGE_VIEW_TYPE_2D, VK_FORMAT_D16_UNORM, mapping, VK_IMAGE_ASPECT_DEPTH_BIT);
	}
	void DirectionalLight::ClearResources()
	{
		if (mShadowTexture != nullptr)
		{
			delete mShadowTexture;
			mShadowTexture = nullptr;
		}
		if (mShadowTextureView != nullptr)
		{
			delete mShadowTextureView;
			mShadowTextureView = nullptr;
		}
	}
}